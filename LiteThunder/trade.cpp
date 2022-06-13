
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "app.h"
#include "trade.h"
#include "Controller.h"


///-------------------------------------------------------------------------------------
///从Python对象到C++类型转换用的函数
///-------------------------------------------------------------------------------------
void assign(char* dest,size_t dest_size,const char* src){
	size_t size = strlen(src);
	dest_size = dest_size - 1;
	if( size > dest_size  ){
		size = dest_size;
	}
	memcpy(dest,src,size);
}

///-------------------------------------------------------------------------------------
///C++的回调函数将数据保存到队列中
///-------------------------------------------------------------------------------------

void TdApi::OnFrontConnected()
{
    Application::instance()->getLogger().info("!! -- Front Server Connected. -- !!");
    if( !requireAuthentication_ ){
        login();
    }else{
        authenticate();
    }
};

void TdApi::OnFrontDisconnected(int nReason){

};

void TdApi::OnHeartBeatWarning(int nTimeLapse){

};

void TdApi::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(pRspInfo->ErrorID == 0){
        authed_ = true;
        this->login();
        Application::instance()->getLogger().info("user authenticated okay.");
    }else{
        std::stringstream ss;
        ss << "authenticated error. code:" << pRspInfo->ErrorID;
        Application::instance()->getLogger().error(ss.str());
    }
};

//mark
void TdApi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    CThostFtdcSettlementInfoConfirmField req;
    if(pRspInfo->ErrorID == 0){ //登录成功
        logined_ = true;
        front_id_ = 0;
        front_id_ = (int)pRspUserLogin->FrontID;
        session_id_ = (int)pRspUserLogin->SessionID;
        std::string value ;
        value = pRspUserLogin->MaxOrderRef;
        boost::trim(value);
        order_ref_ = boost::lexical_cast<int>(value) ;
        ftd_login_field_ = *pRspUserLogin;

        memset(&req,0,sizeof(req));
        StrValueAssign(broker_id_, req.BrokerID);
        StrValueAssign(user_id_ , req.InvestorID);
//        Application::instance()->getLogger().debug("broker:"+ broker_id_ + " user_id:"+user_id_);
        int ret = this->api->ReqSettlementInfoConfirm(&req, nextRequestId());

        std::stringstream ss;
        ss<<"User Login Okay . ReqSettlementInfoConfirm CallRetCode:" << ret ;

//        this->queryInstrument("");
       // queryOrder();
        Controller::instance()->onTradeUserLogin(pRspUserLogin,pRspInfo,nRequestID,bIsLast);
        if(1){ // 2022.6.7
            // std::this_thread::sleep_for(std::chrono::seconds(3));
            // queryCommissionRate("", "");
            // std::this_thread::sleep_for(std::chrono::seconds(2));
            // queryMarginRate("");
            // std::this_thread::sleep_for(std::chrono::seconds(2));
            //    queryDepthMarketData(instrument,"");
            // queryInstrument("");
            // std::this_thread::sleep_for(std::chrono::seconds(1));

        }
    }else{
        logined_ = false;
//        Application::instance()->getLogger().error("Login Failed.");
    }



};

void TdApi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    logined_ = false;
    authed_ = false;

};

//埋单提交
void TdApi::OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){

};

void TdApi::OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

};


//如果撤单被CTP或交易所拒单，会回调OnRspOrderAction
// 或OnErrRtnOrderAction 函 数。只需要打印回调函数中的参数pRspInfo就可以得知被拒原因
// xxxAction : 报单的修改，激活，撤单等行为，只有撤单可以用
///撤单响应 ， 这里通过判别 resp 决定是否撤单拒绝
void TdApi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if( pRspInfo) {
        std::stringstream ss;
        ss << "<< OnRspOrderAction() [trade.cpp] " << pInputOrderAction->InstrumentID 
        << " , ErrorID: " << pRspInfo->ErrorID << " msg:" << pRspInfo->ErrorMsg        
        << "  OrderRef:" <<pInputOrderAction->OrderRef << " Front:"<< pInputOrderAction->FrontID
        << " Session" << pInputOrderAction->SessionID ;
        Application::instance()->getLogger().error(ss.str());

    }
    
//    if( pRspInfo->ErrorID) {
//        Controller::instance()->onOrderCancelErrorReturn(pInputOrderAction, pRspInfo);
//    }
};


///报单操作(撤单)错误回报 CTP或交易所风控未通过
void TdApi::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
    if(! pRspInfo){
        return ;
    }
    std::stringstream  ss;
    ss << "OnErrRtnOrderAction, ErrorID: " << pRspInfo->ErrorID ;
    Application::instance()->getLogger().debug( ss.str());

    Controller::instance()->onOrderCancelErrorReturn(pOrderAction,pRspInfo);
};

void TdApi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Application::instance()->getLogger().debug("processRspSettlementInfoConfirm()..");
};

void TdApi::OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
};

void TdApi::OnRspRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

};

void TdApi::OnRspExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

};

void TdApi::OnRspExecOrderAction(CThostFtdcInputExecOrderActionField *pInputExecOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

};


#ifndef _CHUANTOU
void TdApi::OnRspLockInsert(CThostFtdcInputLockField *pInputLock, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

};

#endif

