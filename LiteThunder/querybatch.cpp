

#include "querybatch.h"
#include "Controller.h"



void QueryBatchTimed::timeTick(){
    if( !started_ ){
        return;
    }

    std::time_t now = std::time(NULL);

    if( now < (latest_ + timeout_)){
        return ;
    }
    latest_ = now;

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
    latest_ = 0 ;
}


void QueryBatchTimed:: start(){
    started_ = true ;
    query_list_.push_back(QueryTask::QUERY_POSITION);
    query_list_.push_back(QueryTask::QUERY_ACCOUNT);
}