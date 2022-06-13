
#include "Controller.h"

#include <chrono>
#include <cmath>
#include <memory>
#include <mutex>
#include <numeric>
#include <iterator>

#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <boost/algorithm/string.hpp>

#include "app.h"
#include "version.h"
#include "market.h"
#include "pubservice.h"
#include "mx.h"
#include "HttpService.h"
#include "RegistryClient.h"
#include "orderslice.h"


#include "utils/logger.h"
#include "mx.h"
#include "utils/os.h"
#include "tradetime.h"

class LogPubHandler:public Logger::Handler {
public:
    std::string format_;

    LogPubHandler(const std::string& fmt = "text"){
        format_ = fmt ;
    }

    void write(const std::string &log, Logger::Types type = Logger::INFO){
        std::string text = log;
        MessagePublisher::instance().publish(TopicLogText,text);
    }

    void flush(){

    }
};

bool Controller::init(const Config& props){
	boot_time_ = std::time(NULL);
	cfgs_ = props;
	timer_ = std::make_shared<boost::asio::steady_timer>(io_service_);
	timer_->expires_after(std::chrono::seconds( check_timer_interval_));
	timer_->async_wait(std::bind(&Controller::workTimedTask, this));
    settings_.tick_json = cfgs_.get_int("ctp.market.tick.json",1);
    settings_.tick_raw = cfgs_.get_int("ctp.market.tick.raw",1);
    settings_.tick_text = cfgs_.get_int("ctp.market.tick.text",1);

    trade_user_login_ = false;
    market_user_login_ = false;
    position_return_ = false;
    order_return_ = false ;
    return true;
}

bool Controller::open(){
    Application::instance()->getLogger().debug("Controller::open() ..");

    if(cfgs_.get_int("pub_service.enable",0)){
        MessagePublisher::instance().init(cfgs_);
        MessagePublisher::instance().open();
        Application::instance()->getLogger().addHandler(std::make_shared<LogPubHandler>());
//        this->msg_pub_ = &MessagePublisher::instance();
    }

    initRedis();
    // 加载交易合约配置参数
    initInstruments();
    initPosition();
    initPositionReceive();

    getMarketApi()->init(cfgs_);
    getMarketApi()->start();

    getTradeApi()->init(cfgs_);
    getTradeApi()->start();
    
    std::time_t start = std::time(NULL);

    int TIMETOUT = cfgs_.get_int("test.dataready.timeout",20);

    // 等待 market ，trade 用户登录成功
    while( trade_user_login_ == false || market_user_login_ == false || position_return_ == false){
        std::stringstream ss;
        if( start + TIMETOUT < std::time(NULL)){
            Application::instance()->getLogger().error("wait login : no response , break! ");
            return false;
        }
        
        ss << "wait login.." << " trade_user_login: "<< trade_user_login_ << " market_user_login: " << market_user_login_ ;
        Application::instance()->getLogger().debug(ss.str());

        if( trade_user_login_ == false || market_user_login_ == false){
            getTradeApi()->queryPosition();
        }
        std::this_thread::sleep_for(std::chrono::seconds(2)); 
    }
    Application::instance()->getLogger().info("ctp user login okay!");
    // 查询挂单
    std::this_thread::sleep_for(std::chrono::seconds(2)); 
    Application::instance()->getLogger().debug("queryOrder..");
    getTradeApi()->queryOrder();
    std::this_thread::sleep_for(std::chrono::seconds(2)); 
    this->discardOrderAll();
    std::this_thread::sleep_for(std::chrono::seconds(2)); 
    // 查询合约信息，直到返回
    std::lock_guard<std::recursive_mutex> lock(mtx_self);
    start = std::time(NULL);
    bool wait = true;    
    while(wait){
        for( auto kv : instruments_){        
            if( kv.second->detail ) {
                wait = false;
                break;                
            }            
        }
        if( wait == false) break;
        Application::instance()->getLogger().debug(">> queryInstrument..");
        getTradeApi()->queryInstrument("");
        std::this_thread::sleep_for(std::chrono::seconds(2)); 
    }


    // std::string instdata;
    // InstrumentRequirement requirement;
    // InstrumentId instrument;
    // while( !dataReady(requirement, instrument)  ) { 
    //     std::this_thread::sleep_for(std::chrono::seconds(1)); 
    //     if( requirement & ~InstrumentRequirement::Instrument){
    //         // 查询合约信息
    //         getTradeApi()->queryInstrument("");
    //     }
    //     requirement = InstrumentRequirement::None;
        
    // }
    // work_thread_ = std::thread(&Controller::keepWalkingThread, this)

    
	return true;
}

bool Controller::dataReady(InstrumentRequirement& require, InstrumentId& instrument){
    // 等待 合约相关信息查询 完成    
    std::lock_guard<std::recursive_mutex> lock(mtx_self);

    int count = 0;
    for( auto kv : instruments_){
        if( cfgs_.get_int("dataready.instrument",1) == 1 && 
                kv.second->detail == NULL ){
            require = InstrumentRequirement::Instrument;
            instrument = kv.first;
            return false;
        }
        if( cfgs_.get_int("dataready.commission",1) == 1 &&
                kv.second->commission == NULL ){
            require = InstrumentRequirement::Commission;
            instrument = kv.first;
            return false;
        }
        if( cfgs_.get_int("dataready.margin",1) == 1 &&
                kv.second->margin == NULL){
            require = InstrumentRequirement::Margin;
            instrument = kv.first;
            return false;    
        }
    }
    // return count == instruments_.size()?true:false;
    return true;
}