// 20211112  , 11:11:12
void TdApi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if( !pOrder ){
        return ;
    }
    SCOPED_LOCK
    std::stringstream ss;
    ss << pOrder->InstrumentID << "." << pOrder->ExchangeID << "." << pOrder->OrderSysID;
    orders_map_back_[ss.str()] = *pOrder;
    orders_back_.push_back(*pOrder);
    if(bIsLast){
        orders_ = orders_back_;
        orders_map_ = orders_map_back_;
        Controller::instance()->onOrderQueryResult( orders_);
        // scheduleQuery();
    }
};

//成交记录返回
void TdApi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if( !pTrade){
        return ;
    }
    SCOPED_LOCK
    //Application::instance()->getLogger().debug("<< processRspQryTrade() ..");
    trades_back_.push_back( *pTrade);
    if( bIsLast){
        trades_ = trades_back_;
        Controller::instance()->onTradeQueryResult(trades_);
        scheduleQuery();
    }

};

void TdApi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!pInvestorPosition){
        return ;
    }

    SCOPED_LOCK
    positions_back_.push_back(*pInvestorPosition)  ;
    if( bIsLast){
        positions_ = positions_back_;
        Controller::instance()->onPositionQueryResult(positions_);
        scheduleQuery();
    }

};

//资金余额
void TdApi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){

    SCOPED_LOCK
    if( pTradingAccount){
        Controller::instance()->onAccountQueryResult(pTradingAccount);
    }
};

void TdApi::OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if( !pInvestor){
        return ;
    }
};


// ::cat 查询合约保证金
void TdApi::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate,
                                         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pInstrumentMarginRate)
	{
//		task.task_data = *pInstrumentMarginRate;
//        margin_rates_[ pInstrumentMarginRate->InstrumentID] = *pInstrumentMarginRate;
        margin_rates_.push_back( *pInstrumentMarginRate );
	}
    if(bIsLast){
        Controller::instance()->onInstrumentMarginRate(margin_rates_);
    }
};

void TdApi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate,
                                             CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(!pInstrumentCommissionRate){
        return;
    }
    SCOPED_LOCK
//    commission_rates_[ pInstrumentCommissionRate->InstrumentID] = *pInstrumentCommissionRate;
    commission_rates_.emplace_back(*pInstrumentCommissionRate);

    std::stringstream ss;
    ss << "processRspQryInstrumentCommissionRate() , instrument:" << pInstrumentCommissionRate->InstrumentID;
    Application::instance()->getLogger().debug( ss.str());
    if(bIsLast){
        Controller::instance()->onInstrumentCommissionRate(commission_rates_);
    }
};


void TdApi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(  !pInstrument){
        return ;
    }
    instruments_[ std::string(pInstrument->InstrumentID) ] = *pInstrument;
    // Application::instance()->getLogger().debug("-- OnRspQryInstrument() " + std::string(pInstrument->InstrumentID) );
    if( bIsLast){
        std::stringstream ss;
        ss << "OnRspQryInstrument()  instrument count:" << instruments_.size() << std::endl;
        Application::instance()->getLogger().debug( ss.str());
        Controller::instance()->onInstrumentQueryResult(instruments_);
//        std::vector<std::string> ins;
//        std::map<std::string, std::string> entries;
//        for (auto & pair : instruments_) {
//            if( pair.first.size() <=7) {
//                ins.emplace_back(pair.first);
//                entries[pair.first] = "ON" ;
//            }
//        }

//        Controller::instance()->getMarketApi()->subscribeMarketData(ins);
//        Redis* redis = MessagePublisher::instance().getRedis();
//        if( redis ){
//            redis->hmset("ctp_instruments", entries.begin(), entries.end());
//            std::cout<<"instruments :" << ins.size() << std::endl;
//        }
    }
};

void TdApi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Application::instance()->getLogger().debug( "processRspQrySettlementInfo() ..");

};

void TdApi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

};

void TdApi::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

void TdApi::OnRspQryInvestorPositionCombineDetail(CThostFtdcInvestorPositionCombineDetailField *pInvestorPositionCombineDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

};

#ifndef _CHUANTOU

void TdApi::OnRspQryLock(CThostFtdcLockField *pLock, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

};

void TdApi::OnRspQryLockPosition(CThostFtdcLockPositionField *pLockPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

};

void TdApi::OnRspQryInvestorLevel(CThostFtdcInvestorLevelField *pInvestorLevel, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

};

void TdApi::OnRspQryExecFreeze(CThostFtdcExecFreezeField *pExecFreeze, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

};
#endif

void TdApi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
//    if(pRspInfo->ErrorID) {
//        Controller::instance()->onOrderErrorReturn(pInputOrder, pRspInfo);
//    }
};

void TdApi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    if(!pOrder){
        return ;
    }
    SCOPED_LOCK
    std::stringstream ss;
    ss << pOrder->InstrumentID << "." << pOrder->ExchangeID << "." << pOrder->OrderSysID;
    orders_map_[ss.str()] = *pOrder;

	Controller::instance()->onOrderReturn(pOrder);
};

void TdApi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    if(!pTrade){
        return;
    }
	Controller::instance()->onTradeReturn(pTrade);
};

