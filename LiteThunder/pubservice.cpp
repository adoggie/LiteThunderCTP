//
// Created by scott on 2021/11/3.
//

#include "pubservice.h"
#include <zmq.h>
#include <jsoncpp/json/json.h>
#include "app.h"
#include "utils/logger.h"
#include "mx.h"
//
//class LogPubHandler:public Logger::Handler {
//public:
//    std::string format_;
//
//    LogPubHandler(const std::string& fmt = "text"){
//        format_ = fmt ;
//    }
//
//    void write(const std::string &log, Logger::Types type = Logger::INFO){
//        std::string text = log;
//        MessagePublisher::instance().publish(TopicLogText,text);
//    }
//
//    void flush(){
//
//    }
//};

////////////////////////////////////////////////////

PubMessage::PubMessage(const std::string& topic){
    this->topic = topic;
}

ByteArray PubMessage::toBytes() const{
    ByteArray  bytes;
    return bytes;
}

std::string PubMessage::toJson() const{
    Json::Value data;
    data["topic"] = topic;
    data["timestamp"] =(Json::Value::UInt64) std::time(NULL);
    data["level"] = level;
    data["category"] = category;

    return data.toStyledString();
}

///////////// MessagePublisher ///////////////////////////

bool MessagePublisher::init(const Config& cfg){
    cfg_ = cfg;
    return true;
}

bool MessagePublisher::open(){
    Application::instance()->getLogger().info("MessagePublisher open()..");

    auto & logger = Application::instance()->getLogger();
    //logger.addHandler( std::make_shared<LogPubHandler>());

//    if ( cfg_.get_int("pub_service.enable") == 1) {
        ctx_ = zmq_ctx_new();
        pub_ = zmq_socket(ctx_, ZMQ_PUB);
        std::string addr = cfg_.get_string("pub_service.addr", "tcp://127.0.0.1:9011");
        
        /// add .mode (bind/connect) 2022/6/6
        int r = 0 ;
        if ( cfg_.get_string("pub_service.mode","bind") == "bind"){
            Application::instance()->getLogger().info("pub bind:" + addr);
            r = zmq_bind(pub_, addr.c_str());
        }else{
            Application::instance()->getLogger().info("pub connect:" + addr);
            r = zmq_connect( pub_,addr.c_str());
        }

        if (r) {
//        auto fmt = boost::format(" message publish open failed! addr: %1%") % addr;
            std::string text = (boost::format(" message publish open failed! addr: %1%") % addr).str();
            Application::instance()->getLogger().error(text);
            return false;
        }
//    }

//    if ( cfg_.get_int("redis.enable") == 1) {
//        ///// init redis ////
//        ConnectionOptions connection_options;
//        Config &cfg = Application::instance()->getConfig();
//
//        connection_options.host = cfg.get_string("redis.host", "127.0.0.1");
//        connection_options.port = cfg.get_int("redis.port", 6379);            // The default port is 6379.
//        connection_options.db = cfg.get_int("redis.db", 0);
//        connection_options.socket_timeout = std::chrono::milliseconds(200);
//
//        // Connect to Redis server with a single connection.
//        redis_ = new Redis(connection_options);
//    }

    return true;
}

void MessagePublisher::close(){
    //
}

//void MessagePublisher::publish(const PubMessage& msg){
//    ByteArray bytes = msg.toBytes();
//   // zmq_send(pub_,(void*)bytes.data(),bytes.size(),ZMQ_SNDMORE);
//   if( cfg_.get_int("pub_service.enable")) {
//       zmq_send(pub_, (void *) bytes.data(), bytes.size(), 0);
//   }
//}

void MessagePublisher::publish(const std::string& topic,const std::string& msg){
    this->publish(topic,msg.c_str(),msg.size());
}

void MessagePublisher::publish(const std::string& topic,const char* bytes_ , size_t size){
    // ByteArray bytes;
    // bytes.resize( TOPIC_MAX_LEN + size );
    // size_t topic_size = topic.size() ;
    // if( topic_size > TOPIC_MAX_LEN){
    //     topic_size = TOPIC_MAX_LEN;
    // }
    // ::strncpy(bytes.data(),topic.c_str(),topic_size);
    // ::memcpy(bytes.data() + TOPIC_MAX_LEN, bytes_,size );

//    if( cfg_.get_int("pub_service.enable")) {
        std::string value(bytes_, size);
        std::stringstream  ss;
        ss << cfg_.get_string("id","ctptick") << ","  <<  value;
        // int r = zmq_send(pub_, (void *) bytes.data(), bytes.size(), 0);
        value = ss.str();
        int r = zmq_send(pub_, (void *) value.c_str(), value.size(), 0);
//    }
}
//
//void MessagePublisher::publish_redis(const std::string& name,const std::string& key , const std::string& value){
//    if( !cfg_.get_int("redis.enable") ){
//        return ;
//    }
//
//    std::map<std::string, std::string> entries;
//    entries[key] = value ;
//    std::stringstream  ss;
//    ss << cfg_.get_string("id","ctptick") << ","  <<  value;
//    //redis_->hmset(name, entries.begin(), entries.end());
//    redis_->rpush("l_"+name,ss.str());
//}

static MessagePublisher& instance(){
    static MessagePublisher publisher;
    return publisher;
}
