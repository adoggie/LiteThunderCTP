
#ifndef CTP_TRADE_H
#define CTP_TRADE_H

#include <jsoncpp/json/json.h>

#include <string>
#include <queue>
#include <functional>

//Boost
#define BOOST_PYTHON_STATIC_LIB
//#include <boost/python/module.hpp>	//python封装
//#include <boost/python/def.hpp>		//python封装
//#include <boost/python/dict.hpp>	//python封装
//#include <boost/python/object.hpp>	//python封装
//#include <boost/python.hpp>			//python封装
#include <boost/thread.hpp>			//任务队列的线程功能
//#include <boost/bind.hpp>			//任务队列的线程功能
#include <boost/any.hpp>			//任务队列的任务实现
#include <boost/asio.hpp>

//#include <thread>
#include <atomic>
#include <mutex>

//API
#include <ThostFtdcTraderApi.h>

#include "base.h"
#include "config.h"

using namespace boost;

//#define StrValueAssign(src,dest) src.copy(dest,sizeof(dest))
#define StrValueAssign(src,dest) strcpy(dest,src.c_str())
#define StrFromDataVariable(var) std::string( var ,sizeof(var)-1 )


///-------------------------------------------------------------------------------------
///C++ SPI的回调函数方法实现
///-------------------------------------------------------------------------------------
#define DIRECTION_LONG  THOST_FTDC_D_Buy
#define DIRECTION_SHORT THOST_FTDC_D_Sell
#define OFFSET_OPEN   "0"  // THOST_FTDC_OFEN_Open
#define OFFSET_CLOSE   "1"  //THOST_FTDC_OFEN_Close

#define PRINT_ORDER_INFO(ss,  input)  \
		ss<<"instrument:"<< input->InstrumentID \
		<< " exchange:"<< input->ExchangeID \
		<< " front:"<< input->FrontID \
		<< " session:"<< input->SessionID \
		<< " orderRef:"<< input->OrderRef \
		<< " order-sysid:"<< input->OrderSysID \
		<<" direction:"<< input->Direction \
		<< " openclose:" << input->CombOffsetFlag \
		<< " price:" << input->LimitPrice \
		<< " volume:" << input->VolumeTotalOriginal \
		<< " volumeTraded:" << input->VolumeTraded \
		<< " volumeTotal:" << input->VolumeTotal \
		<< " status:" << input->OrderStatus ;

#define PRINT_ORDER_INFO2(ss,first,input) \
		ss << first; 	\
		PRINT_ORDER_INFO(ss,input) ;

struct OrderRequest{
	std::time_t		time;
    std::string 	symbol;
    float   		price;
    int     		volume;
    std::string 	priceType;
    TThostFtdcDirectionType 	direction;  // THOST_FTDC_D_Buy
	TThostFtdcOffsetFlagEnType 	offset;
    int     					requestId;
    char    					price_type;
    std::string 				exchange_id;        // 交易所编号 scott 2019.10.11 added
    char 						cc;			// 条件单控制
    char 						tc; 		//时间控制
    char 						vc;  		//量控制
};

// 委托调用返回
struct OrderReturn{
    int error = 0  ;
    std::string symbol;
    std::string exchange;
    std::string order_ref;
    std::string order_local_id;
    int 		front_id;
    int 		session_id;
    std::string order_sys_id;   // 交易服务器返回的订单编号
    std::string order_id;
};

struct CancelOrderRequest{	

	std::time_t		time;
    std::string 	symbol;
	std::string exchange;
    std::string order_ref;
	int 		front_id;
    int  		session_id;
    std::string order_sys_id;   // 交易服务器返回的订单编号
    int     	requestId;
};

//API的继承实现
class TdApi : public CThostFtdcTraderSpi
{
private:
	CThostFtdcTraderApi* api;			//API对象

	std::string     user_id_;
    std::string     password_;
    std::string     broker_id_;
    std::string     auth_code_;
    std::string     product_info_;
	int             front_id_;
	int             session_id_;
	std::atomic_bool logined_ ;
    std::atomic_bool authed_;

