#coding:utf-8
"""
broker.py
委托代理
"""
import threading
import json
import os,time,datetime

import traceback
from collections import OrderedDict
import requests
from elxtrade.fundamental.utils.useful import singleton,object_assign,hash_object
from elxtrade.ctp import Tick, OrderRecord,OrderError,\
        OrderCancel,OrderCancelError,TradeReturn,Constants,\
      get_exchange_id
from elxtrade import ctp
from elxtrade import logger
# from elxtrade import base


class OrderStatus(object):
  ## 0 : 空闲 , 1 : 已委托, 2: 部分成 , 3 : 全成 ，4 ： 错误
  Disabled = 'disabled'
  IDLE = 'idle'
  PENDING = 'pending'
  PART = 'part'
  FINISH = 'finish'
  ERROR = 'error'

class OrderRequest(object):
  """订单委托信息"""

  def __init__(self, code , price=0, quantity=1, direction=Constants.Buy, oc = Constants.Open, user_id='C'):
    self.product = None
    self.code = code
    self.user = None  # 附加数据
    self.hedge = Constants.SPEC
    self.stop_price = 0  # 止盈止损价格
    self.market_order = Constants.MarketOrder.A  # 市价类别

    self.direction = direction
    self.oc = oc  # 默认买开

    self.price = price  # 交易价格
    self.quantity = quantity  # 买卖数量

    self.success = None
    self.failure = None
    self.message = ''
    self.order_id = ''
    self.user_id = user_id  # 用户订单编号

    self.min_knock_num_rate = 1
    self.forceclose = False
    self.price_type = chr(Constants.OrderPriceType.LimitPrice)

    self.exchange_id = ''  # 交易所编号 ，在CTP新sdk接口规定必须提供

    self.opts = {}

  def dict(self):
    return hash_object(self, excludes=('product', 'success', 'failure'))



@singleton
class MarketApi(object):
  def __init__(self):
    self.cfgs = {}

  def subInstrument(self,instruments):
    """订阅tick"""
    url = self.cfgs.get("http") + '/ctp/market/subscribe'

    try:
      instruments = ','.join(instruments)
      params = dict( instruments = instruments)
      res = requests.post(url, data=params, timeout=3)
      result = res.json().get('result')
    except:
      logger.debug(traceback.print_exc())
      return False
    return True

  def unsubInstrument(self, instruments):
    url = self.cfgs.get("http") + '/ctp/market/unsubscribe'

    try:
      instruments = ','.join(instruments)
      params = dict( instruments = instruments)
      res = requests.post(url, data=params, timeout=3)
      result = res.json().get('result')
    except:
      logger.debug(traceback.print_exc())
      return False
    return True

  def init(self,**cfgs):
    self.cfgs.update(**cfgs)
    self.cfgs['http'] = self.cfgs['trade_api.url']
    self.cfgs['access_code'] = self.cfgs.get('trade_api.access_code','')
    return self

  def open(self):
    return self

  def close(self):
    pass

@singleton
class TradeApi(object):
  """委托下单"""
  def __init__(self):
    self.cfgs = {}

  def init(self,**cfgs):
    self.cfgs.update(**cfgs)
    self.cfgs['http'] = self.cfgs['trade_api.url']
    self.cfgs['access_code'] = self.cfgs.get('trade_api.access_code','')

    return self

  def open(self):
    return self

  def close(self):
    pass

  def getPosition(self)->[ctp.Position]:
      """查询指定 代码或者指定策略的持仓记录"""
      url = self.cfgs.get("http") + '/ctp/position/list'

      positions = []
      try:
          res = requests.get(url,timeout=3)
          values = res.json().get('result', [])
          if not values:
              values = []
          for _ in values:
              pos = ctp.Position()
              object_assign(pos, _)
              pos.normalize()
              positions.append(pos)
      except:
          traceback.print_exc()
      return positions

  def getOrder(self):
    """查询委托信息
    """
    url = self.cfgs.get("http") + '/ctp/order/list'
    orders = []
    try:
      res = requests.get(url, timeout=3)
      values = res.json().get('result', [])
      # if not values:
      #   values = []
      # for _ in values:
      #   order = OrderRecord()
      #   object_assign(order, _)
      #   order.normalize()
      #   orders.append(order)
    except:
      traceback.print_exc()

    self.orders = OrderedDict()
    for order in orders:
      self.orders[order.user_order_id] = order

  def getTrade(self):
    url = self.cfgs.get("http") + '/ctp/trade/list'
    trades = []
    try:
      res = requests.get(url,timeout=3)
      values = res.json().get('result',[])
      if not values:
        values =[]
      for _ in values:
        tr = TradeReturn()
        object_assign(tr,_)
        tr.normalize()
        trades.append(tr)
    except:
        traceback.print_exc()

    self.trades = OrderedDict()
    for tr in trades:
      if not self.trades.has_key(tr.order_id):
        self.trades[tr.order_id] = []
      self.trades[tr.order_id].append(tr)

  def getAccountFunds(self):
    """查询资金账户"""
    url = self.cfgs.get("http") + '/ctp/account'
    try:
      res = requests.get(url, timeout=3.)
      data = res.json().get('result', {})
      self.funds = ctp.AccountStat()
      object_assign(self.stat, data)
      self.funds.normalize()
    except:
      print("Error: Request Trader Service Fail Down.")

  def sendOrder(self, order_req= OrderRequest(code='')):
    """发送订单
        :param: order_req : OrderRequest
    """
    order_req.exchange_id = get_exchange_id(order_req.code)
    if not order_req.exchange_id:
      return ''
    # 0 买入
    # 1 卖出
    Buy = "buy"
    Sell = "sell"
    direction = Buy
    if order_req.direction == Constants.Sell:
      direction = Sell
    # print order_req.dict()
    oc = "open"
    if order_req.oc in (Constants.Cover, Constants.Close):
      oc = "close"
    if order_req.oc == Constants.ForceClose:
      oc = "forceclose"
    if order_req.oc == Constants.CloseToday:
      oc = 'closetoday'
    if order_req.oc == Constants.CloseYesterday:
      oc = 'closeyesterday'

    url = self.cfgs.get("http") + '/ctp/order/send'

    order_id = ''
    cc = ctp.Constants.ContingentConditionType.Immediately
    # tc = ctp.Constants.TimeConditionType.IOC
    tc = ctp.Constants.TimeConditionType.GFD
    vc = ctp.Constants.VolumeConditionType.VC_AV

    cc = order_req.opts.get('cc', cc)
    tc = order_req.opts.get('tc', tc)
    vc = order_req.opts.get('vc', vc)

    access_code = self.cfgs.get('access_code', '')
    try:
      data = dict(instrument=order_req.code,
                  price=order_req.price,
                  volume=int(order_req.quantity),
                  direction=direction,
                  oc=oc,
                  price_type=order_req.price_type,
                  exchange_id=order_req.exchange_id,
                  cc=cc,
                  tc=tc,
                  vc=vc,
                  access_code=access_code
                  )
      res = requests.post(url, data)
      order_id = res.json().get('result')
    except:
      traceback.print_exc()
    return order_id

  def cancelOrder(self,order_id):
    """撤销订单"""
    order_id = order_id.replace('_',' ')
    url = self.cfgs.get("http") + '/ctp/order/cancel'
    try:
      access_code = self.cfgs.get('access_code', '')
      data = dict(order_id = order_id,
                  access_code =access_code
      )
      res = requests.post(url, data)
      order_id = res.json().get('result')
    except:
        traceback.print_exc()
    return order_id

