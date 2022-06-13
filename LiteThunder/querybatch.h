#ifndef CAFB9847_6857_4F73_B5CF_239A7751BDA1
#define CAFB9847_6857_4F73_B5CF_239A7751BDA1

#include "trade.h"
#include <cstdint>
#include <ctime>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <thread>
#include <iostream>
#include <mutex>
#include <atomic>



struct QueryBatchTimed{
    enum QueryTask{
        QUERY_POSITION,
        QUERY_ACCOUNT,
        QUERY_ORDER,
        QUERY_TRADE
    };
    bool                  started_ = false;
    std::time_t           latest_ = 0; // 最近一次发起的请求 ，查询返回清除 为 0 
    int                   timeout_ = 3; 
    std::list< QueryTask> query_list_;

    void start();
    void timeTick();
    void resetTime();

};


#endif /* CAFB9847_6857_4F73_B5CF_239A7751BDA1 */
