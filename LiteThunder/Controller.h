
#ifndef INNERPROC_INNERCONTROLLER_H
#define INNERPROC_INNERCONTROLLER_H

#include <memory>
#include <numeric>
#include <jsoncpp/json/json.h>
#include <boost/asio.hpp>
#include <ThostFtdcUserApiStruct.h>
#include <thread>
#include "base.h"
#include "datastruct.h"
#include "message.h"
#include "version.h"
#include "app.h"
#include "PosReceiver.h"
#include "behavior.h"
#include "trade.h"
#include "market.h"
#include "pubservice.h"
#include "PosReceiver.h"
#include "querybatch.h"

// typedef std::int8_t InstrumentRequirement;

 enum  InstrumentRequirement{
    None = 0 ,
    Instrument = 0x01,
    Margin = 0x02,
    Commission = 0x04
};

class Controller :  public PosListener,public  std::enable_shared_from_this<Controller> {

public:
    struct Settings {
        std::time_t start_time;          // 启动时间
        std::time_t login_time;
        std::string login_server_url;              // 登录服务器url
        std::string comm_server_ip;         // 通信服务器
        std::uint32_t comm_server_port;          // 通信服务器端口
        std::time_t establish_time;         //通信建立时间
        std::atomic_bool login_inited;           // 是否已经登录
        std::string token;                  // 接入服务的身份令牌 登录成功时返回
        int        tick_json;
        int         tick_raw;
        int         tick_text;
        Settings() {
            start_time = 0;
        }
    };
    typedef std::shared_ptr<Controller> Ptr;
public:
    Controller() {}
    bool        init(const Config &props);
    bool        open();
    void        close();
    void        run();
    void        onPositionChanged(PositionSignal ps);

    InstrumentConfig::Ptr getInstrumentByCommodity(CommodityName comm);

    static std::shared_ptr<Controller> &instance() {
        static std::shared_ptr<Controller> handle;
        if (!handle.get()) {
            handle = std::make_shared<Controller>();
        }
        return handle;
    }

    //返回CTP交易接口
    TdApi* getTradeApi() const { return  &TdApi::instance();}
    MdApi* getMarketApi() const { return &MdApi::instance();}

    void setBehavior(Behavior* behavior){ behavior_ = behavior;}
    Json::Value getStatusInfo();
    boost::asio::io_service &io_service() {    return io_service_; }
    
    //报单录入请求响应
//    void onOrderRespInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    //报单通知(内存在报单错误状态
    void onOrderReturn(CThostFtdcOrderField *order); // 委托回报
    ////委托单失败
    ///报单录入错误回报
    void onOrderErrorReturn(CThostFtdcInputOrderField * input , CThostFtdcRspInfoField *resp);

    //撤单回报 OnErrRtnOrderAction
//    void onOrderCancelReturn(CThostFtdcInputOrderActionField *action, CThostFtdcRspInfoField *resp);
    //撤单错误回报
    void onOrderCancelErrorReturn(CThostFtdcOrderActionField* action,CThostFtdcRspInfoField* resp);
    //成交回报
    void onTradeReturn(CThostFtdcTradeField *pTrade);
    //深度行情回报
    void onRtnDepthMarketData( CThostFtdcDepthMarketDataField *pDepthMarketData);
    //登录回报
    void onTradeUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    // 行情登录
    void onMarketUserLogin();
    void onTradeQueryResult(const std::vector< CThostFtdcTradeField >& trades );
    void onPositionQueryResult(const std::vector<  CThostFtdcInvestorPositionField > & positons);
    void onAccountQueryResult( CThostFtdcTradingAccountField * account);
    void onOrderQueryResult(const std::vector<CThostFtdcOrderField> & orders);
    void onInstrumentQueryResult(const std::map< std::string , CThostFtdcInstrumentField > & instruments);
    void onInstrumentCommissionRate(const std::vector< CThostFtdcInstrumentCommissionRateField > & commissions);
    void onInstrumentMarginRate(const std::vector< CThostFtdcInstrumentMarginRateField > & margins);

public:    
    std::shared_ptr< CThostFtdcDepthMarketDataField> getMarketData(const std::string & instrument) ;
    void appendOrderLog(const InstrumentId& instid, OrderRequest & req);
    void appendOrderLog(const InstrumentId& instid, CancelOrderRequest& req );
private:
    void workTimedTask();
    void resetStatus();
    int createOrder(OrderSlice::Ptr slice);
    void cancelOrder(OrderSlice::Ptr slice);
    
    void discardOrderAll();
//    void checkSlices(InstrumentConfig& instcfg);

    bool initInstruments();
    bool initPosition();
    bool initRedis();
    bool initPositionReceive();
    Redis *  getRedis() { return redis_;};
    bool dataReady( InstrumentRequirement& requirement, InstrumentId& instrument);
protected:
    void keepWalkingThread();
    DirectionPosition  getDirectionPosition( const InstrumentId& name, const std::string& direction);
    void showPosition();
    void walk(std::shared_ptr<InstrumentConfig> instrument);
    
protected:
    Config                      cfgs_;
    std::recursive_mutex        rmutex_;
    std::recursive_mutex        mtx_self;
    boost::asio::io_service     io_service_;
    Settings                    settings_;
    std::shared_ptr<boost::asio::steady_timer>      timer_;
    int                         check_timer_interval_;
    bool                        data_inited_;
    std::time_t                 last_check_time_ = 0;
    std::time_t                 last_heart_time_ = 0;
    std::time_t                 boot_time_ ;
    Behavior*                   behavior_ = NULL;
    // PosReceiver*                pos_receiver_ = NULL;
//    MessagePublisher*           msg_pub_ = NULL;
    std::map< InstrumentId, std::shared_ptr< CThostFtdcDepthMarketDataField>  >     market_datas_; 
//    std::map< InstrumentId, PositionHolding > position_holding_;

    std::map< CommodityName , std::shared_ptr<CommodityConfig> >  commodities_;    // 配置的交易品种
    std::map< InstrumentId, std::shared_ptr<InstrumentConfig> >  instruments_;    // 合约信息
    std::vector<CThostFtdcOrderField>  orders_; // 最新挂单列表

    std::recursive_mutex        mtx_market_data_;
    std::recursive_mutex        mtx_instruments_;
    std::thread     work_thread_;
    std::atomic_bool running_;
    // std::map< InstrumentId,  CThostFtdcInvestorPositionField > inst_positions_long_,inst_positions_short_;
    std::map< InstrumentId,  DirectionPosition > inst_positions_long_,inst_positions_short_;
    
    CThostFtdcTradingAccountField  account_info_;

    Redis * redis_ = nullptr;
    std::shared_ptr<ZMQ_PosReceiver>  pos_receiver_;
    std::atomic_bool    trade_user_login_ ; 
    std::atomic_bool    market_user_login_ ; 
    std::atomic_bool    position_return_ ;  // 查仓返回
    std::atomic_bool    order_return_ ;     // 查挂单返回


    struct OrderLogs{
        std::map< InstrumentId , std::vector<OrderRequest> > order_requests; 
        std::map< InstrumentId , std::vector<CancelOrderRequest> > order_cancels; 
    };

    OrderLogs   order_logs_;    //记录下单、撤单记录
    QueryBatchTimed     query_batch_;
};


#endif
