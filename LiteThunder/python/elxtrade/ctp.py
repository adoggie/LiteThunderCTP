#coding:utf-8

import fire
from typing import List,Dict,Sequence
from elxtrade.fundamental.utils.useful import hash_object, object_assign
from elxtrade import instrinfo

class Product(object):
    def __init__(self):
      self.name = ''        # 'A,M,..'
      self.tick = 0         # 最小变动价位
      self.multiple = 0     # 合约数量乘数
      self.commission = 0   # 手续费
      self.margin = [0,0]     # 保证金,多头和空头

class Constants(object):
  Buy = 'long'
  Sell = 'short'
  Open = 'open'
  Cover = 'cover'
  Close = 'close'
  ForceClose = 'forceclose'
  CloseToday = 'closetoday'
  CloseYesterday = 'closeyesterday'
  Long = 'long'
  Short = 'short'
  Idle = 'idle'

  ARB = 'ARB'  # 套利
  HEDGE = 'HEDGE'  # 套保
  SPEC = 'SPEC'  # 投机

  class MarketOrder(object):
    A = 'A'  # 即时成交
    B = 'B'  # 最优5挡
    C = 'C'  # 全额成交
    D = 'D'  # 本方最优价格
    E = 'E'  # 对手最优价格

  class OrderStatus(object):
    Unknown = 0
    Auditing = 1  # 审批中
    AuditError = 2  # 审批失败
    Registered = 3  # 已报
    Pending_Dealing = 4  # 已报交易所，等待成交
    Rejected = 5  # 拒绝
    Pending_Cancel = 6  # 撤单，待交易所确认
    Cancelled = 7  # 撤单已完成
    Partial_Pending_Cancel = 8  # 部分成交，等待撤单中
    Partial_Cancelled = 9  # 部分成交，且撤单完成
    Partial_Filled = 10  # 部分成交
    Fully_Filled = 11  # 全部成交

  class TypeBase(object):
    reverseMap = {}
    @classmethod
    def name(cls, value):
      if isinstance(value, (int)):
        value = chr(value)
      return cls.reverseMap.get(value, '')

  class OffsetType(object):
    Open = ord('0')
    Close = ord('1')
    ForceClose = ord('2')
    CloseToday = ord('3')
    CloseYesterday = ord('4')
    ForceOff = ord('5')
    LocalForceClose = ord('6')

    reverseMap = {
      '0':'Open',
      '1': 'Close',
      '2': 'ForceClose',
      '3': 'CloseToday',
      '4': 'CloseYesterday',
      '5': 'ForceOff',
      '6': 'LocalForceClose'
    }

  class DirectionType(object):
    # 买卖方向类型
    Buy = ord('0')
    Sell = ord('1')
    reverseMap = { '0':'Buy','1':'Sell'}

  class PositionDirection(object):
    Net = '1'
    Long = '2'
    Short = '3'
    reverseMap = { '1':'Net', '2':'Long' , '3': 'Short' }

  class HedgeFlagType(object):
    Speculation = '1'  # 投机
    Arbitrage = '2'  # 套利
    Hedge = '3'  # 套保
    Covered = '4'  # 备兑

    reverseMap = { '1':'Speculation', '2':'Arbitrage' , '3':'Hedge' , '4':'Covered' }

  class PositionDate(object):
    Today = '1'  # 今仓
    History = '2'  # 昨仓

    reverseMap = { '1':'Today' , '2':'History'}


  class TimeConditionType(object):  # 有效期类型类型
    # 立即完成，否则撤销
    # simnow 委托下单时仅仅支持IOC ，即刻成交，其他时间控制均不支持
    # 实盘支持任意时间段 。
    #
    # 上期SHFE 平仓要指定平今还是平昨
    IOC = ('1')
    # 本节有效
    GFS = ('2')
    # 当日有效
    GFD = ('3')
    # 指定日期前有效
    GTD = ('4')
    # 撤销前有效
    GTC = ('5')
    # 集合竞价有效
    GFA = ('6')

    reverseMap = { '1':'IOC' , '2': 'GFS', '3':'GFD','4':'GTD','5':'GTC','6':'GFA' }

  class VolumeConditionType(object):  # 成交量类型类型
    # 任何数量
    VC_AV = ('1')
    # 最小数量
    VC_MV = ('2')
    # 全部数量
    VC_CV = ('3')
    reverseMap = { '1':'VC_AV','2':'VC_MV' , '3':'VC_CV'}

  class ContingentConditionType(object):  # 触发条件类型
    # 立即
    Immediately =('1')
    # 止损
    Touch = ('2')
    # 止赢
    TouchProfit = ('3')
    # 预埋单
    ParkedOrder = ('4')
    # 最新价大于条件价
    LastPriceGreaterThanStopPrice = ('5')
    # 最新价大于等于条件价
    LastPriceGreaterEqualStopPrice = ('6')
    # 最新价小于条件价
    LastPriceLesserThanStopPrice = ('7')
    # 最新价小于等于条件价
    LastPriceLesserEqualStopPrice = ('8')
    # 卖一价大于条件价
    AskPriceGreaterThanStopPrice = ('9')
    # 卖一价大于等于条件价
    AskPriceGreaterEqualStopPrice = ('A')
    # 卖一价小于条件价
    AskPriceLesserThanStopPrice = ('B')
    # 卖一价小于等于条件价
    AskPriceLesserEqualStopPrice = ('C')
    # 买一价大于条件价
    BidPriceGreaterThanStopPrice = ('D')
    # 买一价大于等于条件价
    BidPriceGreaterEqualStopPrice = ('E')
    # 买一价小于条件价
    BidPriceLesserThanStopPrice = ('F')
    # 买一价小于等于条件价
    BidPriceLesserEqualStopPrice = ('H')

  class ForceCloseReason(object):  # 强平原因类型
    # 非强平
    NotForceClose = ord('0')
    # 资金不足
    LackDeposit = ord('1')
    # 客户超仓
    ClientOverPositionLimit = ord('2')
    # 会员超仓
    MemberOverPositionLimit = ord('3')
    # 持仓非整数倍
    NotMultiple = ord('4')
    # 违规
    Violation = ord('5')
    # 其它
    Other = ord('6')
    # 自然人临近交割
    PersonDeliv = ord('7')

  class OrderPriceType(object):
    # 任意价
    AnyPrice = ord('1')
    # 限价
    LimitPrice = ord('2')  # simnow 仅支持限价单，其他不支持
    # 最优价
    BestPrice = ord('3')
    # 最新价
    LastPrice = ord('4')
    # 最新价浮动上浮1个ticks
    LastPricePlusOneTicks = ord('5')
    # 最新价浮动上浮2个ticks
    LastPricePlusTwoTicks = ord('6')
    # 最新价浮动上浮3个ticks
    LastPricePlusThreeTicks = ord('7')
    # 卖一价
    AskPrice1 = ord('8')
    # 卖一价浮动上浮1个ticks
    AskPrice1PlusOneTicks = ord('9')
    # 卖一价浮动上浮2个ticks
    AskPrice1PlusTwoTicks = ord('A')
    # 卖一价浮动上浮3个ticks
    AskPrice1PlusThreeTicks = ord('B')
    # 买一价
    BidPrice1 = ord('C')
    # 买一价浮动上浮1个ticks
    BidPrice1PlusOneTicks = ord('D')
    # 买一价浮动上浮2个ticks
    BidPrice1PlusTwoTicks = ord('E')
    # 买一价浮动上浮3个ticks
    BidPrice1PlusThreeTicks = ord('F')
    # 五档价
    FiveLevelPrice = ord('G')
    # 本方最优价
    BestPriceThisSide = ord('H')

  # 报单状态类型
  class OrderStatusType(object):
    # 全部成交
    AllTraded = ord('0') # 48
    # 部分成交还在队列中
    PartTradedQueueing = ord('1') # 49
    # 部分成交不在队列中
    PartTradedNotQueueing = ord('2') # 50
    # 未成交还在队列中
    NoTradeQueueing = ord('3') # 51
    # 未成交不在队列中
    NoTradeNotQueueing = ord('4') #53
    # 撤单
    Canceled = ord('5')  # 54
    # 未知
    Unknown = ord('a')  # 97
    # 尚未触发
    NotTouched = ord('b') # 98
    # 已触发
    Touched = ord('c')  # 99

  # 是一个报单提交状态类型 INT
  class OrderSubmitStatusType(object):
    # 已经提交
    InsertSubmitted = ord('0') # 48
    # 撤单已经提交
    CancelSubmitted = ord('1') # 49
    # 修改已经提交
    ModifySubmitted = ord('2') # 50
    Accepted = ord('3')       # 51
    # 报单已经被拒绝
    InsertRejected = ord('4')  #52
    # 撤单已经被拒绝
    CancelRejected = ord('5')  # 53
    # 改单已经被拒绝
    ModifyRejected = ord('6')  # 54

  class TradeEventType(object):
    EventTrade = 'event_trade' #成交事件
    EventOrder = 'event_order' #委託事件
    EventCancel = 'event_cancel' #委託撤消

    EventError = 'event_error'

