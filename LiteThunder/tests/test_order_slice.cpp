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

#include "../orderslice.h"


#include<bits/stdc++.h>

// 规格化价格
// price  报价 
// tick 最小变动价格
float fixed_price(float price ,float tick){
    float var = ( int(price/tick) * tick );
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << var;
    return boost::lexical_cast<float>(ss.str());    
}

int main(int argc , char ** argvs){
    std::cout<< fixed_price( 37.66666,0.05)<< std::endl;
    std::cout<< fixed_price( 1231,3)<< std::endl;

    // test_orderslice2();
	return 0 ;
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