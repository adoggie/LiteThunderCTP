#coding:utf-8

import fire
import csv
import os,sys
import json
import time
import datetime
import cmd
from threading import RLock,Thread,Condition,Event
from queue import Queue
# import gevent

# from gevent.time import


from elxtrade.fundamental.utils.useful import singleton,input_params
from elxtrade.controller import Controller
from elxtrade.behavior import Behavior
from elxtrade import logger
from elxtrade.registry_client import RegistryClient
from elxtrade.pos_receiver import PosReceiver
from elxtrade import ctp
from elxtrade.ctp import *
from elxtrade.broker import OrderRequest,TradeApi,MarketApi
from elxtrade.tradecmd import TradeCmd
from elxtrade.linecheck import LineChecker

PWD = os.path.dirname(os.path.abspath(__file__))
FN = os.path.join(PWD,  'settings.json')

import signal
import sys

class OrderFuture(object):
  def __init__(self , owner,ins):
    self.owner:OrderSequence = owner
    self.ins = ins
    self.start = 0   # start time
    self.end = 0
    self.threshold = 1  # 每次报单最小手数

    self.target = 0     # 目标仓位
    # self.finished = 0  # 已完成

    self.order_tick_inc = 1 # 重新追单次数 ，每次增加一个tick  todo
    self.reorder_wait = 0.1 # 追单等待间隔时间


    self.oc = ''
    self.num = 0
    self.direction = ''
    self.order_id = ''
    self.order_time = datetime.datetime.now()
    self.poll_time = datetime.datetime.now()

    self.behavior = TWAP_OrderingFuture()
    self.orign_pos = []
    self.target_pos = []

    self.next_poll_time = None
    self.last_order_time = None

  @property
  def finished(self):
    return self.num - self.unfinished


  @property
  def unfinished(self):
    """剩余未成交数量"""
    untrade = 0
    # traded = 0
    if not self.target_pos:
      return self.num

    cur_pos = self.behavior.getPosition(self.ins)
    lp,sp = cur_pos[0].Position,cur_pos[1].Position
    if self.oc == Constants.Open:
      if self.direction == Constants.Buy:
        untrade =  self.target_pos[0] - lp
      else: #
        untrade =  self.target_pos[1] - sp
    else: # Close
      if self.direction == Constants.Buy:
        untrade = sp - self.target_pos[1]
      else:
        untrade = lp - self.target_pos[0]

    if untrade < 0 :
      print('--- WARN: untrade:',untrade)
      # return 0
    # untrade = self.num - traded
    return abs(untrade)


  def destory(self):
    # 撤单
    behavior = TWAP_OrderingFuture()
    orders = behavior.getQueueingOrders(self.ins)
    logger.debug("OrderFuture::destory() ", self.ins, "cancelOrder() , unfinished:",
                 self.unfinished, 'order_id:', self.order_id,
                 'start:', str(self.start), 'end:', str(self.end))
    for r in orders:
      logger.debug("==xx==OrderFuture::destroy() ",self.ins ," order_id:",
                   r.order_id,
                   'start:',str(self.start) ,'end:',str(self.end))
      behavior.getTradeApi().cancelOrder(r.order_id)

  def fill_target_pos(self):
    lp, sp = self.behavior.getPosition(self.ins)
    self.orign_pos = [lp.Position, sp.Position]

    self.target_pos = [0, 0]
    if self.oc == Constants.Open:
      if self.direction == Constants.Buy:
        self.target_pos[0] = self.num + self.orign_pos[0]
      else:  #
        self.target_pos[0] = self.num + self.orign_pos[1]
    else:  # Close
      if self.direction == Constants.Buy:
        self.target_pos[0] = self.orign_pos[0] - self.num
      else:
        self.target_pos[1] = self.orign_pos[1] - self.num

  def poll(self):
    if not self.orign_pos:
      lp,sp = self.behavior.getPosition(self.ins)
      self.orign_pos = [lp.Position,sp.Position]

      self.target_pos = [0,0]
      if self.oc == Constants.Open:
        if self.direction == Constants.Buy:
          self.target_pos[0] = self.num + self.orign_pos[0]
        else:  #
          self.target_pos[1] = self.num + self.orign_pos[1]
      else:  # Close
        if self.direction == Constants.Buy:
          self.target_pos[0] = abs(self.orign_pos[0] - self.num)
        else:
          self.target_pos[1] = abs(self.orign_pos[1] - self.num)

    if self.unfinished == 0:  # 已全部完成
      return

    # 限流
    now = datetime.datetime.now()
    if not self.next_poll_time:
      self.next_poll_time = now + datetime.timedelta(seconds=1)
    else:
      if self.next_poll_time > now:
        return

    self.next_poll_time = now + datetime.timedelta(seconds=1)

    if self.order_id:
      # 查询 成功报单记录是否返回 ，必须全成,否则撤单
      if self.unfinished :
        # 等待2s 检查持仓状况
        wait_time = self.owner.behavior.cfgs.get("twap.behavior.trade_wait_timeout")
        if self.last_order_time + datetime.timedelta(seconds=wait_time) < now : # 2s 未成交 ，撤单
          # 单子撤掉
          logger.debug("OrderFuture::poll ",self.ins, "cancelOrder() , unfinished:" ,
                       self.unfinished , 'order_id:',self.order_id,
                       'start:',str(self.start) ,'end:',str(self.end))
          for r in self.behavior.getQueueingOrders(self.ins):
            self.behavior.getTradeApi().cancelOrder(r.order_id)
          self.order_id = ''

      return

    # 发起下单委托
    num = self.unfinished
    tick = None

    tick = self.behavior.getTick(self.ins)
    if not tick:
      logger.error("OrderFuture::poll() , tick is null , skipped!" , self.ins)
      return

    quant = int(abs(num))

    direction = ''
    if self.direction in ('long', 'buy'):
      direction = Constants.Buy
    elif self.direction in ('short', 'sell'):
      direction = Constants.Sell

    insinfo = get_insrinfo(self.ins)
    if not insinfo:
      print("Instrument not defined !")
      return

    price_type = self.owner.behavior.cfgs.get("twap.behavior.price_type","last")
    price = 0
    if price_type == 'last':
      price = tick.LastPrice    # 暂时用最新价格报单

    if price_type == 'market':
      if not tick.Bids[0] or not tick.Asks[0]:
        logger.error("OrderFuture::poll() , Bids/Asks is nil , skipped!", self.ins)
        return

      if self.oc == Constants.Open:
        if self.direction == Constants.Sell:
          price = tick.Bids[0]   # 高一个tick
        elif self.direction == Constants.Buy:
          price = tick.Asks[0]
      elif self.oc == Constants.Close:
        if self.direction == Constants.Sell:
          price = tick.Bids[0]
        elif self.direction == Constants.Buy:
          price = tick.Asks[0]



    self.order_tick_inc = self.behavior.cfgs.get("twap.behavior.order_price_float_tick",1)

    if self.oc == Constants.Open :
      if  self.direction == Constants.Sell:
        price -= insinfo['ticksize'] *self.order_tick_inc  # 高一个tick
      elif self.direction == Constants.Buy:
        price += insinfo['ticksize'] *self.order_tick_inc # 低一个tick todu.
    elif self.oc == Constants.Close:
      if self.direction == Constants.Sell:
        price -= insinfo['ticksize'] *self.order_tick_inc
      elif self.direction == Constants.Buy:
        price += insinfo['ticksize'] *self.order_tick_inc

    if price < tick.LowerLimitPrice:
      price = tick.LowerLimitPrice
    if price > tick.UpperLimitPrice:
      price = tick.UpperLimitPrice
    if not price or price > 1.7e+100  or price < tick.LowerLimitPrice or price > tick.UpperLimitPrice:
      logger.error("OrderFuture::poll() , price  invalid , skipped!" , self.ins, price,'|',tick.LowestPrice,tick.HighestPrice)
      return

    oc = ''
    if self.oc == 'open':
      oc = Constants.Open
    elif self.oc == 'close':
      oc = Constants.Close

    req = OrderRequest(self.ins, price, quant, direction, oc)
    self.order_id = self.behavior.getTradeApi().sendOrder(req)

    self.last_order_time = datetime.datetime.now()

    self.order_tick_inc += 1

    logger.debug("OrderFuture::poll -> sendOrder() ",self.ins,'lastprice:',tick.LastPrice,
                 'price:',price, 'num:',quant,'direction:',direction,'oc:',oc,
                 'start:',str(self.start) ,'end:',str(self.end) )
    logger.debug(">> ",tick.LastPrice, " Bids:",tick.Bids , " Asks:",tick.Asks)
    logger.debug(">> order_id:", self.order_id)