///报单录入错误回报
void TdApi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
    if(!pInputOrder){
        return;
    }
    Controller::instance()->onOrderErrorReturn(pInputOrder,pRspInfo );
};



#ifndef _CHUANTOU
void TdApi::OnRtnLock(CThostFtdcLockField *pLock)
{

};

void TdApi::OnErrRtnLockInsert(CThostFtdcInputLockField *pInputLock, CThostFtdcRspInfoField *pRspInfo)
{

};

#endif

void TdApi::createFtdcTraderApi(std::string pszFlowPath)
{
	this->api = CThostFtdcTraderApi::CreateFtdcTraderApi(pszFlowPath.c_str());
	this->api->RegisterSpi(this);
};

void TdApi::release()
{
	this->api->Release();
};

void TdApi::init(const Config & cfg)
{

};

int TdApi::join()
{
	int i = this->api->Join();
	return i;
};

int TdApi::exit()
{
	//该函数在原生API里没有，用于安全退出API用，原生的join似乎不太稳定
	this->api->RegisterSpi(NULL);
	this->api->Release();
	this->api = NULL;
	return 1;
};

std::string TdApi::getTradingDay()
{
    std::string day = this->api->GetTradingDay();
	return day;
};

void TdApi::registerFront(std::string pszFrontAddress)
{
	this->api->RegisterFront((char*)pszFrontAddress.c_str());
};

void TdApi::subscribePrivateTopic(int nType)
{
	//该函数为手动编写
	THOST_TE_RESUME_TYPE type;

	switch (nType)
	{
	case 0:
	{
		type = THOST_TERT_RESTART;
		break;
	};

	case 1:
	{
		type = THOST_TERT_RESUME;
		break;
	};

	case 2:
	{
		type = THOST_TERT_QUICK;
		break;
	};
	}

	this->api->SubscribePrivateTopic(type);
};

void TdApi::subscribePublicTopic(int nType)
{
	//该函数为手动编写
	THOST_TE_RESUME_TYPE type;

	switch (nType)
	{
	case 0:
	{
		type = THOST_TERT_RESTART;
		break;
	};

	case 1:
	{
		type = THOST_TERT_RESUME;
		break;
	};

	case 2:
	{
		type = THOST_TERT_QUICK;
		break;
	};
	}

	this->api->SubscribePublicTopic(type);
};


void TdApi::scheduleQuery(){
    int query_interval;
    Config & cfg = Application::instance()->getConfig();

    query_interval = cfg.get_int("ctp.trade.query_interval",1) ;
    if( !query_timer_) {
        query_timer_ = std::make_shared<boost::asio::deadline_timer >(Controller::instance()->io_service(),
                                                                      boost::posix_time::seconds(query_interval));
    }
    query_timer_->async_wait(std::bind(&TdApi::query_work_timer, this));

}


void TdApi::query_work_timer(){
    Config & cfg = Application::instance()->getConfig();
    int query_interval = cfg.get_int("ctp.trade.query_interval",1) ;

    std::lock_guard<std::mutex> lock(mutex_query_);

    query_timer_->expires_from_now(boost::posix_time::seconds(query_interval));
//    Application::instance()->getLogger().debug("query_work_timer()..");
    if( !logined_){
        scheduleQuery();
        return ;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto head = query_functions_.begin();
    if( head != query_functions_.end()){
        auto fx = *head;
        fx();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        query_functions_.erase(head);
        query_functions_.push_back(fx);
        std::stringstream ss;
        ss << "function number:" << query_functions_.size();
        //Application::instance()->getLogger().debug(ss.str());
        heartbeat();
    }
    //scheduleQuery();
}

int TdApi::nextRequestId(){
    static int req_id = int( std::time(NULL) - 1654591866);
    return req_id++;
}

void TdApi::login(){
    if( logined_){
        return;
    }
    CThostFtdcReqUserLoginField myreq = CThostFtdcReqUserLoginField();
    memset(&myreq, 0, sizeof(myreq));
    user_id_.copy(myreq.UserID,sizeof(myreq.UserID));
    broker_id_.copy(myreq.BrokerID,sizeof(myreq.BrokerID));
    password_.copy(myreq.Password,sizeof(myreq.Password));
    int i = this->api->ReqUserLogin(&myreq, nextRequestId());
}

void TdApi::authenticate(){
    CThostFtdcReqAuthenticateField myreq = CThostFtdcReqAuthenticateField();
    memset(&myreq, 0, sizeof(myreq));
    //StrValueAssign( user_id_, myreq.UserID);
    //StrValueAssign( auth_code_, myreq.AuthCode);
    //StrValueAssign( broker_id_, myreq.BrokerID);

    strcpy(myreq.UserID,user_id_.c_str());
    strcpy(myreq.AuthCode,auth_code_.c_str());
    strcpy(myreq.BrokerID, broker_id_.c_str());
    strcpy(myreq.AppID, product_info_.c_str());

#ifdef _CHUANTOU
    // todo enable AppID
   // StrValueAssign( product_info_, myreq.AppID);
#endif

    int i = this->api->ReqAuthenticate(&myreq, nextRequestId());
}



bool TdApi::start(){
    logined_ = false;
    authed_ = false;
//    ConnectionOptions connection_options;
    Config & cfg = Application::instance()->getConfig();
    if ( cfg.get_int("ctp.trade.query_position",0)) {
        query_functions_.push_back(std::bind(&TdApi::queryPosition, this));
    }
    if( cfg.get_int("ctp.trade.query_account",0)) {
        query_functions_.push_back(std::bind(&TdApi::queryAccount, this));
    }

    if( cfg.get_int("ctp.trade.query_order",0)) {
        query_functions_.push_back(std::bind(&TdApi::queryOrder, this));
    }
    if( cfg.get_int("ctp.trade.query_trade",0)) {
        query_functions_.push_back(std::bind(&TdApi::queryTrade, this));
    }

//    std::string text = cfg.get_string("ctp.instruments");
//    boost::split(instruments_query_,text,boost::is_any_of(", "));

    connect();
    // scheduleQuery();
    return true;
}

void TdApi::stop(){
    this->exit();
//    delete redis_;
}

void TdApi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
    std::string text = (boost::format("TdApi:: onRspError = error-id:%d , error-msg:%s") % pRspInfo->ErrorID %
                        pRspInfo->ErrorMsg).str();
    Application::instance()->getLogger().debug(text );
}

