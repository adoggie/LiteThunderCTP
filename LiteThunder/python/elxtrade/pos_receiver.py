#coding:utf-8
import datetime
import threading
import traceback

import fire
import zmq
import os
import json
from elxtrade.fundamental.utils.useful import singleton,open_file

PWD = os.path.dirname(os.path.abspath(__file__))
FN = os.path.join(PWD, './examples/settings.json')

@singleton
class PosReceiver(object):
  def __init__(self):
    self.cfgs = {}
    self.running = False
    self.ctx = zmq.Context()
    self.topic = ''
    self.users = []
    self.thread = None

  def init(self,**kvs):
    self.cfgs.update(**kvs)
    return self

  def addUser(self,user):
    self.users.append(user)
    return self

  def open(self):
    self.sock = self.ctx.socket(zmq.SUB)
    topic = '#'+self.cfgs['account_id']+'#'
    self.topic = topic.upper()
    # self.topic = b''
    self.sock.setsockopt(zmq.SUBSCRIBE, self.topic.encode())  # 订阅所有品种
    addr = self.cfgs["position_receiver.pub_addr"]

    self.sock.connect(addr)
    self.thread = threading.Thread(target = self.recv_thread)
    self.thread.daemon = True
    self.thread.start()
    return self

  def recv_thread(self):
    self.running = True
    poller = zmq.Poller()
    poller.register(self.sock, zmq.POLLIN)

    while self.running:
      events = dict(poller.poll(1000))
      try:
        if self.sock in events:
          text = self.sock.recv_string()  #  '#91001727#a_-4'
          name,pos = text.split(self.topic)[-1].split('_')
          self.log_pos(name,pos)
          for user in self.users:
            user.onPositionSignal(name, int(pos) )  # 回调给关注者仓位改变
      except:
        traceback.print_exc()

  def log_pos(self,name,pos):
    if not self.cfgs.get("position_receiver.log",0):
      return
    fn = os.path.join(self.cfgs.get("position_receiver.log.path","pos"),"ps_{}.txt".format(name))
    fp = open_file(fn,'a')
    now = datetime.datetime.now()
    fp.write( "{} {} {} \n".format(str(now),name, pos) )
    fp.close()

  def close(self):
    self.running = False
    self.sock.close()

    return self

def test():
  kvs = json.loads(open(FN).read())
  PosReceiver().init(**kvs).open()
  PosReceiver().thread.join()

if __name__ == '__main__':
  fire.Fire()