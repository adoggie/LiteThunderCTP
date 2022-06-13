

#include "querybatch.h"
#include "Controller.h"



void QueryBatchTimed::timeTick(){
    static std::time_t last = 0;
    if( !started_ ){
        return;
    }

    std::time_t now = std::time(NULL);
    if( now - last <= 1){
        return ;
    }
    last = now;
    if( now < (latest_ + timeout_ + delta_)){
        return ;
    }
    latest_ = now;
    delta_ = 0;
    if( query_list_.size() == 0){
        return ;
    }
    auto task = query_list_.front();
    query_list_.pop_front();
    query_list_.push_back(task);
    if( task == QueryTask::QUERY_POSITION){
        Controller::instance()->getTradeApi()->queryPosition();
    }else if( task == QueryTask::QUERY_ACCOUNT){
        Controller::instance()->getTradeApi()->queryAccount();
    }
    
}

void QueryBatchTimed:: resetTime(){
    // 过快的返回，要停滞一下
    latest_ = 0;
    std::time_t now = std::time(NULL);
    int  elapsed = now - latest_;
    delta_ = 0;
    // if(elapsed <=1){
    //     delta_ = 1;
    // }    
}


void QueryBatchTimed:: start(){
    started_ = true ;
    query_list_.push_back(QueryTask::QUERY_POSITION);
    // query_list_.push_back(QueryTask::QUERY_ACCOUNT);
}