class AccountFunds(object):
  """CThostFtdcTradingAccountField"""

  def __init__(self):
    # base.AccountStat.__init__(self)
    self.BrokerID = ''  # 经纪公司代码
    self.AccountID = 0  # 投资者帐号
    self.Deposit = 0  # 入金金额
    self.Withdraw = 0  # 出金金额
    self.FrozenMargin = 0  # 冻结的保证金
    self.FrozenCash = 0  # 冻结的资金
    self.CurrMargin = 0  # 当前保证金总额
    self.CloseProfit = 0  # 平仓盈亏
    self.PositionProfit = 0  # 持仓盈亏
    self.Balance = 0  # 期货结算准备金
    self.Available = 0  # 可用资金
    self.WithdrawQuota = 0  # 可取资金
    self.TradingDay = 0  # 交易日
    self.SettlementID = 0  # 结算编号

  def normalize(self):
    return self



class Tick(object):
  def __init__(self):

    self.InstrumentID = ''
    self.AveragePrice = 0  # 当日均价
    self.HighestPrice = 0  # 最高价
    self.UpperLimitPrice = 0
    self.LowerLimitPrice = 0  # 跌停板价
    self.Bids = []          # bid 买入价
    self.OpenPrice = 0  # 今开盘
    self.PreOpenInterest = 0  # 昨持仓量
    self.Volume = 0  # 数量
    self.Asks =  []
    self.PreClosePrice = 0  # 昨收盘
    self.PreSettlementPrice = 0  # 上次结算价
    self.UpdateTime = None  # 最后修改时间
    self.UpdateMillisec = 0  # 最后修改毫秒

    self.BidVols = []
    self.AskVols = []
    self.ClosePrice = 0
    self.ExchangeID = 0
    self.TradingDay = None  # 交易日
    self.PreDelta = 0
    self.OpenInterest = 0  # 持仓量
    self.CurrDelta = 0  # 今虚实度
    self.Turnover = 0  # 成交金额
    self.LastPrice = 0  # 最新价
    self.SettlementPrice = 0  # 本次结算价
    self.ExchangeInstID = 0  # 最高价
    self.LowestPrice = 0  # 最低价
    self.ActionDay = None
    self.DateTime = None

  @property
  def datetime(self):
    self.ActionDay = None
    self.DateTime = None
    return ''

  def normalize(self):
    self.Asks = list(map( normal_price, self.Asks))
    self.Bids = list(map( normal_price, self.Bids))
    self.AskVols = list(map(normal_price, self.AskVols))
    self.BidVols = list(map(normal_price, self.BidVols))

  def assginText(self,text):
    """InstrumentID,DateTime,Bids,BidVols,Asks,AskVols,AveragePrice,ActionDay,LastPrice,LowerLimitPrice,
      UpperLimitPrice,OpenInterest,Timestamp,Turnover,Volume"""
    fs = text.split(",")
    self.InstrumentID,self.DateTime,self.Bids,self.BidVols,self.Asks,self.AskVols,self.AveragePrice,self.ActionDay,\
    self.LastPrice,self.LowerLimitPrice,\
    self.UpperLimitPrice,self.OpenInterest,self.Timestamp,self.Turnover,self.Volume = fs[:15]

    self.Bids = list(map(float,self.Bids.split('#')))
    self.Asks = list(map(float,self.Asks.split('#')))
    self.BidVols = list(map(float,self.BidVols.split('#')))
    self.AskVols = list(map(float,self.AskVols.split('#')))
    self.AveragePrice = float(self.AveragePrice)
    self.LastPrice = float(self.LastPrice)
    self.LowerLimitPrice = float(self.LowerLimitPrice)
    self.UpperLimitPrice = float(self.UpperLimitPrice)
    self.OpenInterest = float(self.OpenInterest)
    self.Timestamp = int(self.Timestamp)
    self.Turnover = float(self.Turnover)
    self.Volume = float(self.Volume)



