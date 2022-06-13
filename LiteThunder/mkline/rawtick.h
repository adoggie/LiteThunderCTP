//
// Created by scott on 2021/11/14.
//

#ifndef ELXTRADER_ROWTICK_H
#define ELXTRADER_ROWTICK_H


#include <chrono>
#include <memory>
#include <iterator>
#include <vector>
#include <map>
#include <algorithm>
#include <mutex>
#include <tuple>

#include "../config.h"
#include <ThostFtdcUserApiStruct.h>

namespace mkline{
struct Tick{
    double  open;
    double  high;
    double  low;
    double  close;
    double  opi;
    double  vol ;
};

#define DAY_SECONDS 3600 * 24
    typedef std::string SYMBOL;
    typedef  std::uint16_t  PERIOD;

#pragma pack(1)
struct TickCluster{
    TickCluster(){
        std::memset(this,0,sizeof(*this));
    }
    double open[DAY_SECONDS];
    double high[DAY_SECONDS];
    double low[DAY_SECONDS];
    double close[DAY_SECONDS];
    double opi[DAY_SECONDS];
    double vol[DAY_SECONDS];
};

struct KLine{
    KLine(){
        std::memset(this,0,sizeof(*this));
    }
    double open;
    double high;
    double low;
    double close;
    double opi;
    double vol;
    std::time_t  timestamp;
    PERIOD  period;

    void assign(const CThostFtdcDepthMarketDataField* d){

    }
};

struct KLineArray{
    PERIOD              period;
    std::list< std::shared_ptr<KLine> >  inner;
    std::mutex          mtx;

};

#pragma pack(pop)


// 定时切割 kline
struct KlineSplitter{

    std::time_t next;       // 分bar时间点
    std::uint16_t   peroid; // 5,10,15,20,...
};


//管理器
class Manager{
private:
    std::vector< std::shared_ptr<KlineSplitter> >   splitters_;
    std::map< SYMBOL  , std::map<PERIOD , KLineArray > >  klines_;
public:
    bool init(const Config& cfgs);
    bool open();
    void close();
    void onTick(const CThostFtdcDepthMarketDataField* tick);

    static Manager& instance(){
        static Manager manger;
        return manger;
    }

};

struct TradeTime{
    SYMBOL  symbol;
    std::vector< std::tuple<std::uint32_t,std::uint32_t> > ranges;
};

void Manager::onTick(const CThostFtdcDepthMarketDataField* tick){
    //1. 过滤非法时间段的tick
    TradeTime tt;
    std::uint32_t cur_sec ;
    bool intime = false;
    for(auto r: tt.ranges){
        if( std::get<0>(r) <= cur_sec and cur_sec < std::get<1> ){
            intime = true;
            break;
        }
    }
    if(!intime){
        return ;
    }

    //2.
    const std::uint32_t N =5;
    std::uint32_t  start_sec = 9 * 3600 ;
//    std::sscanf(input, "%d%f%9s%2d%f%*d %3[0-9]%2lc",
    KLineArray lines;
    std::shared_ptr<KLine> k;
    if( lines.inner.empty() ){
        k = std::make_shared<KLine>();
        lines.inner.push_back(k);
        //
        k->period = N;
        k->assign(tick);
        k->timestamp = (cur_sec / N) *N;
    }else{
        k = lines.inner[lines.inner.size()-1];
        if( cur_sec >= k->timestamp + k->period){
            k = std::make_shared<KLine>();
            lines.inner.push_back(k);
            k->period = N;
            k->assign(tick);
            k->timestamp = (cur_sec / N) *N;
        }else{
            k->assign(tick);
        }
    }
    (cur_sec - start_sec) / N
    std::vector< PERIOD > peroids;
    for(auto p : peroids){

    }

}

}

#endif //ELXTRADER_ROWTICK_H