bool Controller::initPositionReceive(){
    // return true;
    pos_receiver_ = std::make_shared<ZMQ_PosReceiver>();
    pos_receiver_->init( Application::instance()->getConfig());
    pos_receiver_->setListener(this);
    return pos_receiver_->open();    
}

void Controller::onMarketUserLogin(){
//    SCOPED_LOCK_RECURSIVE(mtx_instruments_)
    Application::instance()->getLogger().debug("= OnMarketUserLogin()");
    std::vector<InstrumentId > names;
    for(auto itr : instruments_){
        names.push_back( itr.first);
        Application::instance()->getLogger().debug("= subscribeMarket() " + itr.first);
    }
    getMarketApi()->subscribeMarketData(names);
    market_user_login_ = true;

}

void Controller::onInstrumentQueryResult(const std::map< std::string , CThostFtdcInstrumentField > & instruments){
    Application::instance()->getLogger().debug("<< onInstrumentQueryResult()");
    for(auto itr : instruments){
        auto inst_itr = instruments_.find(itr.first);
        if( inst_itr != instruments_.end()){
            inst_itr->second->detail = &itr.second;
        }
    }
}

void Controller::onInstrumentCommissionRate(const std::vector< CThostFtdcInstrumentCommissionRateField > & commissions){
    Application::instance()->getLogger().debug("= onInstrumentCommissionRate()");
    for(auto  &itr : commissions){
        auto inst_itr = instruments_.find(itr.InstrumentID);
        if( inst_itr != instruments_.end()){
            inst_itr->second->commission = (CThostFtdcInstrumentCommissionRateField*)&itr;
        }
    }
}

void Controller::onInstrumentMarginRate(const std::vector< CThostFtdcInstrumentMarginRateField > & margins){
    for(auto  &itr : margins){
        auto inst_itr = instruments_.find(itr.InstrumentID);
        if( inst_itr != instruments_.end()){
            inst_itr->second->margin = (CThostFtdcInstrumentMarginRateField*)&itr;
        }
    }
}


bool Controller::initRedis(){
    ConnectionOptions connection_options;
    Config &cfg = Application::instance()->getConfig();
    connection_options.host = cfg.get_string("redis.host", "127.0.0.1");
    connection_options.port = cfg.get_int("redis.port", 6379);            // The default port is 6379.
    connection_options.db = cfg.get_int("redis.db", 0);
    connection_options.socket_timeout = std::chrono::milliseconds(200);
    // Connect to Redis server with a single connection.
    redis_ = new Redis(connection_options);
    return true;
}
   
bool Controller::initPosition(){
    // 加载最新的仓位 target ,   thunderid:{instrument:ps }
    for(auto itr : instruments_){
        std::string thunder_id = cfgs_.get_string("id","thunder");
        auto value = redis_->hget( thunder_id,itr.second->name );
        if( value.value() != "") {
            try {
                itr.second->expect = boost::lexical_cast<int>(value.value());
                std::cout<< "Load Position:" << itr.second->name <<  " " << itr.second->target << std::endl;
            }catch (std::exception& e){
                Application::instance()->getLogger().error( to_string(e));
            }
        }
    }
    return true;
}

