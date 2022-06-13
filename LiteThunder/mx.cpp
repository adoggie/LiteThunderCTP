//
// Created by scott on 2021/11/4.
//
#include "mx.h"
#include <jsoncpp/json/json.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>

namespace trade {
    // 委托失败回报
    std::string marshall(const CThostFtdcInputOrderField *input, const CThostFtdcRspInfoField *rsp) {
        Json::Value data;
        data["ContingentCondition"] = input->ContingentCondition;
        data["CombOffsetFlag"] = input->CombOffsetFlag;
        data["UserID"] = input->UserID;
        data["LimitPrice"] = input->LimitPrice;
        data["UserForceClose"] = input->UserForceClose;
        data["Direction"] = input->Direction;
        data["IsSwapOrder"] = input->IsSwapOrder;
        data["VolumeTotalOriginal"] = input->VolumeTotalOriginal;
        data["OrderPriceType"] = input->OrderPriceType;
        data["TimeCondition"] = input->TimeCondition;
        data["IsAutoSuspend"] = input->IsAutoSuspend;
        data["StopPrice"] = input->StopPrice;
        data["InstrumentID"] = input->InstrumentID;
        data["ExchangeID"] = input->ExchangeID;
        data["MinVolume"] = input->MinVolume;
        data["ForceCloseReason"] = input->ForceCloseReason;
        data["BrokerID"] = input->BrokerID;
        data["CombHedgeFlag"] = input->CombHedgeFlag;
        data["GTDDate"] = input->GTDDate;
        data["BusinessUnit"] = input->BusinessUnit;
        data["OrderRef"] = input->OrderRef;
        data["InvestorID"] = input->InvestorID;
        data["VolumeCondition"] = input->VolumeCondition;
        data["RequestID"] = input->RequestID;

        data["ErrorID"] = rsp->ErrorID;
        //data["ErrorMsg"] = rsp->ErrorMsg;
        std::string json_text = data.toStyledString();
        return json_text;
    }

    // 成交回报
    std::string marshall(const CThostFtdcTradeField *trade, Json::Value *pdata) {
        Json::Value data;
        data["InstrumentID"] = trade->InstrumentID;
        data["OrderRef"] = trade->OrderRef;
        data["UserID"] = trade->UserID;
        data["ExchangeID"] = trade->ExchangeID;
        data["TradeID"] = trade->TradeID;
        data["Direction"] = std::string(1, trade->Direction);
        data["OrderSysID"] = trade->OrderSysID;
        data["ParticipantID"] = trade->ParticipantID;
        data["ClientID"] = trade->ClientID;
        data["TradingRole"] = trade->TradingRole;
        data["ExchangeInstID"] = trade->ExchangeInstID;
        data["OffsetFlag"] = trade->OffsetFlag;
        data["HedgeFlag"] = std::string(1, trade->HedgeFlag);
        data["Price"] = trade->Price;
        data["Volume"] = trade->Volume;
        data["TradeDate"] = trade->TradeDate;
        data["TradeTime"] = trade->TradeTime;
        data["TradeType"] = trade->TradeType;
        data["PriceSource"] = trade->PriceSource;
        data["TraderID"] = trade->TraderID;
        data["OrderLocalID"] = trade->OrderLocalID;
        data["ClearingPartID"] = trade->ClearingPartID;
        data["BusinessUnit"] = trade->BusinessUnit;
        data["SequenceNo"] = trade->SequenceNo;
        data["TradingDay"] = trade->TradingDay;
        data["SettlementID"] = trade->SettlementID;
        data["BrokerOrderSeq"] = trade->BrokerOrderSeq;
        data["TradeSource"] = trade->TradeSource;

        std::string json_text;
        if (pdata) {
            *pdata = data;
        } else {
            json_text = data.toStyledString();
        }
        return json_text;
    }