class Position(object):
  """持仓记录"""
  def __init__(self):

    self.InstrumentID = ''
    self.PosiDirection = ''
    self.HedgeFlag = ''
    self.PositionDate = 0
    self.YdPosition = 0  # 昨苍
    self.Position = 0  # 今日总持仓 YdPosition + TodayPosition
    self.LongFrozen = 0  # 多头冻结
    self.ShortFrozen = 0  # 空头冻结
    self.LongFrozenAmount = 0  # 开仓冻结金额
    self.ShortFrozenAmount = 0  # 空仓冻结金额
    self.OpenVolume = 0  # 开仓量
    self.CloseVolume = 0  # 平仓量
    self.OpenAmount = 0  # 开仓金额
    self.CloseAmount = 0  # 平仓金额
    self.PositionCost = 0  # 持仓成本 持仓均价
    self.PreMargin = 0  # 上次占用的保证金
    self.UseMargin = 0  # 占用的保证金
    self.FrozenMargin = 0  # 冻结的保证金
    self.FrozenCash = 0  # 冻结的资金
    self.FrozenCommission = 0  # 冻结的手续费
    self.CashIn = 0  # 资金差额
    self.Commission = 0  # 手续费
    self.CloseProfit = 0  # 平仓盈亏
    self.PositionProfit = 0  # 持仓盈亏
    self.TradingDay = 0  # 交易日
    self.SettlementID = 0  # 结算编号
    self.OpenCost = 0  # 开仓成本
    self.ExchangeMargin = 0  # 交易所保证金
    self.TodayPosition = 0  # 今日持仓
    self.MarginRateByMoney = 0  # 保证金率
    self.MarginRateByVolume = 0  # 保证金率(按手数)
    self.ExchangeID = 0  # 交易所代码
    self.YdStrikeFrozen = 0  # 执行冻结的昨仓

  def normalize(self):
    self.YdPosition = int(self.YdPosition)
    self.Position = int(self.Position)
    self.LongFrozen = float(self.LongFrozen)  # 多头冻结
    self.ShortFrozen = float(self.ShortFrozen)  # 空头冻结
    self.LongFrozenAmount = float(self.LongFrozenAmount)  # 开仓冻结金额
    self.ShortFrozenAmount = float(self.ShortFrozenAmount)  # 空仓冻结金额
    self.OpenVolume = int(self.OpenVolume)  # 开仓量
    self.CloseVolume = int(self.CloseVolume)  # 平仓量
    self.OpenAmount = float(self.OpenAmount)  # 开仓金额
    self.CloseAmount = float(self.CloseAmount)  # 平仓金额
    self.PositionCost = float(self.PositionCost)  # 持仓成本 持仓均价
    self.PreMargin = float(self.PreMargin)  # 上次占用的保证金
    self.UseMargin = float(self.UseMargin)  # 占用的保证金
    self.FrozenMargin = float(self.FrozenMargin)  # 冻结的保证金
    self.FrozenCash = float(self.FrozenCash)  # 冻结的资金
    self.FrozenCommission = float(self.FrozenCommission)  # 冻结的手续费
    self.CashIn = float(self.CashIn)  # 资金差额
    self.Commission = float(self.Commission)  # 手续费
    self.CloseProfit = float(self.CloseProfit)  # 平仓盈亏
    self.PositionProfit = float(self.PositionProfit)  # 持仓盈亏
    self.TradingDay = self.TradingDay  # 交易日
    # self.SettlementID = 0  # 结算编号
    self.OpenCost = float(self.OpenCost)  # 开仓成本
    self.ExchangeMargin = float(self.ExchangeMargin)  # 交易所保证金
    self.TodayPosition = float(self.TodayPosition)  # 今日持仓
    self.MarginRateByMoney = float(self.MarginRateByMoney)  # 保证金率
    self.MarginRateByVolume = float(self.MarginRateByVolume)  # 保证金率(按手数)
    # self.ExchangeID = 0  # 交易所代码
    self.YdStrikeFrozen = float(self.YdStrikeFrozen)  # 执行冻结的昨仓
    return self

  def __str__(self):
    return "Dir:%s , Position:%s"%(self.PosiDirection,self.Position)