class OrderSequence(object):
  def __init__(self,behavior:Behavior,ins):
    self.behavior = behavior
    self.ins = ins   # 合约名称
    # self.stride = stride    # 时间幅度 （ blanks ）
    # self.blanks= OrderedDict()    # { 9:30 => OrderTimeBlank( start=9:30 , end =9:31 )  }
    self.futures:List[OrderFuture] = []
    self.target_pos = 0   # 目标仓位，过程中不断变化，不改变当前运行的future的目标仓位

  def poll(self):
    """当前时间检查触发仓位下单"""
    if not self.futures:
      return # no futures
    now = datetime.datetime.now()

    f = self.futures[0]
    if f.end < now :
      logger.debug("OrderSequence::poll() , timeout and remove it !")
      self.futures.pop(0)
      f.destory()
      if not self.futures:
        self.realloc(self.pos_target)
      return

    f.poll()
    self.realloc(self.pos_target)


  def pos_changed(self,pos):
    """仓位变动,分散到stride个futures"""
    # if self.ins !='SF201':
    #   return

    if not LineChecker().pos_up_time:
      return
    self.pos_target = pos
    logger.debug("pos_changed() ",self.ins, ' new pos:',pos )

    self.realloc(self.pos_target)

  def get_current_pos(self):
    # 多空仓位 总和 long - short
    # 当前仓位 + futures[0] 的未完成部分
    a, b = self.behavior.getPosition(self.ins)
    # ? 细节：平今，平昨

    lp = a.Position # 多仓
    sp = b.Position # 空仓

    if self.futures:
      f = self.futures[0]
      if f.oc == Constants.Open:
        if f.direction == Constants.Buy:
          lp -= f.finished
        if f.direction == Constants.Sell:
          sp -= f.finished
      elif f.oc == Constants.Close:
        if f.direction == Constants.Buy:
          sp -= f.finished
          f.destory()
        elif f.direction == Constants.Sell:
          lp -= f.finished
          f.destory()

    pos = lp - sp
    return pos

  def make_futures(self,oc,direction,pos,start=None):
    STEP = 2
    DURATION = 5*10
    STEP = self.behavior.cfgs.get("twap.make_futures.step",1)
    DURATION = self.behavior.cfgs.get("twap.make_futures.duration",10)
    ps = abs(pos)
    now = datetime.datetime.now()
    next = now
    if start:
      next = start

    futures = []
    if self.futures:
      next = self.futures[0].end
      futures = self.futures[:1]
      ps -= self.futures[0].num

    while ps > 0:
      f = OrderFuture(self,self.ins)
      futures.append(f)
      f.oc = oc
      f.direction = direction
      f.num = min(STEP,ps)
      f.start = next
      f.end = f.start + datetime.timedelta(seconds= DURATION)
      next = f.end
      ps -= STEP
    return futures

  def realloc(self,target_pos):
    M = self.get_current_pos()
    N = target_pos
    D = N - M  # 仓位差
    # if self.ins == 'pp2201':
    #   print()

    if D == 0:
      if self.futures:
        f = self.futures[0]
        f.destory()
      self.futures = []
      return

    if M * N > 0 : # 同向
      if M > 0:  # +5,+8
        if D > 0: # 5,8
          D = int(abs(D))
          fs = self.make_futures(Constants.Open,Constants.Buy,D)
          self.futures = fs
          # if not self.futures:
          #   self.futures = fs
          # else: # 添加到尾部未执行时间区域
          #   self.futures[:1] += fs
        elif D < 0: # 8,5
          D = int(abs(D))
          fs  = self.make_futures(Constants.Close, Constants.Sell, D)
          self.futures = fs
          # if not self.futures:
          #   self.futures = fs
          # else:
          #   self.futures[:1] += fs

      ######
      if M < 0: # -5,-8
        if D > 0 :  # -8,-5
          D = int(abs(D))
          fs = self.make_futures(Constants.Close, Constants.Buy, D)
          self.futures = fs
          # if not self.futures:
          #   self.futures = fs
          # else:
          #   self.futures[:1] += fs
        elif D < 0 : # -5,-8
          D = int(abs(D))
          fs = self.make_futures(Constants.Open, Constants.Sell, D)
          self.futures = fs
          # if not self.futures:
          #   self.futures = fs
          # else:
          #   self.futures[:1] += fs
    ## end M*N > 0
    if M * N  < 0:  # -5 , 5  OR  5,-5  不同向
      if M > 0 : # 5,-5
        fs1= self.make_futures(Constants.Close, Constants.Sell, int(abs(M)))
        self.futures = []
        # if not self.futures:
        #   self.futures = fs
        # else:
        #   self.futures[:1] += fs
        start = None
        if fs1:
          start = fs1[-1].end
        fs2 = self.make_futures(Constants.Open, Constants.Sell, int(abs(N)), start )
        self.futures =  fs1 + fs2
      if M < 0: # -5, 5

        fs1 = self.make_futures(Constants.Close, Constants.Buy, int(abs(M)))
        self.futures = []
        # if not self.futures:
        #   self.futures = fs
        # else:
        #   self.futures[:1] += fs
        start = None
        if fs1:
          start = fs1[-1].end

        fs2 = self.make_futures(Constants.Open, Constants.Buy, int(abs(M)),start)
        self.futures = fs1 + fs2
    ## end M*N < 0
    if M * N == 0 :
      if M !=0:
        fs = []
        if M < 0 :  # -5 ,0
          fs = self.make_futures(Constants.Close, Constants.Buy, int(abs(M)))
        elif M > 0: # 5 , 0
          fs = self.make_futures(Constants.Close, Constants.Sell, int(abs(M)))
        self.futures = fs
        # if not self.futures:
        #   self.futures = fs
        # else:
        #   self.futures[:1] += fs
      else:
        fs = []
        if N < 0: # 0, -5
          fs = self.make_futures(Constants.Open,Constants.Sell, int(abs(N)))
        elif N > 0: # 0, 5
          fs = self.make_futures(Constants.Open ,Constants.Buy , int(abs(N)))
        self.futures = fs
        # if not self.futures:
        #   self.futures = fs
        # else:
        #   self.futures[:1] += fs

    logger.debug("realloc() ",self.ins, '(',M , '->',N,') D:',D)
    for f in self.futures:
      logger.debug("OrderSequence::realloc() ,",f.ins, f.num,f.direction,f.oc,str(f.start) , str(f.end) )
    logger.debug('-'*20)