    // tick
    std::vector<char> marshall(const CThostFtdcDepthMarketDataField *p, TickEncoding type,std::string& key ){
        std:std::stringstream  ss;
        std::vector<char>  bytes;
        if( TickEncoding::Text == type){
            /*
            InstrumentID,DateTime,Bids,BidVols,Asks,AskVols,AveragePrice,ActionDay,LastPrice,LowerLimitPrice,
            UpperLimitPrice,OpenInterest,Timestamp,Turnover,Volume
            */
            std::string datetime = p->ActionDay + std::string(" ") + p->UpdateTime + \
			std::string(".") + boost::lexical_cast<std::string>(p->UpdateMillisec);
            key = datetime ;
            datetime = datetime.substr(0,4) +"-"+ datetime.substr(4,2) + "-" + datetime.substr(6,2) + datetime.substr(8);

            boost::posix_time::ptime pt(boost::posix_time::time_from_string(datetime));
            std::time_t ts = boost::posix_time::to_time_t(pt);
            std::uint64_t timestamp = Json::Value::UInt64 (ts);
            std::uint64_t now_ts = Json::Value::UInt64(std::time(NULL));

            static char FS='#';
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
            std::string s = ss.str();
            bytes.assign(s.begin(),s.end());
        }
        return bytes;
    }

    std::string marshall( const CThostFtdcDepthMarketDataField* pDepthMarketData){
        
        Json::Value data;
        data["Bids"] = Json::Value(Json::arrayValue);
        data["Bids"].append(pDepthMarketData->BidPrice1);
        data["Bids"].append(pDepthMarketData->BidPrice2);
        data["Bids"].append(pDepthMarketData->BidPrice3);
        data["Bids"].append(pDepthMarketData->BidPrice4);
        data["Bids"].append(pDepthMarketData->BidPrice5);

        data["HighestPrice"] = pDepthMarketData->HighestPrice;
        data["LowerLimitPrice"] = pDepthMarketData->LowerLimitPrice;
        data["OpenPrice"] = pDepthMarketData->OpenPrice;

        data["Asks"] = Json::Value(Json::arrayValue);
        data["Asks"].append(pDepthMarketData->AskPrice1);
        data["Asks"].append(pDepthMarketData->AskPrice2);
        data["Asks"].append(pDepthMarketData->AskPrice3);
        data["Asks"].append(pDepthMarketData->AskPrice4);
        data["Asks"].append(pDepthMarketData->AskPrice5);

        data["PreClosePrice"] = pDepthMarketData->PreClosePrice;
        data["PreSettlementPrice"] = pDepthMarketData->PreSettlementPrice;
        data["UpdateTime"] = pDepthMarketData->UpdateTime;
        data["UpdateMillisec"] = pDepthMarketData->UpdateMillisec;
        data["AveragePrice"] = pDepthMarketData->AveragePrice;
        data["BidVols"] = Json::Value(Json::arrayValue);
        data["BidVols"].append(pDepthMarketData->BidVolume1);
        data["BidVols"].append(pDepthMarketData->BidVolume2);
        data["BidVols"].append(pDepthMarketData->BidVolume3);
        data["BidVols"].append(pDepthMarketData->BidVolume4);
        data["BidVols"].append(pDepthMarketData->BidVolume5);

        data["PreOpenInterest"] = pDepthMarketData->PreOpenInterest;
        data["Volume"] = pDepthMarketData->Volume;

        data["AskVols"] = Json::Value(Json::arrayValue);
        data["AskVols"].append(pDepthMarketData->AskVolume1);
        data["AskVols"].append(pDepthMarketData->AskVolume2);
        data["AskVols"].append(pDepthMarketData->AskVolume3);
        data["AskVols"].append(pDepthMarketData->AskVolume4);
        data["AskVols"].append(pDepthMarketData->AskVolume5);

        data["UpperLimitPrice"] = pDepthMarketData->UpperLimitPrice;

        data["InstrumentID"] = pDepthMarketData->InstrumentID;
        data["ClosePrice"] = pDepthMarketData->ClosePrice;
        data["ExchangeID"] = pDepthMarketData->ExchangeID;
        data["TradingDay"] = pDepthMarketData->TradingDay;
        data["PreDelta"] = pDepthMarketData->PreDelta;
        data["OpenInterest"] = pDepthMarketData->OpenInterest;
        data["CurrDelta"] = pDepthMarketData->CurrDelta;
        data["Turnover"] = pDepthMarketData->Turnover;
        data["LastPrice"] = pDepthMarketData->LastPrice;
        data["SettlementPrice"] = pDepthMarketData->SettlementPrice;
        data["ExchangeInstID"] = pDepthMarketData->ExchangeInstID;
        data["LowestPrice"] = pDepthMarketData->LowestPrice;
        data["ActionDay"] = pDepthMarketData->ActionDay;
        std::string datetime = pDepthMarketData->TradingDay + std::string(" ") + pDepthMarketData->UpdateTime + \
			std::string(".") + boost::lexical_cast<std::string>(pDepthMarketData->UpdateMillisec);
        data["DateTime"] = datetime;
        datetime = datetime.substr(0,4) +"-"+ datetime.substr(4,2) + "-" + datetime.substr(6,2) + datetime.substr(8);

        boost::posix_time::ptime pt(boost::posix_time::time_from_string(datetime));
//	boost::posix_time::ptime pt;
//    std::stringstream ss(datetime);
//    std::string format = "%Y%m%d %H:%M:%S.%f";
//    ss.imbue(std::locale(ss.getloc(), new boost::posix_time::time_input_facet(format)));//out
//    ss >> pt;

//    std::stringstream ss;
//    std::cout << task_data.InstrumentID << "," << data["DateTime"] << std::endl;
//    ss << task_data.InstrumentID << "," << data["DateTime"] ;
//    Application::instance()->getLogger().debug(ss.str());

        std::time_t ts = boost::posix_time::to_time_t(pt);
        data["Timestamp"] = Json::Value::UInt64 (ts);
        //(unsigned long)ts;

        std::string json_text = data.toStyledString();
        return json_text;
    }