bool Controller::initInstruments(){
    Json::Reader reader;
    Json::Value root;
    Message::Ptr msg;
    std::string fn = "instruments.json";
    std::ifstream file;
    //fn = osutil::getExePath() + "/" + fn;
    
    file.open(fn, std::ifstream::in);
    if (!file){
        Application::instance()->getLogger().error("Read instruments.json Failed!");
        return false;
    }
    std::string data( (std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());

    if (!reader.parse(data, root)) {
        Application::instance()->getLogger().error("Parse instruments.json Failed!");
        return false;
    }
//    td::ifstream is;
//    is.open (filename, std::ios::binary );
//    if (reader.parse(is, root))
    // ------------------------
    // 读取 交易时间
    auto trading_time = root.get("trading_time",Json::Value(Json::arrayValue));
    std::map< std::string , TradingTime > tradingtime_list;
    for(Json::ArrayIndex n=0; n< trading_time.size() ; n++){
        auto time = trading_time[n];
        TradingTime tt;
        tt.id = time.get("id","").asString();
        auto times = time.get("times",Json::Value(Json::arrayValue));
        int start,end;
        size_t size = times.size();
        for(Json::ArrayIndex m=0;m< times.size() ; m++){
            auto ts = times[m];
            start = ts[0].asInt();
            end = ts[1].asInt();
            std::tuple<int,int> tp = std::make_tuple(start,end);
            tt.times.push_back( tp);
        }
        tradingtime_list[tt.id] = tt;
    }
    // ------------------------
    // 读取商品和合约配置信息
    CommodityConfig::Ptr  comm;
    auto comms = root.get("commodities",Json::Value(Json::arrayValue));
    for(Json::ArrayIndex n=0 ; n< comms.size() ; n++){
       auto data = comms[n];
       comm = std::make_shared<CommodityConfig>();

       comm->name = data.get("name","").asString();
       comm->weight = data.get("weight",1.).asFloat();
       comm->exchange = data.get("exchange","").asString();
       auto name = data["trade_time"].asString();
       comm->trading_time = tradingtime_list[name];
       comm->instrument = data.get("instrument","").asString();
       comm->tick_price = data.get("tick_price",0).asFloat(); // 最小变动价格
       comm->tick_size = data.get("tick_size",0).asInt(); // 交易单位，乘数
       commodities_[comm->name ] = comm;
    }

    InstrumentConfig::Ptr inst ;
    auto instruments = root.get("instruments",Json::Value(Json::arrayValue));
    for(Json::ArrayIndex n=0 ; n< instruments.size() ; n++){
        auto data = instruments[n];
        inst = std::make_shared<InstrumentConfig>();
        auto enable = data.get("enable",false).asBool();
        if( enable == false){
            continue ;
        }
        inst->name = data.get("name","").asString();
        if( inst->name == ""){
            continue;
        }
        inst->timeout = data.get("timeout",30).asInt();
        // inst->price_type = data.get("price_type","market").asString();
        inst->ticks = data.get("ticks",0).asInt(); // 默认 0 ， 最新成交价
        inst->mode = data.get("mode","fee").asString(); // 默认 手续费模式 

        auto name = data.get("commodity","").asString();
        if( commodities_.find(name) == commodities_.end()){
            continue ;

        }
        if( commodities_.find(name) != commodities_.end()){
            auto comm = commodities_[name];
            inst->comm = comm.get();
            instruments_[inst->name] = inst ;
            Application::instance()->getLogger().debug("instrument loaded: " + inst->name);

        }
    }

    return true;
}

DirectionPosition  Controller::getDirectionPosition( const InstrumentId& name, const std::string& direction){
    DirectionPosition dpos;
    if( direction == "long"){
        if( inst_positions_long_.find(name) != inst_positions_long_.end()){
            dpos = inst_positions_long_[name];
        }
    }
    if( direction == "short"){
        if( inst_positions_short_.find(name) != inst_positions_short_.end()){
            dpos = inst_positions_short_[name];
        }
    }
    return dpos;
}

void Controller::keepWalkingThread(){
    running_ = true;
    std::list< std::string > queries;
    std::time_t     next_query_account = std::time(NULL) + cfgs_.get_int("ctp.trade.account.query_interval",2);
    
    query_batch_.start();

    while(running_){
        // std::map< InstrumentId, InstrumentConfig >::iterator itr ;

        // if( std::time(NULL) > next_query_account){
        //     getTradeApi()->queryAccount();
        //     next_query_account = std::time(NULL) + cfgs_.get_int("ctp.trade.account.query_interval",2);
        // }

        query_batch_.timeTick();

        std::stringstream ss;
        ///////// 交易时间控制 ////////////
        // std::tm now = osutil::now();
        // int tv = now.tm_hour * 100 + now.tm_min;
        // if( tv < 900 || ( tv >= 1015 && tv < 1030) || (tv >= 1130 && tv < 1330) ||
        //     ( tv >=1500 && tv < 2100 )
        // ){
        //     std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
        //     continue ;            
        // }
        ////////////////////////////////
        // 进入 for 循环，单步运行将终止 ， 奇怪？

        for( auto kv : instruments_){
            if( !isInTradingTime(&kv.second->comm->trading_time)){
               continue;
            }
            auto instcfg = kv.second; 
            if( instcfg->expect == MAX_POSITION_VALUE){
                continue;
            }

            if( 0 ){
                std::this_thread::sleep_for(std::chrono::milliseconds(100));     
                continue;
            }

            std::lock_guard<std::recursive_mutex> lock(instcfg->mtx);

            int long_pos = 0 ;
            int short_pos = 0 ;
            {
                std::lock_guard<std::recursive_mutex> lock(mtx_self);
                auto long_itr = inst_positions_long_.find(kv.first);
                if (long_itr != inst_positions_long_.end()) { // 无仓位
                    long_pos = long_itr->second.volume;
                }
                auto short_itr = inst_positions_short_.find(kv.first);
                if (short_itr != inst_positions_short_.end()) { // 无仓位
                    short_pos = short_itr->second.volume;
                }
            }

            int present = long_pos - short_pos;
            if( cfgs_.get_int("test.present_pos",0)){
                present = cfgs_.get_int("test.present_pos.value",0);
            }
            // 报单超时撤单
            // std::cout << "=1. slice size:" << instcfg->slices.size() << std::endl;
            if( instcfg->slices.size() ){
                if( (instcfg->start + instcfg->timeout) < std::time(NULL) ){
                    // 超时未能完成全部委托
                    for(auto & slice : instcfg->slices){
                        if( cfgs_.get_int("test.create_order",1)){
                            cancelOrder(slice);
                        }
                    }
                    instcfg->slices.clear() ;
                }
                continue;
            }
            instcfg->target = instcfg->expect;
            
            auto slices = createOrderSlice(instcfg->target,present);
            if( instcfg->mode == "mode"){
                int long_td , short_td;
                long_td = this->getDirectionPosition(instcfg->name , "long").getTd();
                short_td = this->getDirectionPosition( instcfg->name, "short").getTd();

                slices = createOrderSliceMargin(instcfg->target,present,long_td, short_td);
            }
            if( slices.size() == 0){
                continue ;
            }
            instcfg->slices = slices;
            instcfg->start = std::time(NULL);
            
            // std::cout << "=2. slice size:" << instcfg->slices.size() << std::endl;

            if( cfgs_.get_int("debug.slice_order_print",0 ) ){
                ss.str("");
                Application::instance()->getLogger().debug("-------------------------"); 
                ss  << "present:"<< present << " target:" << instcfg->target ;
                Application::instance()->getLogger().debug(ss.str()); 
                for ( auto slice : slices ){
                    ss.str("");
                    ss << instcfg->name << " " << slice->direction << " " << slice->openclose << " " << slice->quantity ;                     
                    Application::instance()->getLogger().debug(ss.str()); 
                }    
                Application::instance()->getLogger().debug("-------------------------"); 
            }

            // 报单
            for( auto &slice : instcfg->slices){
                slice->instcfg = instcfg.get();
                if( cfgs_.get_int("test.create_order",1)){
                   int error = createOrder(slice);                    

                }
            }
            // std::cout << "=3. slice size:" << instcfg->slices.size() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));        
        // std::this_thread::sleep_for(std::chrono::seconds(2));        
    }  // end while 
    Application::instance()->getLogger().info("keepwalking died.");
}

void Controller::walk(std::shared_ptr<InstrumentConfig> instrument){
    std::stringstream ss;   
    auto instcfg = instrument; 
    if( instcfg->expect == MAX_POSITION_VALUE){
        return;
    }

    if( 0 ){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));     
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(instcfg->mtx);

    int long_pos = 0 ;
    int short_pos = 0 ;
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_self);
        auto long_itr = inst_positions_long_.find( instrument->name);
        if (long_itr != inst_positions_long_.end()) { // 无仓位
            long_pos = long_itr->second.volume;
        }
        auto short_itr = inst_positions_short_.find( instrument->name);
        if (short_itr != inst_positions_short_.end()) { // 无仓位
            short_pos = short_itr->second.volume;
        }
    }

    int present = long_pos - short_pos;
    if( cfgs_.get_int("test.present_pos",0)){
        present = cfgs_.get_int("test.present_pos.value",0);
    }
    // 报单超时撤单
    // std::cout << "=1. slice size:" << instcfg->slices.size() << std::endl;
    if( instcfg->slices.size() ){
        if( (instcfg->start + instcfg->timeout) < std::time(NULL) ){
            // 超时未能完成全部委托
            for(auto & slice : instcfg->slices){
                if( cfgs_.get_int("test.create_order",1)){
                    cancelOrder(slice);
                }
            }
            instcfg->slices.clear() ;
        }
        return;
    }
    instcfg->target = instcfg->expect;
    
    auto slices = createOrderSlice(instcfg->target,present);
    if( instcfg->mode == "mode"){
        int long_td , short_td;
        long_td = this->getDirectionPosition(instcfg->name , "long").getTd();
        short_td = this->getDirectionPosition( instcfg->name, "short").getTd();

        slices = createOrderSliceMargin(instcfg->target,present,long_td, short_td);
    }
    if( slices.size() == 0){
        return ;
    }
    instcfg->slices = slices;
    instcfg->start = std::time(NULL);
    
    // std::cout << "=2. slice size:" << instcfg->slices.size() << std::endl;

    if( cfgs_.get_int("debug.slice_order_print",0 ) ){
        ss.str("");
        Application::instance()->getLogger().debug("-------------------------"); 
        ss  << "present:"<< present << " target:" << instcfg->target ;
        Application::instance()->getLogger().debug(ss.str()); 
        for ( auto slice : slices ){
            ss.str("");
            ss << instcfg->name << " " << slice->direction << " " << slice->openclose << " " << slice->quantity ;                     
            Application::instance()->getLogger().debug(ss.str()); 
        }    
        Application::instance()->getLogger().debug("-------------------------"); 
    }

    // 报单
    for( auto &slice : instcfg->slices){
        slice->instcfg = instcfg.get();
        if( cfgs_.get_int("test.create_order",1)){
            int error = createOrder(slice);                    

        }
    }
    // std::cout << "=3. slice size:" << instcfg->slices.size() << std::endl;
}
        