class TradeReturn(object):
  def __init__(self):

    self.InstrumentID = ''  # 合约代码
    self.OrderRef = 0  # 报单引用
    self.UserID = ''  # 用户代码
    self.ExchangeID = ''  # 交易所代码
    self.TradeID = ''  # 成交编号
    self.Direction = ''  # 买卖方向
    self.OrderSysID = ''  # 报单编号
    self.ParticipantID = ''  # 会员代码
    self.ClientID = ''  # 客户代码
    self.TradingRole = ''  # 交易角色
    self.ExchangeInstID = ''  # 合约在交易所的代码
    self.OffsetFlag = ''  # 开平标志
    self.HedgeFlag = ''  # 投机套保标志
    self.Price = 0  # 价格
    self.Volume = 0  # 数量
    self.TradeDate = ''  # 成交时期
    self.TradeTime = ''  # 成交时间
    self.TradeType = ''  # 成交类型
    self.PriceSource = ''  # 成交价来源
    self.TraderID = ''  # 交易所交易员代码
    self.OrderLocalID = ''  # 本地报单编号
    self.ClearingPartID = ''  # 结算会员编号
    self.BusinessUnit = ''  # 业务单元
    self.SequenceNo = ''  # 序号
    self.TradingDay = ''  # 交易日
    self.SettlementID = ''  # 结算编号
    self.BrokerOrderSeq = ''  # 经纪公司报单编号
    self.TradeSource = ''  # 成交来源

  @property
  def order_id(self):
    """获取订单委托编号"""
    ids = 'B#{}#{}'.format(self.ExchangeID, self.OrderSysID)
    ids = ids.replace(' ','_')
    return ids

  def normalize(self):
    # self.OrderSysID = self.OrderSysID.strip()
    # self.TradeID = self.TradeID.strip()
    # self.OrderLocalID = self.OrderLocalID.strip()
    return None

  def assign(self,data):
    object_assign(self,data,True)
    return self

  @property
  def code(self): return self.InstrumentID

