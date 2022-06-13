
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <jsoncpp/json/json.h>
#include "market.h"
#include "app.h"
#include "Controller.h"

void assign(char *dest, size_t dest_size, const char *src);

#define ASSIGN(dest, src) assign(dest,sizeof(dest),src)

void MdApi::OnFrontConnected() {
    onFrontConnected();
}

void MdApi::OnFrontDisconnected(int nReason) {
    onFrontDisconnected(nReason);
};

void MdApi::OnHeartBeatWarning(int nTimeLapse) {
    onHeartBeatWarning(nTimeLapse);
};

void MdApi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                           bool bIsLast) {
    onRspUserLogin(pRspUserLogin,pRspInfo,nRequestID,bIsLast);
};

void MdApi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                            bool bIsLast) {
    onRspUserLogout(pUserLogout,pRspInfo,nRequestID,bIsLast);
};

void MdApi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::string text = (boost::format("MdApi:: onRspError = error-id:%d , error-msg:%s") % pRspInfo->ErrorID %
                        pRspInfo->ErrorMsg).str();
    Application::instance()->getLogger().error(text);
};

void MdApi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                               int nRequestID, bool bIsLast) {
    this->onRspSubMarketData(pSpecificInstrument, pRspInfo, nRequestID, bIsLast);
};

void
MdApi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                            int nRequestID, bool bIsLast) {
    //onRspUnSubMarketData(pSpecificInstrument, pRspInfo, nRequestID, bIsLast);
};

void
MdApi::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                           int nRequestID, bool bIsLast) {
};

void
MdApi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                             int nRequestID, bool bIsLast) {

};

void MdApi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    if (pDepthMarketData) {
        Controller::instance()->onRtnDepthMarketData(pDepthMarketData);
    }
};

void MdApi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) {

};

//"%Y-%m-%d %H:%M:%S.%f"
bool ptime_from_string(boost::posix_time::ptime &pt, std::string datetime, std::string format) {
    std::stringstream ss(datetime);
    //std::locale responsible for releasing memory.
    ss.imbue(std::locale(ss.getloc(), new boost::posix_time::time_input_facet(format)));//out
    if (ss >> pt) {
        return true;
    }
    return false;
}

void MdApi::createFtdcMdApi(string pszFlowPath) {
    this->api = CThostFtdcMdApi::CreateFtdcMdApi(pszFlowPath.c_str());
    this->api->RegisterSpi(this);
};

void MdApi::release() {
    this->api->Release();
}

void MdApi::init(const Config& cfg) {
    //加载定义的合约名称列表
    std::string text = cfg.get_string("ctp.sub_instruments");
    std::vector<std::string > ss ;
    boost::split(ss, text, boost::is_any_of(", "));
    auto first = std::remove_if(ss.begin(),ss.end(),
                                []( std::string& s){
                                    boost::trim(s);
                                    return s.size() == 0;
                                });
    ss.erase(first,ss.end());
    instruments_ = ss;
};

int MdApi::join() {
    int i = this->api->Join();
    return i;
};

int MdApi::exit() {
    //该函数在原生API里没有，用于安全退出API用，原生的join似乎不太稳定
    this->api->RegisterSpi(NULL);
    this->api->Release();
    this->api = NULL;
    return 1;
};

string MdApi::getTradingDay() {
    string day = this->api->GetTradingDay();
    return day;
};

void MdApi::registerFront(string pszFrontAddress) {
    this->api->RegisterFront((char *) pszFrontAddress.c_str());
};

int MdApi::subscribeMarketData(string instrumentID) {
    char *buffer = (char *) instrumentID.c_str();
    char *myreq[1] = {buffer};
    int i = this->api->SubscribeMarketData(myreq, 1);
    return i;
};

int MdApi::unSubscribeMarketData(string instrumentID) {
    char *buffer = (char *) instrumentID.c_str();
    char *myreq[1] = {buffer};;
    int i = this->api->UnSubscribeMarketData(myreq, 1);
    return i;
};

//批量订阅合约行情
int MdApi::subscribeMarketData(const std::vector<std::string> & instruments){
    vector<char*> data(instruments.size());
    size_t n = 0;
    for(auto& instrument : instruments){
        data[n] = (char*) instrument.c_str();
        n++;
    }
    int i = this->api->SubscribeMarketData(data.data(),(int)data.size());

    for (auto &ins : instruments) {
        if( std::end(instruments_) == std::find(instruments_.begin(), instruments_.end(),ins)){
            instruments_.emplace_back(ins);
        }
    }
    instruments_ = instruments;

    return i;
}

int MdApi::unsubscribeMarketData(const std::vector<std::string> & instruments){
    return 0;
}


int MdApi::subscribeForQuoteRsp(string instrumentID) {
    char *buffer = (char *) instrumentID.c_str();
    char *myreq[1] = {buffer};
    int i = this->api->SubscribeForQuoteRsp(myreq, 1);
    return i;
};

int MdApi::unSubscribeForQuoteRsp(string instrumentID) {
    char *buffer = (char *) instrumentID.c_str();
    char *myreq[1] = {buffer};;
    int i = this->api->UnSubscribeForQuoteRsp(myreq, 1);
    return i;
};

int MdApi::reqUserLogin(CThostFtdcReqUserLoginField *req, int nRequestID) {
    int i = this->api->ReqUserLogin(req, nRequestID);
    return i;
};