bool TdApi::connect(){
    Config& cfg = Application::instance()->getConfig();
    user_id_ = cfg.get_string("ctp.user_id");
    broker_id_ = cfg.get_string("ctp.broker_id");
    password_ = cfg.get_string("ctp.password");
    auth_code_ = cfg.get_string("ctp.auth_code");
    product_info_ = cfg.get_string("ctp.product_info");

    std::string conpath = cfg.get_string("ctp.con_path","./cons");
    std::string address = cfg.get_string("ctp.td_addr");

    std::string req_auth ;
    req_auth = cfg.get_string("ctp.require_auth");
    if( req_auth == "true"){
        requireAuthentication_ = true;
    }

    createFtdcTraderApi(conpath);

    subscribePrivateTopic(1);
    subscribePublicTopic(1);

    registerFront(address);
    this->api->Init();

//    std::this_thread::sleep_for(2s);

//    dict req ;
//    req["UserID"] = cfg.get_string("ctp.user_id");
//    req["Password"] = cfg.get_string("ctp.password");
//    req["BrokerID"] = cfg.get_string("ctp.broker_id");
//    int req_id;
//    req_id = 1;
//    reqUserLogin(req,req_id);
    return true;
}

///请求查询合约保证金率
void TdApi::queryMarginRate(const std::string & instrument){
    CThostFtdcQryInstrumentMarginRateField myreq = CThostFtdcQryInstrumentMarginRateField();
    memset(&myreq, 0, sizeof(myreq));

//    user_id_.copy(myreq.InvestorID,sizeof(myreq.InvestorID));
//    broker_id_.copy(myreq.BrokerID, sizeof(myreq.BrokerID));
//    StrValueAssign(instrument,myreq.InstrumentID);
//请求查询合约保证金率
    int ret = this->api->ReqQryInstrumentMarginRate(&myreq, nextRequestId());
    if(ret){
        std::stringstream ss;
        ss<<"ReqQryInstrumentMarginRate Failed, Code: " << ret ;
        Application::instance()->getLogger().error(ss.str());
    }

}

