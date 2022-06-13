
Monkey
========
C++版本的控制主机程序

## 主体功能

1. 
2. 
3. 

## 环境&开发

    boost1.69
    jsoncpp 
    nanomsg



## Test Scripts

*nanocat*

http://manpages.ubuntu.com/manpages/cosmic/man1/nanocat.1.html


```bash

wget http://127.0.0.1:7001/api/smartbox/sensor/status/generate?sensor_type=2&sensor_id=1&name=1&value=1


curl -X POST http://127.0.0.1:7001/api/smartbox/sensor/params -d 'sensor_type=1&sensor_id=2&name=light&value=on'
curl -X POST http://127.0.0.1:7001/api/smartbox/sensor/params -d 'sensor_type=1&sensor_id=2&name=1&value=1'
telnet 127.0.0.1 7002

```

numcpp
-----
https://www.jianshu.com/p/5a2834599a52
https://www.qbitai.com/2019/02/519.html
https://dpilger26.github.io/NumCpp/doxygen/html/index.html

quantlib
======
https://www.quantlib.org/
https://www.quantlib.org/install/linux.shtml
https://blog.csdn.net/weixin_30362233/article/details/95710041?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-1.edu_weight&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-1.edu_weight

https://www.cnblogs.com/xuruilong100/p/8711520.html

ta-lib
-----
https://mrjbq7.github.io/ta-lib/

ctp-api
------
https://zhuanlan.zhihu.com/p/103484086


gdb
------
vim ~/.gdbinit
handle SIGUSR1 noprint nostop