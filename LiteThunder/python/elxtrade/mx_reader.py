#coding:utf-8

import threading
import os,sys,time,datetime,traceback
from collections import OrderedDict
from dateutil.parser import  parse

import json
# from rqdata_wrapper import current_snapshot,convertTick
from elxtrade import logger

from elxtrade.fundamental.utils.useful import object_assign,hash_object,singleton

from elxtrade.constants import *
from elxtrade.ctp import Tick, OrderRecord,OrderError,\
        OrderCancel,OrderCancelError,\
        TradeReturn,Constants,TradeRecordSet,\
        PositionSet,OrderRecordSet,AccountFunds

import zmq

@singleton
class MxReader(object):
    """"""
    def __init__(self,server = None):
        self.cfgs = {}
        self.server = server

        self.users = []
        self.broker = None
        self.basket_ids = []
        self.chanBasket = None          # 行情接收
        self.chan_trade_event = None    #成交事件

        self.ctx = zmq.Context()
        self.sock = self.ctx.socket(zmq.SUB)
        self.running = False


    def init(self,**cfgs):
        self.cfgs.update(**cfgs)
        # self.sock(zmq.SUBSCRIBE, b'')  # 订阅所有

        topic = self.cfgs.get('mx_reader.sub_topic','')

        self.sock.setsockopt(zmq.SUBSCRIBE,topic.encode('utf-8'))  # 订阅所有
        addr = self.cfgs.get('mx_reader.sub_addr',"tcp://127.0.0.1:9012")
        self.sock.connect( addr )
        return self

    def addUser(self,user):
        self.users.append(user)
        return self


    def _recv_thread(self):
        self.running = True
        poller = zmq.Poller()
        poller.register(self.sock, zmq.POLLIN)
        while self.running:
            text = ''
            try:
                events = dict(poller.poll(1000))
                if self.sock in events:
                    text = self.sock.recv_string()
                    self.parse(text)
            except:
                traceback.print_exc()
                print(text)
                time.sleep(.1)

    def dispatch(self,data):
        for user in self.users:
            if isinstance(data, Tick):
                user.onTick(data)
            elif isinstance(data, TradeReturn):
                user.onTradeReturn(data)
            elif isinstance(data, OrderRecord):
                user.onOrderReturn(data)
            elif isinstance(data, OrderError):
                user.onOrderErrorReturn(data)
            elif isinstance(data, OrderCancel):
                user.onOrderCancelReturn(data)
            elif isinstance(data, OrderCancelError):
                user.onOrderCancelErrorReturn(data)

            elif isinstance(data,PositionSet):
                user.onPositionSetReturn(data)
            elif isinstance(data,TradeRecordSet):
                user.onTradeSetReturn(data)
            elif isinstance(data,OrderRecordSet):
                user.onOrderSetReturn(data)
            elif isinstance(data,AccountFunds):
                user.onAccountFunds(data)

    def dump_data(self,data,body):
        logfile = ''

        if isinstance(data,Tick):
            logfile = open('tick.txt','a+')

        if isinstance(data, TradeReturn):
            logfile = open('trade.txt','a+')
        elif isinstance(data, OrderRecord):
            logfile = open('order.txt','a+')
        elif isinstance(data, OrderError):
            logfile = open('order-error.txt','a+')
        elif isinstance(data, OrderCancel):
            logfile = open('order-cancel.txt','a+')
        elif isinstance(data, OrderCancelError):
            logfile = open('order-cancel-error.txt','a+')

        elif isinstance(data, PositionSet):
            logfile = open('pos-set.json','w')
        elif isinstance(data, TradeRecordSet):
            logfile = open('trade-set.json','w')
        elif isinstance(data, OrderRecordSet):
            logfile = open('order-set.json','w')
        elif isinstance(data, AccountFunds):
            logfile = open('accfuns.json','w')
        if logfile:
            logfile.write(body)
            logfile.write("\n\n")
            logfile.close()

    def parse(self,text):
        body = text[TopicHead_MaxLen:]
        obj = None

        # print( text[:TopicHead_MaxLen])
        if text.find(TopicTick) == 0:
            obj = Tick()
        if text.find(TopicTickText) == 0:
            obj = Tick()
            obj.assginText(body)
            self.dispatch(obj)
            return

        elif text.find(TopicAccountFunds) == 0:
            obj = AccountFunds()
        elif text.find(TopicTradeQueryResult) == 0 :
            data = json.loads(body)
            obj = TradeRecordSet()
            obj.assign(data)
        elif text.find(TopicOrderQueryResult) == 0 :
            data = json.loads(body)
            obj = OrderRecordSet()
            obj.assign(data)
        elif text.find(TopicPositionQueryResult) == 0:
            data = json.loads(body)
            obj = PositionSet()
            obj.assign(data)
        elif text.find(TopicTrade) == 0:
            obj = TradeReturn()
        elif text.find(TopicOrder) == 0:
            obj = OrderRecord()
        elif text.find(TopicOrderError) == 0:
            obj = OrderError()
        elif text.find(TopicOrderCancelError) == 0:
            obj = OrderCancelError()


        elif text.find(TopicLogText) == 0:
            # logger.debug(body)
            pass

        if obj :
            data = json.loads(body)
            if isinstance(obj, (TradeRecordSet,PositionSet,OrderRecordSet)):
                obj.assign(data)
            else:
                object_assign(obj,data)
            self.dispatch(obj)

        self.dump_data(obj,body)

    def open(self):
        self.thread = threading.Thread(target=self._recv_thread)
        self.thread.daemon = True
        self.thread.start()
        return self

    def close(self):
        self.running = False
        self.sock.close()
        self.thread.join()

