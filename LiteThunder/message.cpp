#include "message.h"
#include <jsoncpp/json/json.h>
 
typedef std::function< std::shared_ptr<Message> (const Json::Value& root) > ParseFunc;

std::vector<ParseFunc> parse_func_list={
		MessageLoginResp::parse,
		MessageHeartBeat::parse,
		MessageDeviceStatusQuery::parse,
		MessageDeviceValueSet::parse,
};

Message::Ptr MessageJsonParser::parse(const char * data,size_t size){
	Json::Reader reader;
	Json::Value root;
	Message::Ptr msg;
	if (reader.parse(data, root)){
		for(auto func:parse_func_list){
			msg = func(root);
			if(msg){
				msg->id_ = root["id"].asString();
				break;
			}
		}
	}
	return msg;
}


std::shared_ptr<Message> MessageLoginResp::parse(const Json::Value& root){
	std::shared_ptr<Message> result;
	if( root["name"].asString() ==  MESSAGE_LOGIN_RESP){
		std::shared_ptr<MessageLoginResp> msg = std::make_shared<MessageLoginResp>();
		Json::Value values = root["values"];
		msg->error = values["error"].asInt();
		msg->message =  values["message"].asString();
		msg->server_time = values["server_time"].asUInt();
		msg->unmarshall(values);
		result = msg ;
	}
	return result;
}

std::shared_ptr<Message> MessageDeviceStatusQuery::parse(const Json::Value& root){
	std::shared_ptr<Message> result;
	if( root["name"].asString() ==  MESSAGE_STATUS_QUERY){
		std::shared_ptr<MessageDeviceStatusQuery> msg = std::make_shared<MessageDeviceStatusQuery>();
		Json::Value values = root["values"];
		msg->MessageTraverse::unmarshall(values);

		auto params = values["params"];
		if( params.isArray() ){
		    for(Json::ArrayIndex n=0; n< params.size() ; n++){
		        auto value = params[n].asString();
		        msg->params.push_back(value);
		    }
		}
		result = msg ;
	}
	return std::shared_ptr<MessageDeviceStatusQuery>();
}

std::shared_ptr<Message> MessageHeartBeat::parse(const Json::Value& root){
	std::shared_ptr<Message> result;
	if( root["name"].asString() == "heartbeat"){
		std::shared_ptr<MessageHeartBeat> msg = std::make_shared<MessageHeartBeat>();
		Json::Value values = root["values"];
		msg->unmarshall(values);
		result = msg;
	}
	return result;
}

std::shared_ptr<Message> MessageDeviceValueSet::parse(const Json::Value& root){
	std::shared_ptr<Message> result;
	if( root["name"].asString() == MESSAGE_VALUE_SET){
		auto msg = std::make_shared<MessageDeviceValueSet>();
		Json::Value values = root["values"];

        auto node = values["params"];
        if(node.isObject()){
            auto names = node.getMemberNames();
            for(auto & key : names){
                auto value = node[key].asString();
                msg->params[key] = value;
            }
        }

		msg->unmarshall(values);
		result = msg;
	}
	return result;
}

