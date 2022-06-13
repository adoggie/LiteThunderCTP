import time
import traceback
from threading import Thread,Condition,Lock
import redis as Redis
import json 
import fire
import zmq
import random
from kvconfig import SimpleConfig

redis_server = dict(
    host = '127.0.0.1' ,port=6379,db = 0 
)
redis = Redis.StrictRedis( **redis_server)

pub_addr = "tcp://127.0.0.1:15551"
ctx = zmq.Context()
sock = ctx.socket(zmq.PUB)
sock.connect(pub_addr )
time.sleep(.5) 
    
"""
thunderid:{instrument:ps }
exAMP:
  thunder_scott: { AP208: 10 ,jm2208:-4}
"""

## 填充合约仓位
def fill_instruments(configfile='../LiteThunder/settings-scott.txt',
                     instrument_file='../LiteThunder/instruments.json'):
  fn = instrument_file
  configs = SimpleConfig().load(configfile)
  # print(configs.props)
  jdata = json.loads(open(fn).read())
  thunder_id = configs.props['id']
  
  for instr in jdata['instruments']:
      # print(thunder_id,instr['name'],str(instr['ps']))
      redis.hset(thunder_id,instr['name'],instr['ps'])


def generate_pos(commodity,instruments, ps ,thunder ,forever = 'false' ,sleep=5):
  print(instruments)
  if isinstance( instruments ,str):
    instruments = [ instruments]
  def once():
    pps = ps
    for instrument in instruments:
      if not instrument.strip():
          continue
      if forever =='true':
        pps = random.randint(-20,20)
      
      redis.hset(thunder,instrument,pps)
      text = f"{thunder},{commodity},{instrument},{pps}"
      sock.send(text.encode())
      print(text)
    
  if forever == 'true':
    while True:
      once()
      time.sleep(int(sleep))
  else:
    once()
    time.sleep(0.2)
  
  
"""
python thunder_test.py generate_pos --commodity='AP' --instrument='AP210' --ps=10 --thunder=thunder_scott
python thunder_test.py generate_pos --commodity='AP' --instrument='AP210' --ps=10 --thunder=thunder_scott --forever=true
"""    

if __name__ == '__main__':
    fire.Fire() 