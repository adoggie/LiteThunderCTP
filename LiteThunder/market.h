//说明部分
#ifndef ELXTRADE_MARKET_H
#define ELXTRADE_MARKET_H

#include <string>
#include <queue>

#include "base.h"
#include "config.h"
#include <ThostFtdcMdApi.h>

using namespace std;
using namespace boost;


//API的继承实现
class MdApi : public CThostFtdcMdSpi {
public:
    MdApi() {
    };

    ~MdApi() {
    };

    static MdApi& instance();

private:
    ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    virtual void OnFrontConnected();
    ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    ///@param nReason 错误原因
    ///        0x1001 网络读失败
    ///        0x1002 网络写失败
    ///        0x2001 接收心跳超时
    ///        0x2002 发送心跳失败
    ///        0x2003 收到错误报文
    virtual void OnFrontDisconnected(int nReason);

    ///心跳超时警告。当长时间未收到报文时，该方法被调用。
    ///@param nTimeLapse 距离上次接收报文的时间
    virtual void OnHeartBeatWarning(int nTimeLapse);

    ///登录请求响应
    virtual void
    OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                   bool bIsLast);

    ///登出请求响应
    virtual void
    OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                    bool bIsLast);

    ///错误应答
    virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///订阅行情应答
    virtual void
    OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                       int nRequestID, bool bIsLast);

    ///取消订阅行情应答
    virtual void
    OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                         int nRequestID, bool bIsLast);

    ///订阅询价应答
    virtual void
    OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                        int nRequestID, bool bIsLast);

    ///取消订阅询价应答
    virtual void
    OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                          int nRequestID, bool bIsLast);

    ///深度行情通知
    virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

    ///询价通知
    virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp);

    virtual void onFrontConnected();

    virtual void onFrontDisconnected(int i);

    virtual void onHeartBeatWarning(int i);

//	virtual void onRspUserLogin(dict data, dict error, int id, bool last) {};
    virtual void onRspUserLogin(CThostFtdcRspUserLoginField *data, CThostFtdcRspInfoField *error, int id, bool last);

    virtual void onRspUserLogout(CThostFtdcUserLogoutField *data, CThostFtdcRspInfoField *error, int id, bool last);

//	virtual void onRspError(CThostFtdcRspInfoField* error, int id, bool last) ;

    virtual void
    onRspSubMarketData(CThostFtdcSpecificInstrumentField *data, CThostFtdcRspInfoField *error, int id, bool last);

    void createFtdcMdApi(string pszFlowPath = "");
    void release();
    int exit();

    string getTradingDay();
    void registerFront(string pszFrontAddress);

    int subscribeForQuoteRsp(string instrumentID);
    int unSubscribeForQuoteRsp(string instrumentID);
    int reqUserLogin(CThostFtdcReqUserLoginField *req, int nRequestID);
    int reqUserLogout(CThostFtdcUserLogoutField *req, int nRequestID);
    bool connect();
    void disconnect();
    int subscribeMarketData(string instrumentID);
    int unSubscribeMarketData(string instrumentID);

public:
    int join();
    void init(const Config& cfg);
    int subscribeMarketData(const std::vector<std::string> & instruments);
    int unsubscribeMarketData(const std::vector<std::string> & instruments);
    bool start();
    void stop();

protected:
    CThostFtdcMdApi *           api;                //API对象
    std::vector<std::string>    instruments_;
    boost::mutex                mtx_instruments_;
    std::shared_ptr<boost::asio::deadline_timer>    timer_;

};


#endif
