
### ELXTRADER   
期货交易

#### 1. c++ 交易客户端版本

#### 2. python 交易客户端版本

```
elxtrade/examples/twap_step1.py
简单交易客户程序，支持 命令输入 委托操作
```

### 配置

```
examples/settings.json

 
```

### 运行  

```
python -m elxtrade.examples.twap_step run <noprompt>     
   -- noprompt  不进入命令行交互

```

### 操作 

```
show pos      ---  show current position list.   当前持仓
show tick  m  ---  show tick list.    当前合约价格
show order q  ---  show queuing order list.  未完成委托
order send [ code price  quantity direction oc ] --- send order .   发单
      - price  0 or +/- shift    价格偏离tick， 0 表示最新成交价 ， 1 表示 最新成交价上浮1个ticksize
      - quantity  int     数量
      - direction  buy or sell   多空方向
      - oc   close or open    开平
order cancel xxx / all     撤单， all 表示撤除所有未成交订单
pos emit RM201 5    --- 手动触发仓位信号
exit    退出交互命令

... order

```

