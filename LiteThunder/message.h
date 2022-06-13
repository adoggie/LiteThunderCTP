//
// Created by bin zhang on 2019/1/6.
//

#ifndef SMARTBOX_MESSAGE_H
#define SMARTBOX_MESSAGE_H

#include "base.h"
#include <jsoncpp/json/json.h>

#define MESSAGE_TRAVERSE_DOWN     	"traverse_down"       //下行消息
#define MESSAGE_TRAVERSE_UP     	"traverse_up"       //上行消息
#define MESSAGE_LOGIN     			"login"       // 设备登录
#define MESSAGE_LOGIN_RESP     		"login_resp"       // 设备登录反馈
#define MESSAGE_HEARTBEAT     		"heartbeat"       // 设备登录反馈
#define MESSAGE_STATUS_QUERY     	"data_query"       // 设备登录反馈
#define MESSAGE_STATUS     			"data_report"       	// 设备状态上报
#define MESSAGE_VALUE_SET     		"data_set"       // 设备参数设置


class MessageJsonParser;

struct MessagePayload;

struct Message {
	std::string id_;
	std::string name_;
	PropertyMap values_;
	friend class MessageJsonParser;
public:
	Message(){}
	Message(const std::string & name):name_(name){}
	virtual ~Message(){}
public:
	typedef std::shared_ptr<Message> Ptr;
	
	boost::any value(const std::string& name,boost::any def_=boost::any())  {
		try {
			return values_.at(name);
		}catch (...){
		
		}
		return def_;
	}
	
	std::string getValueString(const std::string&name , const std::string& def_=""){
		try {
			return boost::any_cast<std::string>(this->value(name));
		}catch(...){
			return def_;
		}
	}
	
	std::string& name() { return name_;}
	virtual Json::Value values(){ return Json::Value();}
	
	virtual  std::string marshall(){
		Json::Value root;
		Json::Value arrayObj;
		Json::Value item;
		root["id"] = id_;
		root["name"] = name_;
		Json::Value values = this->values();
		if( !values.isNull()){
			root["values"] = values;
		}
		return root.toStyledString();
	}
	virtual bool unmarshall(const Json::Value& root){ return false;}
};

// 消息传输报文基类
struct MessageTraverse:Message{
	std::string device_id;          // 设备编号
    MessageTraverse(){}
    virtual ~MessageTraverse(){}
    MessageTraverse(const std::string& name):Message(name){
	}
	virtual bool unmarshall(const Json::Value& root){
    	device_id = root["device_id"].asString();
    	return true;
    }

	static std::shared_ptr<Message> parse(const Json::Value& root);
	virtual Json::Value values(){
		Json::Value value = Message::values();
		value["device_id"] = device_id;
		return value;
	}
};

// 平台到设备报文（下行)
struct MessageTraverseDown:MessageTraverse{
	MessageTraverseDown():MessageTraverse(MESSAGE_TRAVERSE_DOWN){
	}

	MessageTraverseDown(const std::string& name):MessageTraverse(name){
	}

    virtual ~MessageTraverseDown(){}
	static std::shared_ptr<Message> parse(const Json::Value& root){
		return std::shared_ptr<Message>();
	}
};

struct MessageTraverseUp:MessageTraverse{
	MessageTraverseUp():MessageTraverse(MESSAGE_TRAVERSE_UP){
	}

    MessageTraverseUp(const std::string& name ):MessageTraverse(name){
	}

	virtual ~MessageTraverseUp(){}

	static std::shared_ptr<Message> parse(const Json::Value& root){
		return std::shared_ptr<Message>();
	}
};

struct MessageLogin:MessageTraverseUp{
    std::string token;
    MessageLogin():MessageTraverseUp(MESSAGE_LOGIN){
    }

	static std::shared_ptr<Message> parse(const Json::Value& root);

	virtual Json::Value values(){
		Json::Value value = MessageTraverseUp::values();
		value["token"] = token;
		return value;
	}
};

// 设备登录反馈
struct MessageLoginResp:MessageTraverseDown{
    int             error;		// 0 : okay , else failed
    std::string     message;
    std::uint32_t   server_time;
    MessageLoginResp():MessageTraverseDown(MESSAGE_LOGIN_RESP){
    }
    static std::shared_ptr<Message> parse(const Json::Value& root);

};

struct MessageHeartBeat:MessageTraverse{
    MessageHeartBeat():MessageTraverse(MESSAGE_HEARTBEAT){
	}
	
	static std::shared_ptr<Message> parse(const Json::Value& root);
};

struct MessageDeviceStatusQuery:MessageTraverseDown{
    std::vector<std::string> params;
    MessageDeviceStatusQuery():MessageTraverseDown(MESSAGE_STATUS_QUERY){
	}
	
	static std::shared_ptr<Message> parse(const Json::Value& root);
	//std::shared_ptr<MessagePayload> asPayload() const;
};

struct MessageDeviceStatus:MessageTraverseUp{

    std::string  type; // status or event
	PropertyStringMap params;

    MessageDeviceStatus():MessageTraverseUp(MESSAGE_STATUS){
        this->type = "status";
	}

	Json::Value values(){
		Json::Value root = MessageTraverseUp::values();
		Json::Value array;
		for(auto & _ : params){
			array[_.first] = _.second;
		}
		root["params"] = array;
		root["type"] = type;
		return root;
	}
	
	static std::shared_ptr<Message> parse(const Json::Value& root);
	std::shared_ptr<MessagePayload> asPayload() const;
};

struct MessageDeviceEvent:MessageDeviceStatus{
    MessageDeviceEvent(){
        this->type = "event";
    }
};

struct MessageDeviceValueSet:MessageTraverseDown{

    PropertyStringMap params;

    MessageDeviceValueSet():MessageTraverseDown(MESSAGE_VALUE_SET){
    }

    Json::Value values(){
        Json::Value root;
        for( auto & it : params){
            root[it.first] = it.second;
        }
        return root;
    }

    static std::shared_ptr<Message> parse(const Json::Value& root);
	std::shared_ptr<MessagePayload> asPayload() const;
};




class MessageJsonParser{
public:
	static Message::Ptr parse(const char * data,size_t size);
	
};






#endif