void Controller::close(){
    if (pos_receiver_) pos_receiver_->close();
    
    getTradeApi()->stop();
    getMarketApi()->stop();
}

void Controller::run(){
	// io_service_.run();
    Application::instance()->getLogger().debug("enter mainloop ..");
    keepWalkingThread();
}

void Controller::resetStatus(){
	std::lock_guard<std::recursive_mutex> lock(rmutex_);
	settings_.login_inited = false;
}

int Controller::createOrder(OrderSlice::Ptr slice){
    slice->start = std::time(NULL);

    OrderRequest req;
    req.symbol = slice->instcfg->name;
    req.cc = THOST_FTDC_CC_Immediately;
    req.tc = THOST_FTDC_TC_IOC;
    req.vc = THOST_FTDC_VC_CV;
    req.price = 0;
    req.volume = slice->quantity;
    req.direction = THOST_FTDC_D_Buy;
    if(slice->direction == "sell"){
        req.direction = THOST_FTDC_D_Sell;
    }
    req.offset = THOST_FTDC_OFEN_Open;
    req.price_type = THOST_FTDC_OPT_LimitPrice;
    
    // std::map<int,char> ops ={ {1,'1'},{2,'2'},{3,'3'},{4,'4'},{5,'5'},
    //     {6,'6'},{7,'7'},{8,'8'},{9,'9'},{10,'A'},{11,'B'},{12,'C'},{13,'E'},{14,'D'},{15,'F'}};

    if( slice->openclose == "close"){
        // req.offset = THOST_FTDC_OFEN_Close;
        req.offset = slice->closetype;
    }    
    if( slice->openclose == "forceclose"){
        req.offset = THOST_FTDC_OFEN_ForceClose;
    }
    if( slice->openclose == "closetoday"){
        req.offset = THOST_FTDC_OFEN_CloseToday;
    }
    if( slice->openclose == "closeyesterday"){
        req.offset = THOST_FTDC_OFEN_CloseYesterday;
    }

    req.exchange_id = slice->instcfg->comm->exchange;

    auto market= getMarketData(slice->instcfg->name);
    if(!market){
        std::stringstream ss;
        ss <<  "!! depathMarket is NULL . " << slice->instcfg->name ;
        Application::instance()->getLogger().error(ss.str());
        return -1;
    }

    float offset_price = slice->instcfg->comm->tick_price * slice->instcfg->comm->tick_price * slice->instcfg->ticks;

    if( req.direction == THOST_FTDC_D_Buy){
        // if( req.offset == THOST_FTDC_OFEN_Open){
        //     req.price = (market->LastPrice + slice->instcfg->comm->tick_price * slice->instcfg->ticks) * slice->instcfg->comm->tick_size;
        // }else{ // close
        //     req.price = market->LastPrice + offset_price;
        // }
        req.price = (market->LastPrice + slice->instcfg->comm->tick_price * slice->instcfg->ticks) ; //* slice->instcfg->comm->tick_size;
    }
    if( req.direction == THOST_FTDC_D_Sell){
        // if( req.offset == THOST_FTDC_OFEN_Open){
        //     req.price = market->LastPrice - offset_price;
        // }else{ // close
        //     req.price = market->LastPrice - offset_price;
        // }
        req.price = (market->LastPrice - slice->instcfg->comm->tick_price * slice->instcfg->ticks); // * slice->instcfg->comm->tick_size;
    }
    // req.price = round(req.price,2);

    { // print create order detail 
        std::stringstream ss;
        ss << ">> createOrder() "<<
        slice->instcfg->name << " limit price:" << 
        req.price << " direction:" << slice->direction << " openclose:" << slice->openclose
        << " num:" << slice->quantity << 
        " ticks:" << slice->instcfg->ticks <<  " ticksize:" << slice->instcfg->comm->tick_size
        << "tick-price:" << slice->instcfg->comm->tick_price << 
        " lastPrice:" << market->LastPrice 
        // << " priceTick:" << slice->instcfg->detail->PriceTick  
        ;
        Application::instance()->getLogger().info(ss.str() );

    }
    // return -1;
    this->appendOrderLog(slice->instcfg->name,req);
    slice->order_ret = this->getTradeApi()->sendOrder(req);
    return slice->order_ret.error;
}

