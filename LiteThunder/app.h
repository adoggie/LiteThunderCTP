//
// Created by bin zhang on 2019/1/6.
//

#ifndef INNERPROC_APP_H
#define INNERPROC_APP_H

#include "base.h"
//#include "service.h"
#include "config.h"
#include "utils/logger.h"
#include <boost/asio.hpp>

class Application;



class Application:public Object{
	Logger 	logger_;
	Config  cfgs_;
//	std::vector<std::shared_ptr<Service> > services_;
public:
	static std::shared_ptr<Application>& instance();
	Application&  init(int argc , char ** argvs);
	Logger&     getLogger();
	void        run();
	void        stop();
//	void        addService(std::shared_ptr<Service>& service );
	Config&     getConfig();
	std::string     name();
protected:
	void wait_for_shutdown();

};

#endif
