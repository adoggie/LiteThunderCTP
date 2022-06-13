#include <iostream>
#include <string>
#include <algorithm>
#include <cstdio>
#include <list>
#include <vector>
#include <tuple>
#include <iostream>
#include <stdexcept>
#include <numeric>
#include <iterator>

#include <sstream>
#include <thread>
#include <chrono>
#include <future>

#include <boost/asio.hpp>
#include "app.h"


bool IsSpace(char x) { return x == ' '; }



typedef std::tuple< int,int> TwoInt;


std::string http_get(const std::string & url){
	std::cout<< "thread in http_get().."<<std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(5));
//	std::this_thread::sleep_for(std::chrono::seconds(2));

	return "get succ..";
}

void test_async(void){
	std::cout<< "start.."<<std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::future<std::string> f = std::async(http_get,"baidu.com");
//	std::cout<< "future result:"<< f.get() << std::endl;
	f.wait();
	std::cout<< "end.."<<std::endl;
}


void thread_promise( std::promise<std::string> & p ){
	std::this_thread::sleep_for(std::chrono::seconds(2));
	p.set_value("1000");
	std::puts("thread_promise exit..");
}


void test_promise(){
	std::promise<std::string> promise;
	std::thread thread(thread_promise,std::ref(promise) );
	auto f = promise.get_future();
	auto result = f.wait_for(std::chrono::seconds(5) );
	if (result == std::future_status::timeout){
		std::cout<< "wait timeout.."<<std::endl;
	}else if (result == std::future_status::ready){
		std::cout<<"result :"<< f.get() << std::endl;
	}
	thread.join();
}

class TT{};
class AA:TT{
public:
	AA(){name="some";}
	AA(int name):AA(){}
	std::string name;
};

void test_getline(){
	boost::asio::streambuf streambuf;
	std::ostream os(&streambuf);
	os<<" shanghai begin guangzhou";

	std::string text;
	std::istream is(&streambuf);
	std::string ss;
	ss.assign( (const char*)streambuf.data().data(),streambuf.data().size());
	std::cout<<ss<<std::endl;
	
	while(streambuf.size()){
		std::getline(is,text,'#');
		std::cout<<text<<std::endl;
	}
	
	AA *a = new AA();
	std::shared_ptr<TT> aa((TT*)new AA());
	
	std::cout<< typeid(a).name() << std::endl;
	std::cout<< typeid(AA*).name() << std::endl;
	if( typeid(*aa) ==  typeid(AA)){
		std::puts("equal..");
	}
	if(!aa){}
	
	std::string x = AA().name;
//	x == "shanghai";
	
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(""),123);
}


int _main() {
	std::cout << "Hello, World!" << std::endl;
	std::string name = "name is shanghai.";
//	auto itr = std::remove_if(name.begin(),name.end(),std::isspace);

	std::list<std::string> names={"cily","scott","eric"};

	std::string str2 = "Text with some  spaces";
	auto itr = std::remove_if(str2.begin(),  str2.end(),IsSpace);
//	str2.erase( itr, str2.end());
//	str2.erase(str2.find(' '));
	std::cout<< str2<< std::endl;
//
	std::for_each(itr,str2.end(),[](const unsigned char & ch){
		std::cout<< ch<< std::endl;
	});

	std::for_each(names.begin(),names.end(),[](const std::string& s){
		std::cout<<s<<std::endl;
	});

	TwoInt ti = std::make_tuple(1,2);
//	ti = {2,3};
	std::cout<< std::get<0>(ti) <<std::endl;

	std::cout<< std::hex << 42 << '\n';


	std::vector<int> nums ={1,2,3};
	std::vector<int> nums_2 ;

	int result = std::accumulate(nums.begin(),nums.end(),100);


	std::stringstream ss;
	ss<<"0x" << std::hex << result << std::endl;

	std::cout<< ss.str() ;

	std::copy(nums.cbegin(),nums.cend(),std::back_inserter(nums_2));
	std::for_each(nums_2.begin(),nums_2.end(),[](const  int & v){
		std::cout<<std::hex<<v<<" ";
	});

	std::vector<int> nums_3 ;

	std::transform(nums.begin(),nums.end(),std::back_inserter(nums_3),[](const int & v){
		return v+100;
	});

	std::for_each(nums_3.begin(),nums_3.end(),[](const  int & v){
		std::cout<<std::dec<<v<<" ";
	});


//	test_async();
//	test_promise();
	test_getline();
	return 0;
}

int main(int argc , char ** argvs){
    Application::instance()->init().run();
}



/*
 在C++11中这一部分被成为捕获外部变量

捕获外部变量
[captures] (params) mutable-> type{...} //lambda 表达式的完整形式

在 lambda 表达式引出操作符[ ]里的“captures”称为“捕获列表”，可以捕获表达式外部作用域的变量，在函数体内部直接使用，这是与普通函数或函数对象最大的不同（C++里的包闭必须显示指定捕获，而lua语言里的则是默认直接捕获所有外部变量。）

捕获列表里可以有多个捕获选项，以逗号分隔，使用了略微“新奇”的语法，规则如下

[ ]        ：无捕获，函数体内不能访问任何外部变量
[ =]      ：以值（拷贝）的方式捕获所有外部变量，函数体内可以访问，但是不能修改。
[ &]      ：以引用的方式捕获所有外部变量，函数体内可以访问并修改（需要当心无效的引用）；
[ var]   ：以值（拷贝）的方式捕获某个外部变量，函数体可以访问但不能修改。
[ &var] ：以引用的方式获取某个外部变量，函数体可以访问并修改
[ this]   ：捕获this指针，可以访问类的成员变量和函数，
[ =，&var] ：引用捕获变量var，其他外部变量使用值捕获。
[ &，var]：只捕获变量var，其他外部变量使用引用捕获。


 */