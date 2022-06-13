//
// Created by eladmin on 2021/11/2.
//

#ifndef ELXTRADER_HTTPCLIENT_H
#define ELXTRADER_HTTPCLIENT_H

/*
https://github.com/cesanta/mongoose/blob/master/examples/http-client/main.c
*/

#include <memory>
#include <map>
#include <vector>
#include "mongoose.h"
#include <boost/tuple/tuple.hpp>
#include <atomic>
#include <string>

class HttpClient {
public:
    typedef  std::map<std::string,std::string>  HEADERS;
    HttpClient();
    static boost::tuple<int,std::string> post( const std::string& url , const std::string& body,const HEADERS& headers);
//    static boost::tuple<int,std::string> get(const std::string& url,const std::map<std::string,std::string>& params, const HEADERS& headers );
    static  std::string make_params(const std::map<std::string,std::string>& params );
    static  std::string make_heads(const std::map<std::string,std::string>& params );
protected:
    static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);

private:
    std::atomic_bool s_exit_flag_;
    std::atomic_bool call_fail_;
    std::string     res_data_;
};


#endif //ELXTRADER_HTTPCLIENT_H
