//
// Created by eladmin on 2021/11/2.
//

#include "HttpClient.h"
#include <boost/algorithm/string.hpp>
#include "mongoose.h"


//static int s_exit_flag = 0;
//static int s_show_headers = 0;
//static const char *s_show_headers_opt = "--show-headers";

void HttpClient::ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    struct http_message *hm = (struct http_message *) ev_data;
    HttpClient * client = (HttpClient*)nc->mgr_data ;

    switch (ev) {
        case MG_EV_CONNECT:
            if (*(int *) ev_data != 0) {
//                fprintf(stderr, "connect() failed: %s\n", strerror(*(int *) ev_data));
//                s_exit_flag = 1;
                client->s_exit_flag_ = true;
                client->call_fail_ = true;
            }
            break;
        case MG_EV_HTTP_REPLY:
            nc->flags |= MG_F_CLOSE_IMMEDIATELY;
            client->res_data_.assign(hm->body.p,hm->body.len);

//            if (s_show_headers) {
//                fwrite(hm->message.p, 1, hm->message.len, stdout);
//            } else {
//                fwrite(hm->body.p, 1, hm->body.len, stdout);
//            }
//            putchar('\n');
//            s_exit_flag = 1;
            client->s_exit_flag_ = true;
            break;
        case MG_EV_CLOSE:
//            if (s_exit_flag == 0) {
//                printf("Server closed connection\n");
//                s_exit_flag = 1;
//            }
            client->s_exit_flag_ = true;
            break;

        case MG_EV_TIMER:
            client->s_exit_flag_ = true;
            client->call_fail_ = true;
            break;
        default:
            break;
    }
}

HttpClient::HttpClient(){
    s_exit_flag_ = false;
    call_fail_ = false;
}

//std::string post( const std::string& url , const std::string& body,const HEADERS& headers);
boost::tuple<int,std::string>
        HttpClient::post(const std::string& url,const std::string& body, const HEADERS& headers ){

    int status = 0;
    std::string res;

    HttpClient client;
    struct mg_mgr mgr;

    mg_mgr_init(&mgr, (void*) &client);
    const char* head = "Content-Type: application/x-www-form-urlencoded\r\n";
//    "var_1=value_1&var_2=value_2"
    if(body.size()) {
        mg_connect_http(&mgr, HttpClient::ev_handler, url.c_str(), head, body.c_str());
    }else{
        mg_connect_http(&mgr, HttpClient::ev_handler, url.c_str(), head, NULL);
    }

    while ( !client.s_exit_flag_ ) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);
    if(!client.call_fail_){
        res = client.res_data_;
    }
    return boost::make_tuple(status,res);
}

std::string HttpClient::make_params(const std::map<std::string,std::string>& params ){
    std::vector<std::string> ss;
    std::transform(params.begin(),params.end(),std::back_inserter(ss),
                   [](const std::pair<std::string,std::string>& kv) ->std::string{
       return kv.first +"="+ kv.second;
    });

//    std::for_each(params.begin(),params.end(),[ss](const std::pair<std::string,std::string>& kv) {
////        ss.push_back( kv.first +"="+ kv.second);
//    });

    std::string joined = boost::join(ss, "&");
    return joined;
}


std::string HttpClient::make_heads(const std::map<std::string,std::string>& params ){
    std::vector<std::string> ss;
    std::transform(params.begin(),params.end(),std::back_inserter(ss),
                   [](const std::pair<std::string,std::string>& kv) ->std::string{
                       return kv.first +": "+ kv.second;
                   });

//    std::for_each(params.begin(),params.end(),[ss](const std::pair<std::string,std::string>& kv) {
////        ss.push_back( kv.first +"="+ kv.second);
//    });

    std::string joined = boost::join(ss, "\r\n");
    return joined;
}