void Controller::cancelOrder(OrderSlice::Ptr slice){
    CancelOrderRequest req;
    
    req.front_id = slice->order_ret.front_id; // boost::lexical_cast<int>(b);
    req.session_id = slice->order_ret.session_id; // boost::lexical_cast<int>(c);
    req.order_ref = slice->order_ret.order_ref;
    req.exchange = slice->order_ret.exchange;
    req.order_sys_id = slice->order_ret.order_sys_id;
    req.symbol = slice->order_ret.symbol;

    std::stringstream ss;
    ss << ">> -- cancelOrder() " << slice->instcfg->name 
        << " direction:" << slice->direction
        << " openclose:" << slice->openclose
        << " num:" << slice->quantity
        << " front:" << req.front_id << 
        " session:" << req.session_id << " order_ref:" << req.order_ref;
    Application::instance()->getLogger().info(ss.str());

    if( slice->order_ret.error){
        // 报单错误的返回值，忽略
        Application::instance()->getLogger().info("!! cancelOrder discard, error not zero!");
        return ;
    }
    this->appendOrderLog(slice->instcfg->name,req);
    this->getTradeApi()->cancelOrder(req);
}

// 定时工作任务
void Controller::workTimedTask(){
	std::lock_guard<std::recursive_mutex> lock(rmutex_);
	timer_->expires_after(std::chrono::seconds( check_timer_interval_));
	timer_->async_wait(std::bind(&Controller::workTimedTask, this));
}

