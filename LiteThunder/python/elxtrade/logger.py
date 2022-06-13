#coding:utf-8

import fire
import zmq
import os
import sys
import shutil
import logging
import traceback
from logging.handlers import TimedRotatingFileHandler


from elxtrade.pub_message import MessagePublisher
from elxtrade.constants import TopicLogJson
from elxtrade.fundamental.utils.useful import singleton

@singleton
class Logger(object):
  def __init__(self):
    self.cfgs = {}
    self.level = 'DEBUG'

  def init(self,**kvs):
    self.cfgs.update(**kvs)
    self.level = self.cfgs.get('logger.level',self.level).upper()
    initLogger(self.level,self.cfgs['logger.path'],self.cfgs['logger.filename'])
    return self

  def open(self):

    return self

  def close(self):
    return self

  def write(self,level,text):
    level_defs=['DEBUG','INFO','WARN','ERROR']
    n1 = level_defs.index(self.level)
    n2 = level_defs.index(level)
    if n2 >= n1:
      MessagePublisher().publish(TopicLogJson,text)

initialized = False

def initLogger(log_level="DEBUG", log_path=None, logfile_name=None, clear=False, backup_count=0):
    """ 初始化日志输出
    @param log_level 日志级别 DEBUG/INFO
    @param log_path 日志输出路径
    @param logfile_name 日志文件名
    @param clear 初始化的时候，是否清理之前的日志文件
    @param backup_count 保存按天分割的日志文件个数，默认0为永久保存所有日志文件
    """
    logger = logging.getLogger()
    logger.setLevel(log_level)
    fmt_str = "%(levelname)1.1s [%(asctime)s] %(message)s"
    fmt = logging.Formatter(fmt=fmt_str, datefmt=None)


    if logfile_name:
        if clear and os.path.isdir(log_path):
            shutil.rmtree(log_path)
        if not os.path.isdir(log_path):
            os.makedirs(log_path)
        logfile = os.path.join(log_path, logfile_name)
        handler = TimedRotatingFileHandler(logfile, "midnight", backupCount=backup_count)
        handler.setFormatter(fmt)
        logger.addHandler(handler)
        print("init logger ...", logfile)
    # else:

    # print("init logger ... Console Stream")
    # handler = logging.StreamHandler()
    # handler.setFormatter(fmt)
    # logger.addHandler(handler)


def info(*args, **kwargs):
    func_name, kwargs = _log_msg_header(*args, **kwargs)
    text = _log(func_name, *args, **kwargs)
    logging.info(text)
    Logger().write('INFO',text)

def warn(*args, **kwargs):
    msg_header, kwargs = _log_msg_header(*args, **kwargs)
    text = _log(msg_header, *args, **kwargs)
    logging.warning(text)
    Logger().write('WARN',text)


def debug(*args, **kwargs):
    msg_header, kwargs = _log_msg_header(*args, **kwargs)
    text = _log(msg_header, *args, **kwargs)
    logging.debug(text)
    Logger().write('DEBUG',text)


def error(*args, **kwargs):
    logging.error("*" * 60)
    msg_header, kwargs = _log_msg_header(*args, **kwargs)
    text = _log(msg_header, *args, **kwargs)
    logging.error(text)
    # logging.error("*" * 60)
    Logger().write('ERROR',text)


def exception(*args, **kwargs):
    logging.error("*" * 60)
    msg_header, kwargs = _log_msg_header(*args, **kwargs)
    logging.error(_log(msg_header, *args, **kwargs))
    logging.error(traceback.format_exc())
    logging.error("*" * 60)


def _log(msg_header, *args, **kwargs):
    _log_msg = msg_header
    for l in args:
        if type(l) == tuple:
            ps = str(l)
        else:
            try:
                ps = "%r" % l
            except:
                ps = str(l)
        if type(l) == str:
            _log_msg += ps[1:-1] + " "
        else:
            _log_msg += ps + " "
    if len(kwargs) > 0:
        _log_msg += str(kwargs)
    return _log_msg

def _log_msg_header(*args, **kwargs):
    """ 打印日志的message头
    @param kwargs["caller"] 调用的方法所属类对象
    * NOTE: logger.xxx(... caller=self) for instance method
            logger.xxx(... caller=cls) for @classmethod
    """
    cls_name = ""
    func_name = ""

    session_id = "-"
    try:
        _caller = kwargs.get("caller", None)
        if _caller:
            if not hasattr(_caller, "__name__"):
                _caller = _caller.__class__
            cls_name = _caller.__name__
            del kwargs["caller"]
    except:
        pass
    finally:
        msg_header = "[{session_id}] [{cls_name}] ".format(cls_name=cls_name,
                                                                       session_id=session_id)
        return msg_header, kwargs