#撤單記錄
class OrderCancel(object):
  def __init__(self):
    self.InstrumentID=''
    self.ExchangeID=''
    self.ActionFlag=''
    self.OrderActionRef=''
    self.UserID=''
    self.LimitPrice=0
    self.OrderRef=''
    self.InvestorID=''
    self.SessionID=''
    self.VolumeChange=0
    self.BrokerID=''
    self.RequestID=''
    self.OrderSysID=''
    self.FrontID=''

  def assign(self,data):
    object_assign(self,data,True)
    return self

  @property
  def code(self): return self.InstrumentID

  @property
  def order_id(self):
    return self.OrderSysID

class OrderRecord(object):
  """ CThostFtdcOrderField """
  def __init__(self):
    self.InstrumentID = ''  # 合约代码
    self.OrderRef = 0  # 报单引用
    self.UserID = ''  # 用户代码
    self.OrderPriceType = ''  # 报单价格条件
    self.Direction = ''  # 买卖方向
    self.CombOffsetFlag = ''  # 组合开平标志
    self.CombHedgeFlag = ''  # 组合投机套保标志
    self.LimitPrice = 0  # 价格
    self.VolumeTotalOriginal = ''  # 数量
    self.TimeCondition = ''  # 有效期类型
    self.GTDDate = ''  # GTD日期
    self.VolumeCondition = ''  # 成交量类型
    self.MinVolume = 0  # 最小成交量
    self.ContingentCondition = ''  # 触发条件
    self.StopPrice = 0  # 止损价
    self.ForceCloseReason = ''  # 强平原因
    self.IsAutoSuspend = ''  # 自动挂起标志
    self.RequestID = 0  # 请求编号
    self.OrderLocalID = ''  # 本地报单编号
    self.ExchangeID = ''  # 交易所代码
    self.ClientID = ''  # 客户代码
    self.OrderSubmitStatus = ''  # 报单提交状态
    self.NotifySequence = ''  # 报单提示序号
    self.TradingDay = ''  # 交易日
    self.SettlementID = ''  # 结算编号
    self.OrderSysID = ''  # 报单编号
    self.OrderSource = ''  # 报单来源
    self.OrderStatus = ''  # 报单状态
    self.OrderType = ''  # 报单类型
    self.VolumeTraded = ''  # 今成交数量
    self.VolumeTotal = ''  # 剩余数量
    self.InsertDate = ''  # 报单日期
    self.InsertTime = ''  # 委托时间
    self.ActiveTime = ''  # 激活时间
    self.SuspendTime = ''  # 挂起时间
    self.UpdateTime = ''  # 最后修改时间
    self.CancelTime = ''  # 撤销时间
    self.SequenceNo = ''  # 序号
    self.FrontID = ''  # 前置编号
    self.SessionID = ''  # 会话编号
    self.UserProductInfo = ''  # 用户端产品信息
    self.StatusMsg = ''  # 状态信息
    self.UserForceClose = ''  # 用户强评标志
    self.BrokerOrderSeq = ''  # 经纪公司报单编号
    self.BranchID = ''  # 营业部编号

  @property
  def code(self):
    return self.InstrumentID

  @property
  def name(self):
    return self.InstrumentID

  @property
  def order_id(self):
    """获取订单委托编号"""
    ids = 'B#{}#{}'.format(self.ExchangeID, self.OrderSysID)
    ids = ids.replace(' ', '_')
    return ids

  @property
  def user_order_id(self):
    """获取用户委托编号"""
    ids = 'A#{}#{}#{}'.format(self.FrontID, self.SessionID, self.OrderRef)
    ids = ids.replace(' ','_')
    return  ids

  def normalize(self):
    self.OrderStatus = chr(self.OrderStatus)
    self.OrderSubmitStatus = chr(self.OrderSubmitStatus)
    # self.OrderSysID = self.OrderSysID.strip()

  def cancelable(self):
    if self.OrderStatus not in (Constants.OrderStatusType.AllTraded, Constants.OrderStatusType.Canceled):
      return True
    return False


  def assign(self,data):
    object_assign(self,data,True)
    return self

