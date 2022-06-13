//
// Created by scott on 2021/11/2.
//

#include <sstream>
#include <thread>
#include <zmq.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "PosReceiver.h"
#include "datastruct.h"
#include "app.h"

bool    ZMQ_PosReceiver::init(const Config& cfg) {
    PosReceiver::init(cfg);
    return true;
}


bool    ZMQ_PosReceiver::open(){
    ctx_ = zmq_ctx_new ();
    void *subscriber = zmq_socket (ctx_, ZMQ_SUB); 
    if (subscriber == NULL){
        return false;
    }
    int ret = zmq_connect (subscriber, cfg_.get_string("position_receiver.pub_addr","tcp://127.0.0.1:9901").c_str());
    if (ret < 0){
        zmq_ctx_destroy(subscriber);
        return false;
    }
    std::string topic = cfg_.get_string("id");
    zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, topic.c_str(), topic.size());

    // int iRcvTimeout = cfg_.get_int("zmq.pos.recv_timeout", 1000); // millsecond
    // if(zmq_setsockopt(subscriber, ZMQ_RCVTIMEO, &iRcvTimeout, sizeof(iRcvTimeout)) < 0){
    //     zmq_close(subscriber);
    //     zmq_ctx_destroy(subscriber);
    //     return false;
    // }
    subscriber_ = subscriber;
    thread_ = std::thread( &ZMQ_PosReceiver::recv_thread , this);
    
    return true ;
}

ZMQ_PosReceiver::~ZMQ_PosReceiver(){
    close();
    join();
}

void ZMQ_PosReceiver::join(){
    thread_.join();
}

void    ZMQ_PosReceiver::close(){
    running_ = false;
    if( subscriber_ ){
        zmq_close(subscriber_);
        zmq_ctx_shutdown(ctx_);
    }
    subscriber_ = NULL;
}

void ZMQ_PosReceiver::recv_thread(){
    running_ = true;
    while(running_){
        char szBuf[1024] = {0};
        int ret = zmq_recv(subscriber_, szBuf, sizeof(szBuf) - 1, 0);
        if (ret > 0){
            std::vector<std::string> ss;
            //  account,commodity,instrument,pos  
            // "scott,AP,,10" , "scott,,AP210,10"
            std::stringstream s;
            if( cfg_.get_int("debug.zmq_pos_recv_print",0) == 1){
                s << "zmq::PS : " << szBuf ;
                Application::instance()->getLogger().debug( s.str());
            }
            try{
                boost::split(ss,szBuf,boost::is_any_of(","));
                if( ss.size() >=4 ){                                    
                    PositionSignal ps ; 
                    ps.commodity = ss[1];
                    ps.instrument = ss[2];
                    std::string text = ss[3];
                    ps.pos = boost::lexical_cast<int>(text) ;
                    if( listener_){
                        listener_->onPositionChanged(ps);
                    }
                }
            }catch(std::exception& e){
                Application::instance()->getLogger().error("PosReceiver data error:" + std::string(e.what()) );
                continue ;
            }
        }else{
            // std::this_thread::sleep_for(std::chrono::seconds(1));
            // EAGAIN 返回读超时
            // if( EAGAIN == zmq_errno()){
            //     continue ;
            // }
            std::stringstream ss;
            ss << "PositionRecv socket error :" << zmq_errno() ;
            Application::instance()->getLogger().error( ss.str() );
            break;
        }
    }
    Application::instance()->getLogger().info( " (zmx) Posiont Recv Thread Exit." );
}