	CThostFtdcRspUserLoginField ftd_login_field_;
//	std::map< std::string , CThostFtdcInvestorPositionField > positions_ , positions_tmp_; // 持仓列表
	std::vector<  CThostFtdcInvestorPositionField >                     positions_ ; // 持仓列表
	std::vector<  CThostFtdcInvestorPositionField >                     positions_back_ ; // 持仓列表
//	std::atomic< CThostFtdcTradingAccountField >  ftd_trade_account_;
	CThostFtdcTradingAccountField                                       ftd_trade_account_;
//    std::map< std::string , CThostFtdcInstrumentCommissionRateField >   commission_rates_; // 合约手续费
    std::vector< CThostFtdcInstrumentCommissionRateField >   commission_rates_; // 合约手续费
//    std::map< std::string , CThostFtdcInstrumentMarginRateField >       margin_rates_; // 合约保证金
    std::vector< CThostFtdcInstrumentMarginRateField >       margin_rates_; // 合约保证金
    std::map< std::string , CThostFtdcInstrumentField >                 instruments_; // 合约信息
    std::map< std::string , CThostFtdcDepthMarketDataField >            market_datas_; // 深度行情记录缓存
    std::map< std::string , CThostFtdcOrderField >                      orders_map_ , orders_map_back_; // 当前委托记录
    std::vector<  CThostFtdcOrderField >                      orders_ , orders_back_; // 当前委托记录
//    std::vector< CThostFtdcOrderField > orders_ , orders_back_; // 当前委托记录
//    std::map< std::string , CThostFtdcTradeField > trade_;  // 成交列表
    std::vector< CThostFtdcTradeField >                 trades_,trades_back_;  // 成交列表
    std::vector< std::string >                          instruments_query_;  // 查询的合约列表
    std::vector< std::function<void() > >               query_functions_ ;

    std::shared_ptr< boost::asio::deadline_timer> query_timer_;
    bool                    requireAuthentication_ = false;
	std::recursive_mutex              mutex_;
	std::recursive_mutex              mutex_query_;
    std::recursive_mutex 	rmutex_;
    std::atomic<int>        order_ref_;

public:
    void query_work_timer();

	int nextRequestId();
	void login();
	void authenticate();
    void queryAccount();
    void queryPosition();
    void queryInstrument(const std::string& instrument, const std::string& exchange="");
    void queryDepthMarketData(const std::string& instrument, const std::string& exchange);
    void queryCommissionRate(const std::string& instrument, const std::string& exchange); // 查询交易手续费
    void queryMarginRate(const std::string & instrument);
    void queryOrder();
    void queryTrade();

    void onError(const std::string& event,const int errcode,const std::string & errmsg);
    void onQueryResult(const std::string& event,const int errcode,const std::string & errmsg);

public:
    void scheduleQuery();
    bool isLogin(){ return logined_;}
    OrderReturn sendOrder(const OrderRequest& req);

    int cancelOrder(const CancelOrderRequest& req);
//    CThostFtdcTradingAccountField& getAccountInfoInternal();

    Json::Value getAccountInfo();
    CThostFtdcTradingAccountField getAccountInfoInner();

    std::vector<  CThostFtdcInvestorPositionField > getPositionsInner();
    Json::Value getPositions();


    //std::map< std::string , CThostFtdcOrderField >  getOrdersInner( std::string& user_no,  std::string& trans_no);
    std::vector< CThostFtdcOrderField >  getOrdersInner( std::string& user_no,  std::string& trans_no);
    Json::Value getOrders( std::string& user_no,  std::string& trans_no);

    Json::Value getTradeRecords();
    std::vector< CThostFtdcTradeField > getTradeRecordsInner();

    Json::Value getInstrumentInfo(const std::string& instrument);
    int queryInstrumentInfo(const std::string& instrument);
	void heartbeat();
public:
    static TdApi& instance();

	TdApi(){ }

	~TdApi()
	{
	};