//////////////////////////////////////////////////

// 委托回报 ,
void Controller::onOrderReturn(CThostFtdcOrderField *order) {
    CThostFtdcOrderField * input = order;
     std::stringstream ss;
    // ss << "<< onOrderReturn()  OrderRef:" <<input->OrderRef << " InstrumentID:"<< input->InstrumentID
    //     << " Direction" << input->Direction << " LimitPrice:" << input->LimitPrice <<
    //     " OrderRef:" << order->OrderRef << " Front:"<<order->FrontID << " Session:" << order->SessionID;
    // ss << " << onOrderReturn() ";
    PRINT_ORDER_INFO2(ss, "<< onOrderReturn() ",order);
    Application::instance()->getLogger().info(ss.str());


    for(auto &inst : instruments_){
        std::lock_guard<std::recursive_mutex> _(inst.second->mtx);
        auto &slices = inst.second->slices;
        for(auto  itr = slices.begin();itr!=slices.end();itr++){
            if( (*itr)->order_ret.order_ref == order->OrderRef){
                // if( order->OrderStatus == THOST_FTDC_OST_AllTraded ){
                    // slices.erase(itr);
                    // return ;  // 全部成交则取消
                // }
                (*itr)->order_ret.front_id = order->FrontID;
                (*itr)->order_ret.exchange = order->ExchangeID;
                (*itr)->order_ret.session_id = order->SessionID;
                (*itr)->order_ret.order_sys_id = order->OrderSysID;                
            }
        }
    }
}

// 仓位变动  ， alt+O , F12
void Controller::onPositionChanged(PositionSignal ps){
    if( commodities_.find(ps.commodity) == commodities_.end()){
        std::string msg = (boost::format("Commodity not Registerred:%s") % ps.commodity).str();
        Application::instance()->getLogger().error(msg);
    }
    InstrumentConfig::Ptr  instr;
    if(ps.instrument.size()){
        auto itr = instruments_.find(ps.instrument);
        if( itr!= instruments_.end()){
            instr = itr->second;
        }
    }
    if( !instr){
        instr = getInstrumentByCommodity(ps.commodity);
    }

    if(!instr){   //
        Application::instance()->getLogger().info( ps.to_string());
        return ;
    }
    std::lock_guard<std::recursive_mutex> _( instr->mtx);
    instr->expect = ps.pos;
}

// 根据品种返回交易的合约名
InstrumentConfig::Ptr Controller::getInstrumentByCommodity(CommodityName comm){
    auto itr = commodities_.find(comm);
    if( itr != commodities_.end()){
        auto inst_name = itr->second->instrument;
        auto itr_inst = instruments_.find(inst_name);
        if( itr_inst != instruments_.end()){
            return itr_inst->second;
        }
    }
    return InstrumentConfig::Ptr();
}

//void Controller::onOrderRespInsert(CThostFtdcInputOrderField *input, CThostFtdcRspInfoField *resp, int nRequestID, bool bIsLast){
//    if(behavior_) {
//        behavior_->onOrderInsert(input,resp);
//    }
//    if(msg_pub_){
//        std::string json_text = trade::marshall(input,resp);
//        msg_pub_->publish(TopicOrderError,json_text);
//    }
//}

void Controller::onOrderErrorReturn(CThostFtdcInputOrderField *input, CThostFtdcRspInfoField *resp) {
    std::stringstream ss;
    ss << "<< onOrderErrorReturn()  OrderRef:" <<input->OrderRef << " InstrumentID:"<< input->InstrumentID
        << " Direction" << input->Direction << " LimitPrice:" << input->LimitPrice;
    Application::instance()->getLogger().error(ss.str());


    for(auto &inst : instruments_){
        std::lock_guard<std::recursive_mutex> _(inst.second->mtx);
        auto &slices = inst.second->slices;
        for(auto  itr = slices.begin();itr!=slices.end();itr++){
            if( (*itr)->order_ret.order_ref == input->OrderRef){
                // slices.erase(itr);
                (*itr)->order_ret.error = -1; // 标示为错误，在撤单时忽略
                return ;  // 删除错误 order
            }
        }
    }
}

std::shared_ptr< CThostFtdcDepthMarketDataField>  Controller::getMarketData(const std::string & instrument) {
    std::lock_guard<std::recursive_mutex> _(mtx_market_data_);
    std::shared_ptr< CThostFtdcDepthMarketDataField> ret;

    auto itr = market_datas_.find(instrument);
    if( itr != market_datas_.end() ){
        ret = itr->second;
    }
    
    return ret;

}


//void Controller::onOrderCancelReturn(CThostFtdcInputOrderActionField *action, CThostFtdcRspInfoField *resp){
//    if(behavior_) {
//        behavior_->onOrderCancelReturn(action,resp);
//    }
//    if(msg_pub_){
//        std::string json_text = trade::marshall(action,resp);
//        msg_pub_->publish(TopicOrderCancel,json_text);
//    }
//}

