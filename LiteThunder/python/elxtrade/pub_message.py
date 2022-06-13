#coding:utf-8

import fire
import json
import zmq
from elxtrade.fundamental.utils.useful import singleton, hash_object, object_assign



#连接Pub服务，发送本地日志信息到远端MX
@singleton
class MessagePublisher(object):
  def __init__(self):
    self.cfgs = {}
    self.sock = None

  def init(self,**kvs):
    self.cfgs.update(**kvs)
    if self.cfgs.get('pub_service.enable',1):
      self.ctx = zmq.Context()
      self.sock = self.ctx.socket(zmq.PUB)

      addr = self.cfgs.get('pub_service.addr')
      self.sock.connect(addr)
    return self

  def open(self):
    return self

  def close(self):
    return self

  # 消息推送到远端zmq接收服务
  def publish(self,topic,text):
    if not self.sock:
      return
    data = {
      "id": self.cfgs['id'],
      "service_type":self.cfgs['service_type'],
      "account_id": self.cfgs['account_id'],
      "topic":topic,
      "data": text
    }
    jsondata = json.dumps(data)
    self.sock.send_string( jsondata )