    //委托回报
    std::string marshall( const CThostFtdcOrderField* order, Json::Value* pdata){
        Json::Value data;
        data["InstrumentID"] = order->InstrumentID;
        data["OrderRef"] = order->OrderRef;
        data["UserID"] = order->UserID;
        data["OrderPriceType"] = order->OrderPriceType;
        data["Direction"] = std::string(1, order->Direction);
        data["CombOffsetFlag"] = order->CombOffsetFlag;
        data["CombHedgeFlag"] = order->CombHedgeFlag;
        data["LimitPrice"] = order->LimitPrice;
        data["VolumeTotalOriginal"] = order->VolumeTotalOriginal;
        data["TimeCondition"] = std::string(1, order->TimeCondition);
        data["GTDDate"] = order->GTDDate;
        data["VolumeCondition"] = std::string(1, order->VolumeCondition);
        data["MinVolume"] = order->MinVolume;
        data["ContingentCondition"] = order->ContingentCondition;
        data["StopPrice"] = order->StopPrice;
        data["ForceCloseReason"] = std::string(1, order->ForceCloseReason);
        data["IsAutoSuspend"] = order->IsAutoSuspend;
        data["RequestID"] = order->RequestID;
        data["OrderLocalID"] = order->OrderLocalID;
        data["ExchangeID"] = order->ExchangeID;
        data["ClientID"] = order->ClientID;
        data["OrderSubmitStatus"] = order->OrderSubmitStatus;
        data["NotifySequence"] = order->NotifySequence;
        data["TradingDay"] = order->TradingDay;
        data["SettlementID"] = order->SettlementID;
        data["OrderSysID"] = order->OrderSysID;
        data["OrderSource"] = order->OrderSource;
        data["OrderStatus"] = order->OrderStatus;
        data["OrderType"] = order->OrderType;
        data["VolumeTraded"] = order->VolumeTraded;
        data["VolumeTotal"] = order->VolumeTotal;
        data["InsertDate"] = order->InsertDate;
        data["InsertTime"] = order->InsertTime;
        data["ActiveTime"] = order->ActiveTime;
        data["SuspendTime"] = order->SuspendTime;
        data["UpdateTime"] = order->UpdateTime;
        data["CancelTime"] = order->CancelTime;
        data["SequenceNo"] = order->SequenceNo;
        data["FrontID"] = order->FrontID;
        data["SessionID"] = order->SessionID;
        data["UserProductInfo"] = order->UserProductInfo;
//    data["StatusMsg"] = order->StatusMsg;
        data["UserForceClose"] = order->UserForceClose;
        data["BrokerOrderSeq"] = order->BrokerOrderSeq;
        data["BranchID"] = order->BranchID;

        std::string json_text ;
        if(pdata){
            *pdata = data;
        }else {
            json_text = data.toStyledString();
        }
        return json_text;
    }

