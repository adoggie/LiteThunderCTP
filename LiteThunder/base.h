//
// Created by bin zhang on 2019/1/6.
//

#ifndef _BASE_H
#define _BASE_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>

#include <boost/any.hpp>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>            //任务队列的线程功能
//#include <boost/bind.hpp>            //任务队列的线程功能
#include <boost/any.hpp>            //任务队列的任务实现
#include <boost/asio.hpp>

typedef  std::map< std::string , boost::any > PropertyMap;
typedef  std::map< std::string , std::string > PropertyStringMap;

typedef unsigned char * BytePtr;
//typedef std::vector<unsigned char> ByteStream;
typedef std::vector<char> ByteArray;
typedef std::vector<std::string> StringList;

class  Object{
public:
	typedef std::shared_ptr<Object> Ptr;
	Ptr data(){
		return data_;
	}

	void data(const Ptr& ptr){
		data_ = ptr;
	}
	Object(){}
private:

protected:
	Ptr data_;
	std::condition_variable cv_;
	std::recursive_mutex rmutex_;
	std::mutex			mutex_;
};

#define SCOPED_LOCK  std::lock_guard<std::recursive_mutex> lock(mutex_);
#define SCOPED_LOCK_RECURSIVE(mtx)  std::lock_guard<std::recursive_mutex> lock(mtx);

//任务结构体
struct Task {
    int task_name;        //回调函数名称对应的常量
    boost::any task_data;        //数据结构体
    boost::any task_error;        //错误结构体
    int task_id;        //请求id
    bool task_last;        //是否为最后返回
};


#ifdef _ARM
#else
#define SETTINGS_FILE "settings.txt"
#define SETTINGS_USER_FILE "settings.user"
#define HOME_PATH "."
#endif

//#define _CHUANTOU 1

#endif //_BASE_H
