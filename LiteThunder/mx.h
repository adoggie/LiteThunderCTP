//
// Created by scott on 2021/11/4.
//

#ifndef ELXTRADER_MX_H
#define ELXTRADER_MX_H
#include <ThostFtdcUserApiStruct.h>
#include <string>
#include <jsoncpp/json/json.h>

#define EVENT_TICK "tick"
#define EVENT_ORDER "order"
#define EVENT_TRADE "trade"
#define EVENT_ERROR "error"
#define EVENT_LOG "log"

#define TopicTick               "tick/json"
#define TopicTickRaw               "tick/raw"
#define TopicTickText               "tick/text"
#define TopicOrder              "order"
#define TopicOrderError         "order_error"
//#define TopicOrderInsert         "order_insert"
//#define TopicOrderCancel        "order_cancel"
#define TopicOrderCancelError   "order_cancel_error"
#define TopicTrade              "trade"
#define TopicPosition           "position"   // 仓位变动
#define TopicLogText                "log/text"   // 仓位变动
#define TopicLogJson                "log/json"   // 仓位变动
#define TopicTradeQueryResult                "trade/qry/result"   // 成交记录集合
#define TopicPositionQueryResult        "position/qry/result"     //持仓查询
#define TopicOrderQueryResult           "order/qry/result"        // 委托记录查询
#define TopicAccountFunds           "account/funds"                // 账户资金查询

#define TOPIC_MAX_LEN 30
#define TOPIC_SPLIT_TOKEN "||"

enum class  TickEncoding{
    Json,
    Text,
    Raw
};

namespace trade {

    typedef CThostFtdcDepthMarketDataField Tick_t;
    typedef CThostFtdcTradeField Trade_t;
    typedef CThostFtdcOrderField Order_t;
    typedef CThostFtdcInputOrderField InputOrder_t;
    typedef CThostFtdcRspInfoField RespInfo_t;

    typedef CThostFtdcInputOrderActionField  InputOrderAction_t; //撤单错误回报
    typedef CThostFtdcOrderActionField  OrderAction_t;  // 撤单回报


    std::string marshall( const CThostFtdcInputOrderField* input, const CThostFtdcRspInfoField* rsp);
    std::string marshall( const CThostFtdcTradeField* trade,Json::Value* pdata = NULL);
    std::string marshall( const CThostFtdcDepthMarketDataField* data);
    std::vector<char> marshall( const CThostFtdcDepthMarketDataField* data ,TickEncoding type ,std::string& key);
    std::string marshall( const CThostFtdcOrderField* order,Json::Value* pdata = NULL);

    std::string marshall(const std::vector< CThostFtdcTradeField >& trades);
    std::string marshall(const std::vector<  CThostFtdcInvestorPositionField > & positions);
    std::string marshall(const CThostFtdcTradingAccountField* account);
    std::string marshall(const std::vector<CThostFtdcOrderField>& orders);

    std::string marshall(const CThostFtdcInputOrderActionField *action, const CThostFtdcRspInfoField *resp);
    std::string marshall( const CThostFtdcOrderActionField *action, const  CThostFtdcRspInfoField *resp);
};


#endif //ELXTRADER_MX_H
