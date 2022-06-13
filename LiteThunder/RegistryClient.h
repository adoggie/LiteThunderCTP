//
// Created by scott on 2021/11/3.
// registyclient.h
// 连接注册服务获取交易合约相关信息

#ifndef ELXTRADER_REGISTRYCLIENT_H
#define ELXTRADER_REGISTRYCLIENT_H

#include <atomic>
#include "base.h"
#include "config.h"

class RegistryClient {
public:
    bool init(const Config& cfg);
    bool open();
    void close();

    static RegistryClient& instance(){
        static RegistryClient client;
        return client;
    }
private:
    void work_thread();
    void pull();
private:
    std::atomic_bool    running_;
    std::thread         thread_;
    Config              cfg_;
};


#endif //ELXTRADER_REGISTRYCLIENT_H
