#coding:utf-8
import datetime

import fire
from typing import List,Dict
from collections import OrderedDict
from threading import RLock

from elxtrade.ctp import *
from elxtrade.linecheck import LineChecker

class Behavior(object):
  def __init__(self):
    self.cfgs = {}
    self.positions : Dict[str,List[Position]] ={} # symbol=>{ long, short}
    self.orders : Dict[str,OrderRecord] ={}    # id => OrderRecord
    self.acc_funds : AccountFunds = AccountFunds()
    self.ticks: Dict[str,Tick] = {}

    self.lock = RLock()



  def getTick(self,ins):
    with self.lock:
      return self.ticks.get(ins)

  def onTick(self,tick:Tick):
    with self.lock:
      if tick.LastPrice > 1.0e+20:
        return
      tick.normalize()
      self.ticks[ tick.InstrumentID ] = tick

  def onTradeReturn(self,data:TradeReturn):
    pass

  def onOrderReturn(self,r:OrderRecord):
    # with self.lock:
    #   self.orders[r.order_id] = r
    pass

  def onOrderErrorReturn(self,data:OrderError):
    pass

  def onOrderCancelErrorReturn(self,data:OrderCancelError):
    pass

  def onTradeSetReturn(self,set:TradeRecordSet):
    pass

  def onPositionSetReturn(self,set:PositionSet):
    LineChecker().pos_up_time = datetime.datetime.now()

    positions = {}
    for p in set.inner:
      if p.InstrumentID not in positions:
        positions[p.InstrumentID] = [Position(),Position()]
      if p.PosiDirection == Constants.PositionDirection.Long:
        positions[p.InstrumentID][0] = p
      elif p.PosiDirection == Constants.PositionDirection.Short:
        positions[p.InstrumentID][1] = p
    self.positions = positions

  def getPosition(self,ins,direction = Constants.PositionDirection.Long ) ->List[Position]:
    ret = [Position(),Position()] # [ long ,short ]
    if ins in self.positions:
      ret = self.positions[ins]
    return ret

  def onOrderSetReturn(self,set:OrderRecordSet):
    with self.lock:
      LineChecker().order_up_time = datetime.datetime.now()

      orders :Dict[str,OrderRecord] ={}
      for r in set.inner:
        orders[r.order_id] = r
      self.orders = orders

  def getQueueingOrders(self,ins='')->List[OrderRecord]:
    """查询指定合约的挂单"""
    with self.lock:
      orders:List[OrderRecord] = []
      for r in self.orders.values():
        if ins:
          if r.InstrumentID != ins:
            continue

        if r.OrderStatus in (Constants.OrderStatusType.PartTradedQueueing,
                           Constants.OrderStatusType.PartTradedNotQueueing,
                           Constants.OrderStatusType.NoTradeQueueing):
          orders.append(r)
      return orders

  def onAccountFunds(self,acc:AccountFunds):
    self.acc_funds = acc

  def getTradeApi(self):
    from elxtrade.broker import TradeApi
    return TradeApi()

  def getMarketApi(self):
    from elxtrade.broker import MarketApi
    return MarketApi()

  def ready(self,**kvs):
    self.cfgs.update(**kvs)
    if self.cfgs.get("twap.behavior.test",0):
      self.init_for_test()

  def init_for_test(self):
    """模拟测试数据，离线运行
      持仓 静态不变
      委托  空
      c++ / elxtrader 停止运行
    """
    LineChecker().pos_up_time = datetime.datetime.now()
    LineChecker().order_up_time = datetime.datetime.now()
    lp = Position()
    sp = Position()
    lp.Position = 0
    sp.Position = 10
    self.positions['CF201'] = [lp,sp]

    tick = Tick()
    tick.InstrumentID ='CF201'
    tick.LastPrice = 21830
    self.ticks[tick.InstrumentID] = tick

    def sendOrder(req):
      return "OrderId"

    def cancelOrder(order_id):
      return ""

    def subInstrument(inss):
      return True

    self.getTradeApi().sendOrder = sendOrder
    self.getTradeApi().cancelOrder = cancelOrder
    self.getMarketApi().subInstrument = subInstrument

  def stop(self):
    pass

  def onPositionSignal(self,symbol,pos):
    """外部通知持仓改变"""
    pass