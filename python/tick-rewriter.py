import datetime
import os,os.path,time,traceback

import redis as  Redis
import fire
from config import data_root,REDIS

now = datetime.datetime.now()
redis = Redis.StrictRedis( **REDIS)
data_dir= os.path.join(data_root , str(now.date())[:7])
open_files = {}

def make_dir(*args):
  path = os.path.join(*args)
  if not os.path.exists(path):
    os.makedirs(path)

def open_file(filename,mode='w'):
  dirname = os.path.dirname( filename )
  make_dir(dirname)
  return open(filename,mode)


def run ():
    instruments = redis.hgetall('ctp_instruments').keys()
    for ins in instruments:
        # data = redis.hgetall(ins)
        # print(ins, redis.hlen(ins))
        # process_instrument_hash(ins)
        process_instrument_list(ins)
    time.sleep(2)
    # reset()

def reset():
    # instruments = redis.hgetall('ctp_instruments').keys()
    # for ins in instruments:
    #     redis.delete("l_"+ins.decode())
    # redis.delete('ctp_instruments')
    redis.flushall()


def process_instrument_list(ins):
    name = "l_"+ins.decode()
    num = redis.llen( name )

    print(">>",name,num,"..")

    fn = os.path.join(data_dir, f"{ins.decode()}.txt")
    fp = open_files.get(fn)
    if not fp:
        fp = open_file(fn, 'a+')
        open_files[fn] = fp
    while True:
        tick = redis.lpop(name)
        if not tick:
            break
        fp.write(tick.decode())
        fp.write('\n')

    for fp  in open_files.values():
        fp.close()

"""
ss << p->InstrumentID <<"," << p->ExchangeID << ","
    << timestamp << "," << now_ts << ","
    << p->ActionDay << "," << p->TradingDay <<","
    << p->UpdateTime << "," << p->UpdateMillisec << ","
    << p->BidPrice1 << FS << p->BidPrice2 << FS << p->BidPrice3 << FS << p->BidPrice4 << FS << p->BidPrice5 << ","
    << p->BidVolume1 << FS << p->BidVolume2 << FS << p->BidVolume3 << FS << p->BidVolume4 << FS << p->BidVolume5 << ","
    << p->AskPrice1 << FS << p->AskPrice2 << FS << p->AskPrice3 << FS << p->AskPrice4 << FS << p->AskPrice5 << ","
    << p->AskVolume1 << FS << p->AskVolume2 << FS << p->AskVolume3 << FS << p->AskVolume4 << FS << p->AskVolume5 << ","

    << p->AveragePrice <<  "," << p->LastPrice << "," << p->LowerLimitPrice << ","
    << p->UpperLimitPrice << "," << p->OpenInterest << "," << p->Turnover << "," << p->Volume;
    
"   20220215 10:57:48.0" "IF2209,,1644922668,1644893868,20220215,20220215,10:57:48,0,
    4510#1.79769e+308#1.79769e+308#1.79769e+308#1.79769e+308,
    5#0#0#0#0,
    4512.6#1.79769e+308#1.79769e+308#1.79769e+308#1.79769e+308,
    2#0#0#0#0,
    1.3497e+06,4511,4028.4,4923.6,13317,2.02185e+09,1498"
    
    now_ts 接收系统时间 
    timestamp  ActionData + UpdateTime + UpdateMillisec 
    real_ts = timestamp - secs_of_day  if  timestamp > (now_ts + 3600 *4)  
    timestamp 跨交易日 超过本地时间 4 小时， 则
"""

# 处理缓存在hash内的数据
def process_instrument_hash(ins):
    data = redis.hgetall(ins)
    keys = data.keys()

    class wrapped(object):
        def __init__(self,dt,key):
            self.dt = dt
            self.key = key

    sorted_keys = []
    for key in keys:
        key = key.decode()
        ymd,hmss = key.split(' ')
        hmss = hmss.split('.')
        hms = hmss[0]
        ms = 0
        if len(hmss) > 1:
            ms = int(hmss[1])
        h,m,s = hms.split(':')
        dt = datetime.datetime(year=int(ymd[:4]),month=int(ymd[4:6]),day = int(ymd[6:]) , hour=int(h), minute=int(m),second=int(s),microsecond=ms)
        sorted_keys.append( wrapped(dt,key))
    sorted_keys = sorted(sorted_keys,key=lambda x:x.dt)
    for w in sorted_keys:
        tick = data[w.key.encode()].decode()
        date = w.key.split(' ')[0]
        fn = os.path.join(data_dir,date,f"{ins.decode()}.csv")
        fp = open_files.get(fn)
        if not fp:
            fp = open_file(fn,'a+')
            open_files[fn] = fp
        # tick = tick.replace('#',',')
        fp.write(tick)
        fp.write('\n')
        # fs = tick.split(",")
        # InstrumentID, DateTime, Bids, BidVols, Asks, AskVols, AveragePrice, ActionDay, \
        # LastPrice, LowerLimitPrice, \
        # UpperLimitPrice, OpenInterest, Timestamp, Turnover, Volume = fs[:15]
        # Bids = list(map(float, Bids.split('#')))
        # Asks = list(map(float, Asks.split('#')))
        # BidVols = list(map(float, BidVols.split('#')))
        # AskVols = list(map(float, AskVols.split('#')))
        # AveragePrice = float(AveragePrice)
        # LastPrice = float(LastPrice)
        # LowerLimitPrice = float(LowerLimitPrice)
        # UpperLimitPrice = float(UpperLimitPrice)
        # OpenInterest = float(OpenInterest)
        # Timestamp = int(Timestamp)
        # Turnover = float(Turnover)
        # Volume = float(Volume)

    for fp  in open_files.values():
        fp.close()

if __name__ == '__main__':
    # reset()
    fire.Fire()
    # run()