void Controller::onOrderCancelErrorReturn(CThostFtdcOrderActionField *action, CThostFtdcRspInfoField *resp) {
//    if(behavior_) {
//        behavior_->onOrderCancelErrorReturn(action,resp);
//    }
//    if(msg_pub_){
//        // std::string json_text = trade::marshall( (const CThostFtdcOrderActionField*)action,resp);
//        // msg_pub_->publish(TopicOrderCancelError,json_text);
//    }
}

//成交回报
void Controller::onTradeReturn(CThostFtdcTradeField *data) {
    std::lock_guard<std::recursive_mutex> _(mtx_self);
    std::stringstream ss;
    for(auto &inst : instruments_){
        if( inst.second->name == data->InstrumentID){
            if ( data->Direction == THOST_FTDC_D_Buy ){
                if( data->OffsetFlag == THOST_FTDC_OF_Open ){
                    DirectionPosition dpos;
                    if( inst_positions_long_.find(data->InstrumentID) != inst_positions_long_.end()){
                        dpos = inst_positions_long_[data->InstrumentID];
                    }                
                    dpos.volume += data->Volume;
                    dpos.avg_price = data->Price;                
                    inst_positions_long_[data->InstrumentID] = dpos;
                }else{
                    DirectionPosition dpos;
                    if( inst_positions_short_.find(data->InstrumentID) != inst_positions_short_.end()){
                        dpos = inst_positions_short_[data->InstrumentID];
                    }                
                    dpos.avg_price = data->Price;  
                    dpos.volume -= data->Volume;              
                    inst_positions_short_[data->InstrumentID] = dpos;
                }

            }

            if ( data->Direction == THOST_FTDC_D_Sell ){
                if( data->OffsetFlag == THOST_FTDC_OF_Open ){
                    DirectionPosition dpos;
                    if( inst_positions_short_.find(data->InstrumentID) != inst_positions_short_.end()){
                        dpos = inst_positions_short_[data->InstrumentID];
                    }                
                    dpos.volume += data->Volume;
                    dpos.avg_price = data->Price;                
                    inst_positions_short_[data->InstrumentID] = dpos;
                }else{
                    DirectionPosition dpos;
                    if( inst_positions_long_.find(data->InstrumentID) != inst_positions_long_.end()){
                        dpos = inst_positions_long_[data->InstrumentID];
                    }                
                    dpos.avg_price = data->Price;  
                    dpos.volume -= data->Volume;              
                    inst_positions_long_[data->InstrumentID] = dpos;
                }
                
            }   
            //////////// debug pos //////////
            // {
            //     DirectionPosition dpos;
            //     if( inst_positions_short_.find(data->InstrumentID) != inst_positions_short_.end()){
            //         dpos = inst_positions_short_[data->InstrumentID];
            //         ss.str("");
            //         ss << "-- " << data->InstrumentID << " short_pos:" << dpos.volume;
            //         Application::instance()->getLogger().debug(ss.str());
            //     }
            // }   
            // {
            //     DirectionPosition dpos;
            //     if( inst_positions_long_.find(data->InstrumentID) != inst_positions_long_.end()){
            //         dpos = inst_positions_long_[data->InstrumentID];
            //         ss.str("");
            //         ss << "-- " << data->InstrumentID << " long_pos:" << dpos.volume;
            //         Application::instance()->getLogger().debug(ss.str());
            //     }
            // }               
             ////////////  end debug pos //////////
            

            // 每次成交回报，检查 目标仓位与当前仓位是否一致，一致则清除等待任务
            int long_pos ,short_pos , present;
            long_pos = inst_positions_long_[data->InstrumentID].volume;
            short_pos = inst_positions_short_[data->InstrumentID].volume;
            present = long_pos - short_pos;

            
            
            std::lock_guard<std::recursive_mutex> lock(inst.second->mtx);
            if( inst.second->target == present){
                inst.second->slices.clear();   
                ss.str("");
                ss << "<< onTradeReturn:: finished all, " << data->InstrumentID << " target=present: "<< present ;
                Application::instance()->getLogger().debug(ss.str()); 
            }

        } // end if - for 
        //  
        // std::lock_guard<std::recursive_mutex> _(inst.second->mtx);
        // auto &slices = inst.second->slices;
        // for(auto  itr = slices.begin();itr!=slices.end();itr++){
        //     if( (*itr)->order_ret.order_ref == data->OrderRef){
        //         slices.erase(itr);
        //         return ;  // 删除成交的order
        //     }
        // }
    } // end for 
    showPosition();

}


void Controller::showPosition(){
    std::lock_guard<std::recursive_mutex> _(mtx_self);
    std::stringstream ss;
    Application::instance()->getLogger().debug("----------show positons-------------");
    for(auto &inst : instruments_){
        int long_pos =0 , short_pos =0;
        {
            DirectionPosition dpos;
            
            if( inst_positions_short_.find(inst.first) != inst_positions_short_.end()){
                dpos = inst_positions_short_[inst.first];
                ss.str("");
                ss << "-- " << inst.first << " short_pos:" << dpos.volume << " td:" << dpos.getTd() << " yd:"<<dpos.getYd();
                short_pos = dpos.volume;
                Application::instance()->getLogger().debug(ss.str());
            }
        }   
        {
            DirectionPosition dpos;
            if( inst_positions_long_.find(inst.first) != inst_positions_long_.end()){
                dpos = inst_positions_long_[inst.first];
                ss.str("");
                ss << "-- " << inst.first << " long_pos:" << dpos.volume << " td:" << dpos.getTd() << " yd:"<<dpos.getYd();
                long_pos = dpos.volume;
                Application::instance()->getLogger().debug(ss.str());
            }
        }  
        ss.str("");
        ss << "-- " << inst.first << " present_pos:" << long_pos - short_pos ;
        Application::instance()->getLogger().debug(ss.str());
    }   
    Application::instance()->getLogger().debug("----------^^^^^^^^^^^^-------------");   
}