int MdApi::reqUserLogout(CThostFtdcUserLogoutField *req, int nRequestID) {
//	CThostFtdcUserLogoutField myreq = CThostFtdcUserLogoutField();
//	memset(&myreq, 0, sizeof(myreq));
//	getStr(req, "UserID", myreq.UserID);
//	getStr(req, "BrokerID", myreq.BrokerID);
    int i = this->api->ReqUserLogout(req, nRequestID);
    return i;
};


bool MdApi::start() {
    Config &cfg = Application::instance()->getConfig();
    connect();
    return true;
}

void MdApi::stop() {
    this->exit();

}

bool MdApi::connect() {
    Config &cfg = Application::instance()->getConfig();
    std::string conpath = cfg.get_string("ctp.con_path", "cons");
    std::string address = cfg.get_string("ctp.md_addr");
    createFtdcMdApi(conpath);
    registerFront(address);
    this->api->Init();
    return true;
}

void MdApi::disconnect() {
    // 。。。
}

void MdApi::onFrontConnected() {
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    std::string userid, password, brokerid;
    Config &cfg = Application::instance()->getConfig();
    userid = cfg.get_string("ctp.user_id");
    password = cfg.get_string("ctp.password");
    brokerid = cfg.get_string("ctp.broker_id");

    assign(req.BrokerID, sizeof(req.BrokerID), brokerid.c_str());
    reqUserLogin(&req, 0);//
}

void MdApi::onFrontDisconnected(int i) {
    Application::instance()->getLogger().info("md::front disconnected..");
}

void MdApi::onHeartBeatWarning(int i) {
    Application::instance()->getLogger().debug("md::front heartbeat..");
}

// 登陆成功订阅合约
void MdApi::onRspUserLogin(CThostFtdcRspUserLoginField *data, CThostFtdcRspInfoField *error, int id, bool last) {
    Application::instance()->getLogger().info("onRspUserLogin : To subscribe Market Data");
//    boost::mutex::scoped_lock lock(mtx_instruments_);
//    this->subscribeMarketData( instruments_ );
    Controller::instance()->onMarketUserLogin();
//    for (auto _ : instruments_) {
//        Application::instance()->getLogger().debug("subscribe: " + _);
//        subscribeMarketData(_);
//    }
}

void MdApi::onRspUserLogout(CThostFtdcUserLogoutField *data,
                            CThostFtdcRspInfoField *error, int id, bool last) {
    Application::instance()->getLogger().info("onRspUserLogout : ");
}

//void MdApi::onRspError(CThostFtdcRspInfoField* error, int id, bool last) {
//	std::string text = (boost::format("onRspError = error-id:%d , error-msg:%s")%error->ErrorID%error->ErrorMsg).str();
//	Application::instance()->getLogger().error(text);
//}

void
MdApi::onRspSubMarketData(CThostFtdcSpecificInstrumentField *data, CThostFtdcRspInfoField *error, int id, bool last) {
    std::string text = (boost::format("<< onRspSubMarketData =  code:%s  error:%d , error-msg:%s") %
                        data->InstrumentID % error->ErrorID % error->ErrorMsg).str();
    Application::instance()->getLogger().error(text);
}



//void MdApi::scheduleQuery() {
//    int interval;
//    Config &cfg = Application::instance()->getConfig();
//    interval = cfg.get_int("query_interval", 1);
//    if (!timer_) {
//        timer_ = std::make_shared<boost::asio::deadline_timer>(Controller::instance()->io_service(),
//                                                               boost::posix_time::seconds(interval));
//    }
//    timer_->async_wait(std::bind(&MdApi::work_timer, this));
//
//}
//
//void MdApi::work_timer() {
//    Config &cfg = Application::instance()->getConfig();
//    int query_interval = cfg.get_int("query_interval", 1);
////	std::lock_guard<std::mutex> lock(mutex_query_);
//    timer_->expires_from_now(boost::posix_time::seconds(query_interval));
//    heartbeat();
//
//    scheduleQuery();
//}

//void MdApi::heartbeat() {
//    Json::Value node;
////	puts("heartbeat()..");
//    node["event"] = "event_heartbeat";
//    node["timestamp"] = (Json::Value::UInt64) std::time(NULL);
//    node["id"] = Application::instance()->getConfig().get_string("id");
//    node["type"] = "ctp-market";
//
//    std::string pubchan_event_name;
//    pubchan_event_name = Application::instance()->getConfig().get_string("ctp.pubchan_event_name");
//    std::string json_text = node.toStyledString();
//    redis_->publish(pubchan_event_name, json_text);
//}

//void MdApi::subscribeMarketData(const std::vector<std::string> & instruments){
//    for (auto &ins : instruments) {
//        if( std::end(instruments_) == std::find(instruments_.begin(), instruments_.end(),ins)){
//            instruments_.emplace_back(ins);
//            this->subscribeMarketData(ins);
//        }
//    }
//    instruments_ = instruments;
//}

MdApi& MdApi::instance(){
    static MdApi api;
    return api;
}

//MdApi service;
//static std::thread *joinThread = NULL;
//
//int main_loop() {
//
//    service.start();
////	service.join();
//    joinThread = new std::thread([] {
//        Application::instance()->getLogger().debug("CTP Api Thread Join..");
//        service.join();
//    });
//
//    return 0;
//}