    //撤单错误回报
    std::string marshall( const  CThostFtdcInputOrderActionField *action,const CThostFtdcRspInfoField *resp){
        Json::Value data;
        std::string json_text = data.toStyledString();

        data["BrokerID"] = action->BrokerID;
        ///投资者代码
        data["InvestorID"] = action->InvestorID ;
        ///报单操作引用
        data["OrderActionRef"] = action->OrderActionRef ;
        ///报单引用
        data["OrderRef"] = action->OrderRef ;
        ///请求编号
        data["RequestID"] = action->RequestID ;
        ///前置编号
        data["FrontID"] = action->FrontID ;
        ///会话编号
        data["SessionID"] = action->SessionID ;
        ///交易所代码
        data["ExchangeID"] = action->ExchangeID ;
        ///报单编号
        data["OrderSysID"] = action->OrderSysID ;
        ///操作标志
        data["ActionFlag"] = action->ActionFlag ;
        ///价格
        data["LimitPrice"] = action->LimitPrice ;
        ///数量变化
        data["VolumeChange"] = action->VolumeChange ;
        ///用户代码
        data["UserID"] = action->UserID ;
        ///合约代码
        data["InstrumentID"] = action->InstrumentID ;
        data["ErrorID"] = resp->ErrorID;
        //data["ErrorMsg"] = resp->ErrorMsg;
        return json_text;
    }

    //撤单回报
    std::string marshall( const CThostFtdcOrderActionField *action,const  CThostFtdcRspInfoField *resp){
        Json::Value data;

        ///经纪公司代码
        data["BrokerID"] = action->BrokerID ;
        ///投资者代码
        data["InvestorID"] = action->InvestorID ;
        ///报单操作引用
        data["OrderActionRef"] = action->OrderActionRef ;
        ///报单引用
        data["OrderRef"] = action->OrderRef ;
        ///请求编号
        data["RequestID"] = action->RequestID ;
        ///前置编号
        data["FrontID"] = action->FrontID ;
        ///会话编号
        data["SessionID"] = action->SessionID ;
        ///交易所代码
        data["ExchangeID"] = action->ExchangeID ;
        ///报单编号
        data["OrderSysID"] = action->OrderSysID ;
        ///操作标志
        data["ActionFlag"] = action->ActionFlag ;
        ///价格
        data["LimitPrice"] = action->LimitPrice ;
        ///数量变化
        data["VolumeChange"] = action->VolumeChange ;
        ///操作日期
        data["ActionDate"] = action->ActionDate ;
        ///操作时间
        data["ActionTime"] = action->ActionTime ;
        ///交易所交易员代码
        data["TraderID"] = action->TraderID;
        ///本地报单编号
        data["OrderLocalID"] = action->OrderLocalID ;
        ///操作本地编号
        data["ActionLocalID"] = action->ActionLocalID ;
        ///报单操作状态
        data["OrderActionStatus"] = action->OrderActionStatus ;
        ///用户代码
        data["UserID"] = action->UserID ;
        ///状态信息
        //data["StatusMsg"] = action->StatusMsg ;
        ///合约代码
        data["InstrumentID"] = action->InstrumentID ;

        data["ErrorID"] = resp->ErrorID;
        //data["ErrorMsg"] = resp->ErrorMsg;
        std::string json_text = data.toStyledString();
        return json_text;
    }

