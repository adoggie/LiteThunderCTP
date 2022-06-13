//
// Created by eladmin on 2021/11/2.
//

#ifndef ELXTRADER_BEHAVIOR_H
#define ELXTRADER_BEHAVIOR_H

#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include "base.h"
#include "mx.h"
#include "config.h"

class TdApi;
class MdApi;

struct Behavior:public Object{
    // 行情分时
    virtual void onTick(trade::Tick_t* data){};
    // 成交回报
    virtual void onTradeReturn(trade::Trade_t *pTrade) {} ;
    // 委托回报
    virtual void onOrderReturn(trade::Order_t *order) {};
//    virtual void onOrderInsert(trade::InputOrder_t * input,trade::RespInfo_t *resp) {};
    // 委托失败回报
    virtual void onOrderErrorReturn(trade::InputOrder_t * input , trade::RespInfo_t *resp) {} ;
    //撤单回报
//    virtual void onOrderCancelReturn(trade::InputOrderAction_t *action, trade::RespInfo_t *resp) {};
    //撤单错误回报
    virtual void onOrderCancelErrorReturn(trade::OrderAction_t* action,trade::RespInfo_t* resp) {};

    virtual void init(const Config&  cfg);
    virtual bool open();
    virtual void close();

    TdApi *  getTradeApi() const;
    MdApi *  getMarketApi() const;
};


#endif //ELXTRADER_BEHAVIOR_H
