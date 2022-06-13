//
// Created by scott on 2021/11/3.
//

#include "RegistryClient.h"
#include "utils/HttpClient.h"

bool RegistryClient::init(const Config& cfg){
    return true;

}

bool RegistryClient::open(){
    thread_ = std::thread( &RegistryClient::work_thread , this);
    pull();
    return true;
}

void RegistryClient::close(){

}


void RegistryClient::pull(){
    std::string url = cfg_.get_string("registry.pull_url");

    //HttpClient::post()
}

void RegistryClient::work_thread(){
    running_ = true;
    int interval = cfg_.get_int("registry_client.interval",1);

    while(running_) {
        std::this_thread::sleep_for(std::chrono::seconds(interval));
        pull();
    }
}