    std::string marshall(const std::vector< CThostFtdcTradeField >& trades){
        Json::Value root(Json::arrayValue);
        for(auto & trade : trades){
            Json::Value node;
            marshall(&trade,&node);
            root.append( node);
        }
        std::string json_text = root.toStyledString();
        return json_text;
    }

    std::string marshall(const std::vector<  CThostFtdcInvestorPositionField > & positions){
        Json::Value value(Json::arrayValue);
        for(auto & pos : positions){ //CThostFtdcInvestorPositionField
//        auto & pos = itr.second;
            Json::Value node;
            node["InstrumentID"] = pos.InstrumentID;
            node["PosiDirection"] = std::string(1,pos.PosiDirection);
            node["HedgeFlag"] = std::string(1,pos.HedgeFlag);
            node["PositionDate"] = std::string(1,pos.PositionDate);
            node["YdPosition"] = pos.YdPosition;
            node["Position"] = pos.Position;
            node["LongFrozen"] = pos.LongFrozen;
            node["ShortFrozen"] = pos.ShortFrozen;
            node["LongFrozenAmount"] = pos.LongFrozenAmount;
            node["ShortFrozenAmount"] = pos.ShortFrozenAmount;
            node["OpenVolume"] = pos.OpenVolume;
            node["CloseVolume"] = pos.CloseVolume;
            node["OpenAmount"] = pos.OpenAmount;
            node["CloseAmount"] = pos.CloseAmount;
            node["PositionCost"] = pos.PositionCost;
            node["PreMargin"] = pos.PreMargin;
            node["UseMargin"] = pos.UseMargin;
            node["FrozenMargin"] = pos.FrozenMargin;
            node["FrozenCash"] = pos.FrozenCash;
            node["FrozenCommission"] = pos.FrozenCommission;
            node["CashIn"] = pos.CashIn;
            node["Commission"] = pos.Commission;
            node["CloseProfit"] = pos.CloseProfit;
            node["PositionProfit"] = pos.PositionProfit;
            node["TradingDay"] = pos.TradingDay;
            node["SettlementID"] = pos.SettlementID;
            node["OpenCost"] = pos.OpenCost;
            node["ExchangeMargin"] = pos.ExchangeMargin;
            node["TodayPosition"] = pos.TodayPosition;
            node["MarginRateByMoney"] = pos.MarginRateByMoney;
            node["MarginRateByVolume"] = pos.MarginRateByVolume;
            node["ExchangeID"] = pos.ExchangeID;
            node["YdStrikeFrozen"] = pos.YdStrikeFrozen;
            value.append(node);
        }
        std::string json_text = value.toStyledString();
        return json_text;
    }

