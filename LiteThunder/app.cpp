//
// Created by bin zhang on 2019/1/6.
//

#include "app.h"
#include "Controller.h"

std::shared_ptr<Application>& Application::instance(){
	static std::shared_ptr<Application> handle ;
	if(!handle.get()){
		handle = std::make_shared<Application>();
	}
	return handle;
}

Application&  Application::init(int argc , char ** argvs){
    if( argc < 2){
        printf("\n"
               "Error: 'settings file ' should be specified! \n");
        return *this;
    }
    char * settings_file = argvs[1];
	cfgs_.load(settings_file);
	// 更新用户自定义的参数
	//Config user;
	//user.load(SETTINGS_USER_FILE);
	//cfgs_.update(user);
	logger_.setLevel(Logger::DEBUG);
	logger_.addHandler( std::make_shared<LogStdoutHandler>());
	logger_.addHandler( std::make_shared<LogFileHandler>("litethunder.log"));
	//开启调试输出
	if( cfgs_.get_string("debug.log.udp.enable") == "true") {
		std::string host = cfgs_.get_string("debug.log.udp.host", "127.0.0.1");
		uint16_t port = cfgs_.get_int("debug.log.udp.port", 9906);
		logger_.addHandler(std::make_shared<LogUdpHandler>(host, port));
	}
    Controller::instance()->init(cfgs_);
    
	return *this;
}

Logger& Application::getLogger(){
	return logger_;
}

void Application::run(){
	if( !Controller::instance()->open()){
		Controller::instance()->close();
		return ;
	}
	Controller::instance()->run();
	// wait_for_shutdown();
	getLogger().info("application terminated .");
}

void Application::stop(){
	cv_.notify_one();
}

Config& Application::getConfig(){
	return cfgs_;
}

void Application::wait_for_shutdown(){
	getLogger().info(name() + " started..");
	std::unique_lock<std::mutex> lk(mutex_);
	cv_.wait(lk);
}

std::string Application::name(){
	return cfgs_.get_string("application.name","Application");
}


