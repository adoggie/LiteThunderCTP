#coding:utf-8

'''
//取得表 [Future].[dbo].[symbol_instrumentid]，得到各个品种对应的新老合约的比例，这个每分钟需要取一次
void get_pz_hy()

pip install pymssql pymysql
https://pypi.org/project/pymssql/

WK190422
'''

import fire
import json

import os
import pymysql
import pymssql
import pymysql.cursors

from elxtrade.fundamental.utils.useful import singleton

PWD = os.path.dirname(os.path.abspath(__file__))
FN = os.path.join(PWD, './examples/settings.json')

@singleton
class RegistryClient(object):
  def __init__(self):
    self.cfgs = {}
    self.ins_pos = {}  # 合约仓位
    self.ins = {}     # 合约信息
    self.add_price = {}      # 合约增加的交易价格
    self.delay_time = 0     # 延迟委托时间
    self.allow_ins = []
    self.forbid_ins = []
    self.account_id = ''
    self.acc = None  # 账号信息
    self.entry_db_info = None

  def init(self,**kvs):
    self.cfgs.update(**kvs)
    return self

  def open(self):
    self.account_id = self.cfgs['account_id']
    self.entry_main = self.get_conn_info(self.cfgs['registry_client.db.entry.main'])
    self.entry_heartbeat = self.get_conn_info(self.cfgs['registry_client.db.entry.heartbeat'])
    self.entry_zmqrouter = self.get_conn_info(self.cfgs['registry_client.db.entry.zmq_router'])

    self.acc = self.get_account_info()
    # brokername,brokerid,ac,pw,mdadd,tdadd,when_only_close,appid,authcode,hytable

    self.get_init_pos(self.account_id)
    self.get_instrument_info(self.account_id)
    print(RegistryClient())

  def get_ins_by_product(self,product):
    ins = None
    for _ in self.ins.values():
      if _['pz'] ==  product:
        ins = _
        break
    return ins

  def get_conn_info(self,entry):
    conn = self.get_entry_db_conn()
    sql = "select * from dbconn where info='%s' "%entry
    cur = conn.cursor()
    cur.execute(sql)
    r = cur.fetchone()
    """ defaultdb , host, port ,username ,passwd"""
    return r

  def get_init_pos(self,account_id):
    # 读取db中的初始仓位
    conn = self.get_db_conn(self.entry_main)
    sql = "SELECT  *  from get_zmq_initial_position('%s')"%account_id
    cur = conn.cursor(as_dict = True)
    cur.execute(sql)
    rs = cur.fetchall()
    for r in rs:
      # dbg_symbols = self.cfgs.get("registry_client.debug.symbols",[])
      # if dbg_symbols:
      #   # print("Debug Symbols:" , str(dbg_symbols))
      #   for s in dbg_symbols:
      #     if r['symbol'] == s:
      #       self.ins_pos[r['symbol']] = r['position']
      # else:
        self.ins_pos[r['symbol']] = r['position']

  def get_instrument_info(self,account_id):
    # 取合约信息
    # void get_symbol_instrumentid()
    # [Future].[dbo].[symbol_instrumentid] 得到表里的所有合约的相关属性，只需要在开始时初始化一次

    conn = self.get_db_conn(self.entry_main)
    sql = "select * from %s "%self.acc['hytable']
    cur = conn.cursor(as_dict = True)
    cur.execute(sql)
    rs = cur.fetchall()
    for r in rs :
      r['stockid'] = int(r['stockid'])
      r['tick'] = float( r['tick'] ) # 最小变动价
      r['percentage'] = float( r['percentage'] ) #
      r['target'] = float( r['target'] ) #
      r['tradetype'] = 'closey_closet' #

      dbg_symbols = self.cfgs.get("registry_client.debug.symbols", [])
      if dbg_symbols:
        for s in dbg_symbols:
          if r['instrumentid'] == s:
            self.ins[r['instrumentid']] = r
      else:
        self.ins[ r['instrumentid'] ] = r

      g_when_only_close = self.acc['when_only_close']
    end = 'H'
    """
    夜盘20:00- 03:00 时间 closey_closet
    日盘 08:00 - 20:00   closey_closet
    
    int time=st.wHour*10000 + st.wMinute*100 + st.wSecond;
							if(g_when_only_close>=200000)
							{
								if(time>=g_when_only_close || time<=30000) hy->tradetype="closey_closet";
							}
							else if(g_when_only_close<200000 && g_when_only_close>=80000)
							{
								hy->tradetype="closey_closet";
							}

    """

  def get_entry_db_conn(self):
    conn = pymysql.connect(host=self.cfgs['registry_client.db.host'],
                           port=self.cfgs['registry_client.db.port'],
                           db=self.cfgs['registry_client.db.name'],
                           user=self.cfgs['registry_client.db.user'],
                           passwd=self.cfgs['registry_client.db.passwd'],
                           cursorclass=pymysql.cursors.DictCursor
                           )
    return conn

  def get_db_conn(self,entry):
    # mssqlserver
    defaultdb = entry['defaultdb']
    host = entry['host']
    port = entry['port']
    username = entry['username']
    passwd = entry['passwd']

    conn = pymssql.connect(host,username,passwd,defaultdb)
    # cur = conn.cursor(as_dict=True)
    return conn

  def get_account_info(self):
    conn = self.get_db_conn(self.entry_main)

    cur = conn.cursor(as_dict = True )

    sql = "select brokername,brokerid,ac,pw,mdadd,tdadd,when_only_close,appid,authcode,hytable " \
        "from Future.dbo.ACstat where ac='%s' UNION all select brokername,brokerid,ac,pw,mdadd," \
        "tdadd,when_only_close,appid,authcode,hytable from Logrecord.dbo.temp_Acstat where ac='%s'"%( self.account_id, self.account_id)
    cur.execute(sql)
    acc = cur.fetchone()
    return acc


    sql = "select * from dbconn where info='ac_db_txy' "
    cur.execute(sql)
    r = cur.fetchone()
    # host = r['host']
    # username = r['username']
    # passwd = r['passwd']
    # sqltype = r['sqltype']
    # defaultdb = r['defaultdb']
    return r

  def get_trade_contract(self):
    # "contract.txt"
    # 交易合约
    return ''

  def keepalive(self):
    # UpdateTimeToDatabase()
    # sprintf(a, "insert into CTP_heartbeat(name,time) values('%s',%d);", pname.c_str(), nowtime);
    # 数据库长连接，定时插入心跳, 程序名称和当前时间
    pass

  def close(self):
    return self

def test ():
  kvs = json.loads(open(FN).read())
  RegistryClient().init( **kvs).open()

if __name__ == '__main__':
  fire.Fire()

"""
取得表 [Future].[dbo].[symbol_instrumentid]，得到各个品种对应的新老合约的比例，这个每分钟需要取一次
void get_pz_hy()
sprintf(a,"SELECT * FROM %s order by id",g_heyue_table.c_str());

合约月份切换时的比重

s= (char *)((_bstr_t)pRecord->GetCollect("pz"));
pzname=s;
s= (char *)((_bstr_t)pRecord->GetCollect("instrumentid"));
hyname=s;

s= (char *)((_bstr_t)pRecord->GetCollect("percentage"));
per=stod(s);

s= (char *)((_bstr_t)pRecord->GetCollect("target"));
tar=stod(s);
"""