// http  用户触发查询
int TdApi::queryInstrumentInfo(const std::string& instrument){
    std::lock_guard<std::mutex> lock(mutex_query_);

    if( !logined_){
        return -1;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    queryCommissionRate(instrument,"");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    queryMarginRate(instrument);
    std::this_thread::sleep_for(std::chrono::seconds(1));
//    queryDepthMarketData(instrument,"");
    queryInstrument(instrument);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}



void TdApi::queryAccount(){
    SCOPED_LOCK
//    dict req;
//    reqQryTradingAccount(req,nextRequestId());
    //Application::instance()->getLogger().debug("queryAccount() ..");

    CThostFtdcQryTradingAccountField myreq = CThostFtdcQryTradingAccountField();
    memset(&myreq, 0, sizeof(myreq));

//    getStr(req, "CurrencyID", myreq.CurrencyID);
//    getStr(req, "InvestorID", myreq.InvestorID);
//    getChar(req, "BizType", &myreq.BizType);
//    getStr(req, "BrokerID", myreq.BrokerID);

    broker_id_.copy(myreq.BrokerID,sizeof(myreq.BrokerID));
    user_id_.copy(myreq.InvestorID, sizeof(myreq.InvestorID));
    int ret = this->api->ReqQryTradingAccount(&myreq, nextRequestId());
    
    this->onQueryResult("queryAccount",ret,"");


}


void TdApi::onQueryResult(const std::string& event,const int ret,const std::string & errmsg){
    if(ret){
        std::stringstream ss;
        Application::instance()->getLogger().error(ss.str());
        ss << "Code:" << ret;
        if( ret == -1){
            ss << " Network Failed ";
        } else if( ret == -2){
            ss << " Pending Requests Exceed Limit";
        }else if( ret == -3){
            ss << " Sending Speed Exceed Limit";
        }
        ss << " " << errmsg;
        Application::instance()->getLogger().error(ss.str());
        // this->onError(event,ret,ss.str());
    }
}

//发送委托单记录，异步调用，委托记录在 getOrders() 返回
void TdApi::queryOrder(){
    SCOPED_LOCK    
    CThostFtdcQryOrderField myreq;
    memset(&myreq, 0, sizeof(myreq));
    orders_back_.clear();
    orders_map_back_.clear();

    int ret = api->ReqQryOrder(&myreq, nextRequestId());
    onQueryResult("queryOrder",ret,"");
    
}

// 查询成交记录
void TdApi::queryTrade(){
    SCOPED_LOCK
    CThostFtdcQryTradeField req;
    memset(&req, 0, sizeof(req));
    trades_back_.clear();
    int ret = api->ReqQryTrade(&req,nextRequestId());
    if(ret){
        std::stringstream ss;
        ss<<"ReqQryTrade Failed, Code: " << ret ;
        Application::instance()->getLogger().error(ss.str());
    }
}

void TdApi::queryPosition(){
    SCOPED_LOCK
    //Application::instance()->getLogger().debug("queryPosition()..");
    CThostFtdcQryInvestorPositionField myreq = CThostFtdcQryInvestorPositionField();
    memset(&myreq, 0, sizeof(myreq));
    broker_id_.copy(myreq.BrokerID,sizeof(myreq.BrokerID));
    user_id_.copy(myreq.InvestorID, sizeof(myreq.InvestorID));
    positions_back_.clear();
    int i = this->api->ReqQryInvestorPosition(&myreq, nextRequestId());
    this->onQueryResult("queryPosition",i,"");
}

void TdApi::queryDepthMarketData(const std::string& instrument, const std::string& exchange){
    SCOPED_LOCK
}

//查询合约信息
void TdApi::queryInstrument(const std::string& instrument, const std::string& exchange){
    SCOPED_LOCK

    CThostFtdcQryInstrumentField req;
    memset(&req,0,sizeof(req));
    api->ReqQryInstrument(&req,nextRequestId());
}

//查询交易手续费
void TdApi::queryCommissionRate(const std::string& instrument, const std::string& exchange){
    SCOPED_LOCK

    CThostFtdcQryInstrumentCommissionRateField myreq = CThostFtdcQryInstrumentCommissionRateField();
    memset(&myreq, 0, sizeof(myreq));

//    instrument.copy(myreq.InstrumentID, sizeof myreq.InstrumentID );
   user_id_.copy(myreq.InvestorID,sizeof(myreq.InvestorID));
   broker_id_.copy(myreq.BrokerID,sizeof(myreq.BrokerID));
   
//    exchange.copy(myreq.ExchangeID,sizeof(myreq.ExchangeID));

    int i = this->api->ReqQryInstrumentCommissionRate(&myreq, nextRequestId());
    std::stringstream ss;
    ss << ">> queryCommissionRate() callret:" << i ;
    Application::instance()->getLogger().debug( ss.str());
}

/// 发送委托
OrderReturn TdApi::sendOrder(const OrderRequest& req){
    OrderReturn ret;

    CThostFtdcInputOrderField r;
    memset(&r, 0, sizeof(r));

    StrValueAssign(req.symbol, r.InstrumentID);
    r.LimitPrice =  req.price;
    r.VolumeTotalOriginal = req.volume;
    r.Direction = req.direction;
    // StrValueAssign(req.offset,r.CombOffsetFlag);    
    r.CombOffsetFlag[0] = req.offset;
    
    StrValueAssign(user_id_ , r.InvestorID);
    StrValueAssign(user_id_ , r.UserID);
    StrValueAssign(broker_id_ , r.BrokerID);
    StrValueAssign( std::string("1") , r.CombHedgeFlag);
//    StrValueAssign( std::string("DCE") , r.ExchangeID);
    StrValueAssign( req.exchange_id , r.ExchangeID);

//    r.ContingentCondition = THOST_FTDC_CC_Immediately ; // 立即触发
    r.ContingentCondition = req.cc ; // 立即触发
    r.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    r.IsAutoSuspend = 0;
//    r.TimeCondition = THOST_FTDC_TC_GFD; // 当日有效
    r.TimeCondition = req.tc; // 当日有效
    r.VolumeCondition = req.vc;  // 任何数量
    //# 判断FAK和FOK
    // FOK
    r.OrderPriceType = req.price_type; // THOST_FTDC_OPT_LimitPrice;
//    r.TimeCondition = THOST_FTDC_TC_IOC; // 立即完成，否则撤销
//    r.VolumeCondition = THOST_FTDC_VC_CV;  // 全部数量
//    r.VolumeCondition = THOST_FTDC_VC_CV;  // 全部数量

    r.ContingentCondition = THOST_FTDC_CC_Immediately ; // 立即触发
    r.TimeCondition = THOST_FTDC_TC_GFD;    //
    r.VolumeCondition = THOST_FTDC_VC_AV;   // 
   

    r.RequestID = nextRequestId();
    r.MinVolume = 1;

    order_ref_ +=1 ;
    std::stringstream ss;
    ss<< order_ref_;
    StrValueAssign(ss.str(),r.OrderRef);

    std::string order_id;
    ret.error = api->ReqOrderInsert(&r, r.RequestID);
    if(ret.error){
        std::stringstream ss;
        ss<<"!! SendOrder Failed, Code: " << ret.error ;
        Application::instance()->getLogger().error(ss.str());
        return ret ;
    }
    ret.front_id = front_id_;
    ret.session_id = session_id_;
    ret.order_ref = std::to_string(order_ref_);
    ret.symbol = req.symbol;
    ss.str("");
    ss << "    front:"<< front_id_ << " session:" << session_id_
       <<  " order_ref:" << order_ref_;
    Application::instance()->getLogger().warn(ss.str());        


//     *  UserOrderId : A-FrontID-SessionID-OrderRef
//     *  SysOrderId : B-ExchangeID-OrderSysID
//     *
//    ss.clear();
//    ss.str("");
//    ss<< "A#" << front_id_ <<"#"<< session_id_ << "#" << (int)order_ref_;
//    ss<< "A#" << front_id_ <<"#"<< session_id_ << "#" << (int)order_ref_;
//    ret.order_id = ss.str();
    // ret.order_id = ret.order_ref;
    return ret;
}



// 撤单
int TdApi::cancelOrder(const CancelOrderRequest& req){
    CThostFtdcInputOrderActionField r;
    memset(&r, 0, sizeof(r));
    StrValueAssign(req.symbol,r.InstrumentID);

    StrValueAssign(req.exchange, r.ExchangeID);
    StrValueAssign(req.order_sys_id , r.OrderSysID);
    StrValueAssign(broker_id_ , r.BrokerID );
    StrValueAssign(user_id_ , r.InvestorID);
    StrValueAssign(req.order_ref, r.OrderRef);
    r.ActionFlag = THOST_FTDC_AF_Delete;
    r.FrontID = req.front_id;
    r.SessionID = req.session_id;

    std::stringstream ss;
    ss << ">> cancelOrder() exchange:"<<r.ExchangeID << " order_sys_Id:"<< req.order_sys_id << 
        " borker_id:"<< r.BrokerID << " investor:" << r.InvestorID << " order_ref:" << r.OrderRef 
        << " front_id:"<< r.FrontID << " session:" << r.SessionID ;
    Application::instance()->getLogger().info(ss.str());

    int ret = api->ReqOrderAction(&r,nextRequestId());

    if(ret){
        std::stringstream ss;
        ss<<"cancelOrder Failed, Code: " << ret ;
        Application::instance()->getLogger().error(ss.str());
    }

    return ret;
}

//CThostFtdcTradingAccountField& TdApi::getAccountInfoInternal(){
//    return ftd_trade_account_;
//}

std::vector<  CThostFtdcInvestorPositionField >
TdApi::getPositionsInner(){
    return positions_;
}

CThostFtdcTradingAccountField TdApi::getAccountInfoInner() {
    SCOPED_LOCK
    return ftd_trade_account_;
}

Json::Value TdApi::getAccountInfo(){

    Json::Value value;
    value["BrokerID"] = ftd_trade_account_.BrokerID;
    value["AccountID"] = ftd_trade_account_.AccountID;
    value["Deposit"] = ftd_trade_account_.Deposit;
    value["Withdraw"] = ftd_trade_account_.Withdraw;
    value["FrozenMargin"] = ftd_trade_account_.FrozenMargin;
    value["FrozenCash"] = ftd_trade_account_.FrozenCash;
    value["CurrMargin"] = ftd_trade_account_.CurrMargin;
    value["CloseProfit"] = ftd_trade_account_.CloseProfit;
    value["PositionProfit"] = ftd_trade_account_.PositionProfit;
    value["Balance"] = ftd_trade_account_.Balance;
    value["Available"] = ftd_trade_account_.Available;
    value["WithdrawQuota"] = ftd_trade_account_.WithdrawQuota;
    value["TradingDay"] = ftd_trade_account_.TradingDay;
    value["SettlementID"] = ftd_trade_account_.SettlementID;
    return value;
}

Json::Value TdApi::getPositions(){
    SCOPED_LOCK
    Json::Value value;
    for(auto & pos : positions_){ //CThostFtdcInvestorPositionField
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
    return value;
}


std::vector< CThostFtdcOrderField >
TdApi::getOrdersInner( std::string& user_no,  std::string& trans_no){
    return orders_;
}

//返回所有当前委托
// Just get current orders.
Json::Value TdApi::getOrders( std::string& user_no,  std::string& trans_no){
    SCOPED_LOCK
    Json::Value value;
    std::vector<  CThostFtdcOrderField > orders;
    for(auto &kv : orders_map_){
        orders.push_back(kv.second);
    }
    Controller::instance()->onOrderQueryResult( orders);

    return value ;


    std::replace(user_no.begin(),user_no.end(),'.','#');
    std::replace(trans_no.begin(),trans_no.end(),'.','#');


    for(auto& order : orders_){
        Json::Value node;
       // CThostFtdcOrderField& order = itr.second;
//        CThostFtdcOrderField& order = itr;
        node["InstrumentID"] = order.InstrumentID;
        node["OrderRef"] = order.OrderRef;
        node["UserID"] = order.UserID;
        node["OrderPriceType"] = order.OrderPriceType;
        node["Direction"] = std::string(1,order.Direction);
        node["CombOffsetFlag"] = order.CombOffsetFlag;
        node["CombHedgeFlag"] = order.CombHedgeFlag;
        node["LimitPrice"] = order.LimitPrice;
        node["VolumeTotalOriginal"] = order.VolumeTotalOriginal;
        node["TimeCondition"] = std::string(1,order.TimeCondition);
        node["GTDDate"] = order.GTDDate;
        node["VolumeCondition"] = std::string(1,order.VolumeCondition);
        node["MinVolume"] = order.MinVolume;
        node["ContingentCondition"] = order.ContingentCondition;
        node["StopPrice"] = order.StopPrice;
        node["ForceCloseReason"] = std::string(1,order.ForceCloseReason);
        node["IsAutoSuspend"] = order.IsAutoSuspend;
        node["RequestID"] = order.RequestID;
        node["OrderLocalID"] = order.OrderLocalID;
        node["ExchangeID"] = order.ExchangeID;
        node["ClientID"] = order.ClientID;
        node["OrderSubmitStatus"] = order.OrderSubmitStatus;
        node["NotifySequence"] = order.NotifySequence;
        node["TradingDay"] = order.TradingDay;
        node["SettlementID"] = order.SettlementID;
        node["OrderSysID"] = order.OrderSysID;
        node["OrderSource"] = order.OrderSource;
        node["OrderStatus"] = order.OrderStatus;
        node["OrderType"] = order.OrderType;
        node["VolumeTraded"] = order.VolumeTraded;
        node["VolumeTotal"] = order.VolumeTotal;
        node["InsertDate"] = order.InsertDate;
        node["InsertTime"] = order.InsertTime;
        node["ActiveTime"] = order.ActiveTime;
        node["SuspendTime"] = order.SuspendTime;
        node["UpdateTime"] = order.UpdateTime;
        node["CancelTime"] = order.CancelTime;
        node["SequenceNo"] = order.SequenceNo;
        node["FrontID"] = order.FrontID;
        node["SessionID"] = order.SessionID;
        node["UserProductInfo"] = order.UserProductInfo;
        node["StatusMsg"] = order.StatusMsg;
        node["UserForceClose"] = order.UserForceClose;
        node["BrokerOrderSeq"] = order.BrokerOrderSeq;
        node["BranchID"] = order.BranchID;

        if( user_no.size()){
            std::stringstream ss;
            ss<< "A#" << order.FrontID <<"#"<< order.SessionID << "#" << StrFromDataVariable(order.OrderRef);
            if( ss.str() == user_no){
                value.append(node);
            }
        }else if ( trans_no.size()){
            std::stringstream ss;
            ss<< "B#" << StrFromDataVariable(order.ExchangeID) <<"#"<< StrFromDataVariable(order.OrderSysID );
            if( ss.str() == trans_no){
                value.append(node);
            }
        }else {
            value.append(node);
        }
    }
    return value;
}

std::vector< CThostFtdcTradeField > TdApi::getTradeRecordsInner() {
    SCOPED_LOCK
    return trades_;
}

Json::Value TdApi::getTradeRecords(){
    SCOPED_LOCK

    Json::Value value;
    for(auto& trade : trades_){
        Json::Value node;
//        CThostFtdcTradeField& order = itr.second;
        node["InstrumentID"] = trade.InstrumentID;
        node["OrderRef"] = trade.OrderRef;
        node["UserID"] = trade.UserID;
        node["ExchangeID"] = trade.ExchangeID;
        node["TradeID"] = trade.TradeID;
        node["Direction"] = std::string(1,trade.Direction);
        node["OrderSysID"] = trade.OrderSysID;
        node["ParticipantID"] = trade.ParticipantID;
        node["ClientID"] = trade.ClientID;
        node["TradingRole"] = trade.TradingRole;
        node["ExchangeInstID"] = trade.ExchangeInstID;
        node["OffsetFlag"] = trade.OffsetFlag;
        node["HedgeFlag"] = std::string(1, trade.HedgeFlag);
        node["Price"] = trade.Price;
        node["Volume"] = trade.Volume;
        node["TradeDate"] = trade.TradeDate;
        node["TradeTime"] = trade.TradeTime;
        node["TradeType"] = trade.TradeType;
        node["PriceSource"] = trade.PriceSource;
        node["TraderID"] = trade.TraderID;
        node["OrderLocalID"] = trade.OrderLocalID;
        node["ClearingPartID"] = trade.ClearingPartID;
        node["BusinessUnit"] = trade.BusinessUnit;
        node["SequenceNo"] = trade.SequenceNo;
        node["TradingDay"] = trade.TradingDay;
        node["SettlementID"] = trade.SettlementID;
        node["BrokerOrderSeq"] = trade.BrokerOrderSeq;
        node["TradeSource"] = trade.TradeSource;
        value.append(node);
    }
    return value;
}

Json::Value TdApi::getInstrumentInfo(const std::string& instrument){
    SCOPED_LOCK
    Json::Value value;
    value["symbol"] = instrument;

    {
        auto itr = instruments_.find(instrument);
        if( itr != instruments_.end()){
            Json::Value node;
            auto & r = itr->second;
            node["InstrumentID"] =  r.InstrumentID;
            node["ExchangeID"] =  r.ExchangeID;
            node["InstrumentName"] =  r.InstrumentName;
            node["ExchangeInstID"] =  r.ExchangeInstID;
            node["ProductID"] =  r.ProductID;
            node["ProductClass"] =  r.ProductClass;
            node["DeliveryYear"] =  r.DeliveryYear;
            node["DeliveryMonth"] =  r.DeliveryMonth;
            node["MaxMarketOrderVolume"] =  r.MaxMarketOrderVolume;
            node["MinMarketOrderVolume"] =  r.MinMarketOrderVolume;
            node["MaxLimitOrderVolume"] =  r.MaxLimitOrderVolume;
            node["MinLimitOrderVolume"] =  r.MinLimitOrderVolume;
            node["VolumeMultiple"] =  r.VolumeMultiple;
            node["PriceTick"] =  r.PriceTick;
            node["CreateDate"] =  r.CreateDate;
            node["OpenDate"] =  r.OpenDate;
            node["ExpireDate"] =  r.ExpireDate;
            node["StartDelivDate"] =  r.StartDelivDate;
            node["EndDelivDate"] =  r.EndDelivDate;
            node["InstLifePhase"] =  r.InstLifePhase;
            node["IsTrading"] =  r.IsTrading;
            node["PositionType"] =  r.PositionType;
            node["PositionDateType"] =  r.PositionDateType;
            node["LongMarginRatio"] =  r.LongMarginRatio;
            node["ShortMarginRatio"] =  r.ShortMarginRatio;
            node["MaxMarginSideAlgorithm"] =  r.MaxMarginSideAlgorithm;
            node["UnderlyingInstrID"] =  r.UnderlyingInstrID;
            node["StrikePrice"] =  r.StrikePrice;
            node["OptionsType"] =  r.OptionsType;
            node["UnderlyingMultiple"] =  r.UnderlyingMultiple;
            node["CombinationType"] =  r.CombinationType;
#ifndef _CHUANTOU
            node["MinBuyVolume"] =  r.MinBuyVolume;
            node["MinSellVolume"] =  r.MinSellVolume;
            node["InstrumentCode"] =  r.InstrumentCode;
#endif
            value["instrument"] = node;
        }
    }
    // margin rate
//    {
//        auto itr = margin_rates_.find(instrument);
//        if (itr != margin_rates_.end()) {
//            Json::Value node;
//            auto &r = itr->second;
//            node["InstrumentID"] = r.InstrumentID;
//            node["InvestorRange"] = r.InvestorRange;
//            node["BrokerID"] = r.BrokerID;
//            node["InvestorID"] = r.InvestorID;
//            node["HedgeFlag"] = std::string(1,r.HedgeFlag);
//            node["LongMarginRatioByMoney"] = r.LongMarginRatioByMoney;
//            node["LongMarginRatioByVolume"] = r.LongMarginRatioByVolume;
//            node["ShortMarginRatioByMoney"] = r.ShortMarginRatioByMoney;
//            node["ShortMarginRatioByVolume"] = r.ShortMarginRatioByVolume;
//            node["IsRelative"] = r.IsRelative;
//
//            value["margin_rate"] = node;
//        }
//
//    }

    // commission rate
//    {
//        auto itr = commission_rates_.find(instrument);
//        for(auto itr: commission_rates_){
//            auto & name = itr.first;
//            if( instrument.find_first_of(name) == std::string::npos){
//                continue;
//            }
//
////        if (itr != commission_rates_.end()) {
//            Json::Value node;
//            auto &r = itr.second;
//            node["InstrumentID"] = r.InstrumentID;
//            node["InvestorRange"] = r.InvestorRange;
//            node["BrokerID"] = r.BrokerID;
//            node["InvestorID"] = r.InvestorID;
//            node["OpenRatioByMoney"] = r.OpenRatioByMoney;
//            node["OpenRatioByVolume"] = r.OpenRatioByVolume;
//            node["CloseRatioByMoney"] = r.CloseRatioByMoney;
//            node["CloseRatioByVolume"] = r.CloseRatioByVolume;
//            node["CloseTodayRatioByMoney"] = r.CloseTodayRatioByMoney;
//            node["CloseTodayRatioByVolume"] = r.CloseTodayRatioByVolume;
//            node["ExchangeID"] = r.ExchangeID;
//            node["BizType"] = r.BizType;
//
//            value["commission_rate"] = node;
//        }

//    }

    return value;
}

void TdApi::disconnect(){

}

TdApi& TdApi::instance(){
    static TdApi api;
    return api;
}

void TdApi::onError(const std::string& event,const int errcode,const std::string & errmsg){
    Json::Value node;
    node["event"] = "event_error";
    node["timestamp"] =(Json::Value::UInt64) std::time(NULL);
    node["id"] = Application::instance()->getConfig().get_string("id");
    node["type"] = "ctp-trader";

    node["errcode"] = errcode;
    node["errmsg"] = errmsg;

    std::string json_text = node.toStyledString();
//    redis_->publish(pubchan_event_name_, json_text);
}

void TdApi::heartbeat(){
    Json::Value node;
    node["event"] = "event_heartbeat";
    node["timestamp"] =(Json::Value::UInt64) std::time(NULL);
    node["id"] = Application::instance()->getConfig().get_string("id");
    node["type"] = "ctp-trader";

    std::string json_text = node.toStyledString();



}
/*
 * vim ~/.gdbinit
 *
 * handle SIGUSR1 noprint nostop
 *
 *
 */