#
# class TradeError(object):
#   """CThostFtdcTradeField"""
#   def __init__(self):
#     self.id = ''
#     self.errcode = 0
#     self.errmsg = ''
#
#   def assign(self,data):
#     object_assign(self,data,True)
#     return self

class OrderError(object):
  """CThostFtdcInputOrderField"""
  def __init__(self):
    self.ErrorID = 0


class OrderCancelError(object):
  """CThostFtdcOrderActionField"""
  def __init__(self):
    self.ErrorID = 0


class OrderRecordSet(object):
  """委托记录集合"""
  def __init__(self):
    self.inner :List[OrderRecord] = []

  def assign(self,data:List):
    for d in data:
      obj = OrderRecord()
      object_assign(obj,d)
      self.inner.append(obj)
    return self

class PositionSet(object):
  """仓位记录集合"""
  def __init__(self):
    self.inner: List[Position] = []

  def assign(self,data:List):
    for d in data:
      obj = Position()
      object_assign(obj,d)
      self.inner.append(obj)
    return self

class TradeRecordSet(object):
  """成交记录集合"""
  def __init__(self):
    self.inner: List[TradeReturn] = []

  def assign(self,data:List):
    for d in data:
      obj = TradeReturn()
      object_assign(obj,d)
      self.inner.append(obj)
    return self


def normal_price(price):
  if price > 1.7976931348623157e+30:
    price = 0
  return price

def get_symbol_prefix(symbol):
  """合约类型 M,AU"""
  import re
  m = re.findall('^([A-Za-z]{1,3})\d{2,5}', symbol)
  if m:
    return m[0]
  return ''

def get_exchange_id(symbol):
  """根据合约名获得交易所 exchange_id """
  prefix = get_symbol_prefix(symbol)
  # print(prefix)
  for p,v in instrinfo.INSTRINFO.items():
    if prefix == p:
      # print(p)
      return v['exchange']
  return ''

def get_insrinfo(symbol):
  prefix = get_symbol_prefix(symbol)
  info = instrinfo.INSTRINFO.get(prefix)
  return info

def get_order_price_float_tick(symbol):
  """获得指定合约 基于最新成交价的委托报价浮动单位，"""
  return 5 # 偏离最新成交价 5个单位报价


def test():
  symbols = ['CF201','m202']
  products = list(map(get_exchange_id,symbols))
  print(products)

if __name__ == '__main__':
  # fire.Fire()
  test()