    std::string marshall( const CThostFtdcTradingAccountField* account){
        Json::Value value;

        ///经纪公司代码
        value["BrokerID"] = account->BrokerID ;
        ///投资者帐号
        value["AccountID"] = account->AccountID;
        ///上次质押金额
        value["PreMortgage"] = account->PreMortgage;
        ///上次信用额度
        value["PreCredit"] = account->PreCredit;
        ///上次存款额
        value["PreDeposit"] = account->PreDeposit;
        ///上次结算准备金
        value["PreBalance"] = account->PreBalance;
        ///上次占用的保证金
        value["PreMargin"] = account->PreMargin;
        ///利息基数
        value["InterestBase"] = account->InterestBase;
        ///利息收入
        value["Interest"] = account->Interest;
        ///入金金额
        value["Deposit"] = account->Deposit;
        ///出金金额
        value["Withdraw"] = account->Withdraw;
        ///冻结的保证金
        value["FrozenMargin"] = account->FrozenMargin;
        ///冻结的资金
        value["FrozenCash"] = account->FrozenCash;
        ///冻结的手续费
        value["FrozenCommission"] = account->FrozenCommission;
        ///当前保证金总额
        value["CurrMargin"] = account->CurrMargin;
        ///资金差额
        value["CashIn"] = account->CashIn;
        ///手续费
        value["Commission"] = account->Commission;
        ///平仓盈亏
        value["CloseProfit"] = account->CloseProfit;
        ///持仓盈亏
        value["PositionProfit"] = account->PositionProfit;
        ///期货结算准备金
        value["Balance"] = account->Balance;
        ///可用资金
        value["Available"] = account->Available;
        ///可取资金
        value["WithdrawQuota"] = account->WithdrawQuota;
        ///基本准备金
        value["Reserve"] = account->Reserve;
        ///交易日
        value["TradingDay"] = account->TradingDay;
        ///结算编号
        value["SettlementID"] = account->SettlementID;
        ///信用额度
        value["Credit"] = account->Credit;
        ///质押金额
        value["Mortgage"] = account->Mortgage;
        ///交易所保证金
        value["ExchangeMargin"] = account->ExchangeMargin;
        ///投资者交割保证金
        value["DeliveryMargin"] = account->DeliveryMargin;
        ///交易所交割保证金
        value["ExchangeDeliveryMargin"] = account->ExchangeDeliveryMargin;
        ///保底期货结算准备金
        value["ReserveBalance"] = account->ReserveBalance;
        ///币种代码
        value["CurrencyID"] = account->CurrencyID;
        ///上次货币质入金额
        value["PreFundMortgageIn"] = account->PreFundMortgageIn;
        ///上次货币质出金额
        value["PreFundMortgageOut"] = account->PreFundMortgageOut;
        ///货币质入金额
        value["FundMortgageIn"] = account->FundMortgageIn;
        ///货币质出金额
        value["FundMortgageOut"] = account->FundMortgageOut;
        ///货币质押余额
        value["FundMortgageAvailable"] = account->FundMortgageAvailable;
        ///可质押货币金额
        value["MortgageableFund"] = account->MortgageableFund;
        ///特殊产品占用保证金
        value["SpecProductMargin"] = account->SpecProductMargin;
        ///特殊产品冻结保证金
        value["SpecProductFrozenMargin"] = account->SpecProductFrozenMargin;
        ///特殊产品手续费
        value["SpecProductCommission"] = account->SpecProductCommission;
        ///特殊产品冻结手续费
        value["SpecProductFrozenCommission"] = account->SpecProductFrozenCommission;
        ///特殊产品持仓盈亏
        value["SpecProductPositionProfit"] = account->SpecProductPositionProfit;
        ///特殊产品平仓盈亏
        value["SpecProductCloseProfit"] = account->SpecProductCloseProfit;
        ///根据持仓盈亏算法计算的特殊产品持仓盈亏
        value["SpecProductPositionProfitByAlg"] = account->SpecProductPositionProfitByAlg;
        ///特殊产品交易所保证金
        value["SpecProductExchangeMargin"] = account->SpecProductExchangeMargin;
        ///业务类型
        value["BizType"] = account->BizType;

        std::string json_text = value.toStyledString();
        return json_text;
    }

    std::string marshall(const std::vector<CThostFtdcOrderField>& orders){
        Json::Value root(Json::arrayValue);
        for(auto & order : orders){
            if( order.OrderStatus == THOST_FTDC_OST_PartTradedQueueing ||
                order.OrderStatus == THOST_FTDC_OST_PartTradedNotQueueing ||
                order.OrderStatus == THOST_FTDC_OST_NoTradeQueueing
            ) {
                Json::Value node;
                marshall(&order, &node);
                root.append(node);
            }
        }
        std::string json_text = root.toStyledString();
        return json_text;
    }
}