@singleton
class TWAP_OrderingFuture(Behavior,TradeCmd):
  """help
    show pos
    show order q
  """
  prompt = 'elxtrader > '
  intro = 'welcome to elabs..'
  def __init__(self):
    Behavior.__init__(self)
    # cmd.Cmd.__init__(self )
    TradeCmd.__init__(self)

    self.real_pos = {}      # 实际仓位
    self.target_pos  = {}   # 目标仓位
    self.running = True
    self.trades = {}        # 成交记录
    self.ins_tick = {}      # 合约最新tick
    self.order_seqs :Dict[str,OrderSequence]= {} #OrderSequence()

  def ready(self,**kvs):
    Behavior.ready(self,**kvs)
    pass

  def open(self):
    # 行情订阅
    ins = RegistryClient().ins.keys()
    # ins = ['CF201']  # 测试合约

    mdapi = self.getMarketApi()
    mdapi.subInstrument(ins)

    # 准备合约
    for ins_ in ins:
      self.order_seqs[ins_] = OrderSequence(self,ins_)

    # 设置仓位接收
    PosReceiver().addUser(self)


    if self.cfgs.get("twap.behavior.polling"):
      logger.debug("start twap behavior polling....")
      self.thread = Thread(target=self.polling)
      self.thread.daemon = True
      self.thread.start()

  def polling(self):
    self.running = True

    # print("waiting for position query result ..")
    # while self.running:
    #   time.sleep(.1)
    #   if not LineChecker().pos_up_time:
    #     continue
    #   print(">> Position ready!")
    #   break
    #
    # #
    # print("Wait for order query result..")
    # while self.running:
    #   time.sleep(.1)
    #   if not LineChecker().order_up_time:
    #     continue
    #   print(">> Orders ready!")
    #   break

    # cancal all untrade orders
    for r in self.getQueueingOrders():
      self.getTradeApi().cancelOrder(r.order_id)
    print('All unTrade Orders have been discard!')

    if self.cfgs.get("registry_client.init_pos_when_startup",0):
      print('Start update All instrument position..')
      inspos_list = RegistryClient().ins_pos
      for name,pos in inspos_list.items():
        self.onPositionSignal(name,pos)

    print("MainLoop Entering...")

    intr = self.cfgs.get("twap.behavior.polling.interval",0.1)
    while self.running:
      time.sleep(1)

      # check trade time
      if self.cfgs.get("twap.trade_time.check",0):
        tr = self.cfgs.get("twap.trade_time_range",[])
        now = datetime.datetime.now().time()
        polling = False
        for s,e in tr:
          # ds = datetime.datetime(now.year,now.month,now)
          ds = datetime.time(hour=s[0],minute=s[1])
          de = datetime.time(hour=e[0],minute=e[1])
          if ds<= now < de:
            polling = True
            break
        if not polling:
          continue
      # end check time
      with self.lock:
        for name,osq in self.order_seqs.items():
          # if 'PP2201' == name:
          #   osq.poll()
          osq.poll()


  def close(self):
    self.running = False
    self.thread.join()

  def do_pos(self,line):
    """pos emit CF201 11
       持仓信号改变
    """
    args = input_params(line,['emit'],2)
    if args:
      ins,num = args
      num = int(num)
      self.onPositionSignal(ins,num )

  def onPositionSignal(self,symbol:str,pos:float):
    """指定商品仓位改变"""
    with self.lock:
      ins = RegistryClient().get_ins_by_product(symbol)
      if not ins:
        return
      '''
      {'id': 303, 'stockid': 77, 'pz': 'lu', 'instrumentid': 'lu2202', 'percentage': 100.0, 'target': 100.0,
       'tradetype': 'closey_closet', 'maxposition': 9999999, 'nighttime': 230000, 'exname': 'INE', 'tick': 1.0,
       'chengshu': 10, 'addtick': 1, 'maxopen': 9999999}
       '''
      ins_id = ins['instrumentid']
      orsq = self.order_seqs.get(ins_id)
      if orsq:
        orsq.pos_changed(pos)

  def onTick(self,data:Tick):
    Behavior.onTick(self,data)

  def onTradeReturn(self,data:TradeReturn):
    pass

  def onOrderReturn(self,order: OrderRecord):

    order_id = order.user_order_id
    if order_id not in self.orders:
      self.orders[order_id] = order
      return

  def onOrderErrorReturn(self,data: OrderError):
    pass

  def onOrderCancelErrorReturn(self,data: OrderCancelError):
    pass

  def onTradeSetReturn(self,set:TradeRecordSet):
    Behavior.onTradeSetReturn(self,set)
    # print('tradeset:',len(set.inner))


  def onPositionSetReturn(self,set:PositionSet):
    Behavior.onPositionSetReturn(self, set)
    return


  def onOrderSetReturn(self,set:OrderRecordSet):
    Behavior.onOrderSetReturn(self,set)
    # print('ord_set:', len(set.inner))
    pass



  def signal_handler(signal, frame):
    sys.exit(0)



def signal_handler(signal,frame):
  TWAP_OrderingFuture().close()
  Controller().close()
  print('bye bye!')
  sys.exit(0)

#------------------------------------------

def run(fn='',noprompt=False):
  global FN
  if fn:
    FN = fn
  params = json.loads(open(FN).read())

  Controller().init(**params).addBehavior("twap",TWAP_OrderingFuture()).open()
  if noprompt:
    signal.signal(signal.SIGINT, signal_handler)
    print("")
    print("~~ Press Ctrl+C to kill .. ~~")
    while True:
      time.sleep(1)
  else:
    TWAP_OrderingFuture().cmdloop()


if __name__ == '__main__':
  fire.Fire()