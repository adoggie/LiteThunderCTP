#coding:utf-8

import fire

from elxtrade.fundamental.utils.useful import singleton, hash_object, object_assign
from elxtrade.mx_reader import MxReader
from elxtrade.pos_receiver import PosReceiver
from elxtrade.registry_client import RegistryClient
from elxtrade.logger import Logger
from elxtrade.pub_message import MessagePublisher
from elxtrade.broker import TradeApi,MarketApi

@singleton
class Controller(object):
  def __init__(self):
    self.cfgs = {}
    self.behaviors = {}
    self.pos_reciever = None
    self.registry_client = None
    self.risk_manager = None

  def getConfig(self):
    return self.cfgs

  def init(self,**kvs):
    Logger().init(**kvs).open()
    self.cfgs.update(**kvs)

    MessagePublisher().init(**kvs)

    MxReader().init(**kvs)
    if kvs.get("position_receiver.enable",1):
      self.pos_reciever = PosReceiver().init(**kvs)
    if kvs.get("registry_client.enable",1):
      self.registry_client = RegistryClient().init(**kvs)

    TradeApi().init(**kvs)
    MarketApi().init(**kvs)
    return self

  def open(self):
    # TradeApi().getOrder()

    MessagePublisher().open()

    for b in self.behaviors.values():
      b.ready(**self.cfgs)

    MxReader().open()

    if self.pos_reciever:
      self.pos_reciever.open()

    if self.registry_client:
      self.registry_client.open()

    for b in self.behaviors.values():
      b.open()

  def close(self):
    MxReader().close()

    if self.pos_reciever:
      self.pos_reciever.close()

    if self.registry_client:
      self.registry_client.close()

    for b in self.behaviors.values():
      b.close()
    pass

  def addBehavior(self,name,behavior):
    self.behaviors[name] = behavior

    MxReader().addUser(behavior)
    # if self.pos_reciever:
    #   self.pos_reciever.addUser(behavior)

    return self


def run_behavior():
  b = None
  Controller().init().addBehavior("test",b)
  Controller().open()