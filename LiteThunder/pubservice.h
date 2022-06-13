//
// Created by scott on 2021/11/3.
//

#ifndef ELXTRADER_PUBSERVICE_H
#define ELXTRADER_PUBSERVICE_H

#include <sw/redis++/redis++.h>
using namespace sw::redis;

#include "base.h"
#include "config.h"




struct PubMessage{
    PubMessage(const std::string& topic);
    ByteArray toBytes() const;
    std::string toJson() const;

    std::string name;
    std::string topic;
    std::time_t timestamp;
    std::string level;
    std::string category;
    std::string target;
    std::string service_id;
    std::string service_type;
    std::string format; // json , plain
    std::string text ;
    std::string ip_pub;
    std::string ip;
//    static create()

};

/// 应用消息发布
class MessagePublisher {
public:
    bool init(const Config& cfg);
    bool open();
    void close();
    void publish(const PubMessage& msg);
    void publish(const std::string& topic,const std::string& msg);
    void publish(const std::string& topic,const char* bytes , size_t size);
    void publish_redis(const std::string& name,const std::string& key , const std::string& value);

    static MessagePublisher& instance(){
        static MessagePublisher publisher;
        return publisher;
    }

    Redis *  getRedis() { return redis_;};
protected:
//    void work_thread();
private:
    Config cfg_;
    void*   ctx_ = nullptr;
    void*   pub_ = nullptr;
    Redis * redis_ = nullptr;
};


#endif //ELXTRADER_PUBSERVICE_H