	bool start();
	bool connect();
	void disconnect();
	void stop();

	//-------------------------------------------------------------------------------------
	//API回调函数
	//-------------------------------------------------------------------------------------

	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected();

	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	virtual void OnFrontDisconnected(int nReason);

	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	///@param nTimeLapse 距离上次接收报文的时间
	virtual void OnHeartBeatWarning(int nTimeLapse);

	///客户端认证响应
	virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;


	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///登出请求响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///报单录入请求响应
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///预埋单录入请求响应
	virtual void OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///预埋撤单录入请求响应
	virtual void OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///报单操作请求响应
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///查询最大报单数量响应
	//virtual void OnRspQueryMaxOrderVolume(CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///投资者结算结果确认响应
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///删除预埋单响应
	virtual void OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///删除预埋撤单响应
	virtual void OnRspRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///执行宣告录入请求响应
	virtual void OnRspExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///执行宣告操作请求响应
	virtual void OnRspExecOrderAction(CThostFtdcInputExecOrderActionField *pInputExecOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;


#ifndef _CHUANTOU
	///锁定应答
	virtual void OnRspLockInsert(CThostFtdcInputLockField *pInputLock, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
#endif
	///申请组合录入请求响应
	//virtual void OnRspCombActionInsert(CThostFtdcInputCombActionField *pInputCombAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///请求查询报单响应
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///请求查询成交响应
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///请求查询投资者持仓响应
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///请求查询资金账户响应
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///请求查询投资者响应
	virtual void OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

//	///请求查询交易编码响应
//	virtual void OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///请求查询合约保证金率响应
	virtual void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///请求查询合约手续费率响应
	virtual void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;


	///请求查询合约响应
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

//	///请求查询行情响应
//	virtual void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
//
//	///请求查询投资者结算结果响应
	virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

//	///请求查询转帐银行响应
//	virtual void OnRspQryTransferBank(CThostFtdcTransferBankField *pTransferBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///请求查询投资者持仓明细响应
	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

//	///请求查询客户通知响应
//	virtual void OnRspQryNotice(CThostFtdcNoticeField *pNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///请求查询结算信息确认响应
	virtual void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///请求查询投资者持仓明细响应
	virtual void OnRspQryInvestorPositionCombineDetail(CThostFtdcInvestorPositionCombineDetailField *pInvestorPositionCombineDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;


#ifndef _CHUANTOU
	///请求查询锁定应答
	virtual void OnRspQryLock(CThostFtdcLockField *pLock, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///请求查询锁定证券仓位应答
	virtual void OnRspQryLockPosition(CThostFtdcLockPositionField *pLockPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///请求查询投资者分级
	virtual void OnRspQryInvestorLevel(CThostFtdcInvestorLevelField *pInvestorLevel, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///请求查询E+1日行权冻结响应
	virtual void OnRspQryExecFreeze(CThostFtdcExecFreezeField *pExecFreeze, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
#endif

//	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	///报单通知
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder) ;

	///成交通知
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade) ;

	///报单录入错误回报
	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) ;

	///报单操作错误回报
	virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo) ;

#ifndef _CHUANTOU
	///锁定通知
	virtual void OnRtnLock(CThostFtdcLockField *pLock) ;

	///锁定错误通知
	virtual void OnErrRtnLockInsert(CThostFtdcInputLockField *pInputLock, CThostFtdcRspInfoField *pRspInfo) ;
#endif

	virtual void onFrontConnected(){};

	virtual void onFrontDisconnected(int i){};

	virtual void onHeartBeatWarning(int i){};

	//-------------------------------------------------------------------------------------
	//req:主动函数的请求字典
	//-------------------------------------------------------------------------------------

	void createFtdcTraderApi(std::string pszFlowPath = "");

	void release();

	void init(const Config & cfg);

	int join();

	int exit();

	std::string getTradingDay();

	void registerFront(std::string pszFrontAddress);

	void subscribePrivateTopic(int nType);

	void subscribePublicTopic(int nType);


};


#endif