void Controller::onTradeUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
    Application::instance()->getLogger().debug("<< onTradeUserLogin()");
    trade_user_login_ = true;
    Application::instance()->getLogger().debug(">> queryPosition()");
    getTradeApi()->queryPosition();
}


// Tick 行情记录
void Controller::onRtnDepthMarketData( CThostFtdcDepthMarketDataField *pDepthMarketData){
    
    if(!pDepthMarketData){
        return;
    }
    std::lock_guard<std::recursive_mutex> _(mtx_market_data_);

    std::string  name = pDepthMarketData->InstrumentID ;
    CThostFtdcDepthMarketDataField * pdata = new CThostFtdcDepthMarketDataField;
    *pdata = *pDepthMarketData;
    std::shared_ptr< CThostFtdcDepthMarketDataField > data (pdata) ; // std::make_shared< CThostFtdcDepthMarketDataField > (pdata);
    market_datas_[name] = data;
}

void Controller::onTradeQueryResult(const std::vector< CThostFtdcTradeField >& trades ){
//    if(msg_pub_){
//        std::string json_text = trade::marshall(trades);
//        msg_pub_->publish(TopicTradeQueryResult,json_text);
//    }
    query_batch_.resetTime();
}

void Controller::onPositionQueryResult(const std::vector<  CThostFtdcInvestorPositionField > & positons){
    std::lock_guard<std::recursive_mutex> lock(mtx_self);
    // 当日平掉的仓位也会返回, Position = 0 
    // 
    std::stringstream ss;
    ss <<  "<< onPositionQueryResult()";
    Application::instance()->getLogger().debug(ss.str());
    for(auto pos :positons ){
        DirectionPosition dpos;
        std::shared_ptr< CThostFtdcInvestorPositionField > _pos = std::make_shared<CThostFtdcInvestorPositionField>();
        *_pos = pos ;
        dpos.underlying = _pos;
        dpos.direction = pos.PosiDirection;
        dpos.volume = pos.Position;               

        if( pos.PosiDirection == THOST_FTDC_PD_Long){
            inst_positions_long_[pos.InstrumentID] = dpos;
        }
        if( pos.PosiDirection == THOST_FTDC_PD_Short){
            inst_positions_short_[pos.InstrumentID] = dpos;
        }
    }
    position_return_ = true;
    showPosition();
    query_batch_.resetTime();
}

void Controller::onAccountQueryResult( CThostFtdcTradingAccountField * account){
    std::lock_guard<std::recursive_mutex> _(mtx_self);
    account_info_ = *account;
    query_batch_.resetTime();
}

void Controller::onOrderQueryResult(const std::vector<CThostFtdcOrderField> & orders){
    // 查询当前挂单返回
    std::stringstream ss;
    ss <<  "<< onOrderQueryResult()";
    Application::instance()->getLogger().debug(ss.str());
   std::lock_guard<std::recursive_mutex> _(mtx_self);
    orders_ = orders;
    query_batch_.resetTime();
}


void Controller::discardOrderAll(){
    Application::instance()->getLogger().debug(">> discardOrderAll()..");
    std::lock_guard<std::recursive_mutex> _(mtx_self);
    for(auto &order : orders_){
        if(order.OrderStatus !=THOST_FTDC_OST_AllTraded && order.OrderStatus != THOST_FTDC_OST_Canceled){
            CancelOrderRequest req;
            req.front_id = order.FrontID; // boost::lexical_cast<int>(b);
            req.session_id = order.SessionID; // boost::lexical_cast<int>(c);
            req.order_ref = order.OrderRef;
            req.symbol = order.InstrumentID; // 确实加上这个 可以撤单，不需要第二种

            // req.exchange = order.ExchangeID;
            // req.order_sys_id = order.OrderSysID;
                        
            this->getTradeApi()->cancelOrder(req);
        }
    }
}

 void Controller::appendOrderLog(const InstrumentId& instid, OrderRequest & req){
     if( order_logs_.order_requests.find(instid) == order_logs_.order_requests.end() ){
         order_logs_.order_requests[instid] = std::vector<OrderRequest>(); 
     }
     req.time = std::time(NULL);
     order_logs_.order_requests[instid].push_back(req);
 }

 void Controller::appendOrderLog(const InstrumentId& instid, CancelOrderRequest& req ){
     if( order_logs_.order_cancels.find(instid) == order_logs_.order_cancels.end() ){
         order_logs_.order_cancels[instid] = std::vector<CancelOrderRequest>(); 
     }
     req.time = std::time(NULL);
     order_logs_.order_cancels[instid].push_back(req);
 }