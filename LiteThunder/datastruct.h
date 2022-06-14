#ifndef DC580FF9_6B3C_4F18_B405_75CD022EDB20
#define DC580FF9_6B3C_4F18_B405_75CD022EDB20

#include "trade.h"
#include <cstdint>
#include <ctime>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <thread>
#include <iostream>
#include <mutex>
#include <atomic>


typedef std::string CommodityName ; 
typedef std::string InstrumentId ; 


struct PositionSignal{
    std::string     commodity;      // 可指定品种或某个月份合约
    std::string     instrument;     //
    std::time_t     time;
    int64_t    pos;
    std::string to_string(){
        std::stringstream  ss;
        ss << "PositionSignal comm:"<<commodity<<" instr:"<<instrument << " pos:"<<pos;
        return ss.str();
    }
};

#define SLICE_DIRECTION_LONG "long"
#define SLICE_DIRECTION_SHORT "short"
#define SLICE_OC_OPEN "open"
#define SLICE_OC_CLOSE "close"
#define SLICE_PRICE_MARKET "market"
#define SLICE_PRICE_LIMIT  "limit"

struct InstrumentConfig ;

struct  OrderSlice{
    typedef TThostFtdcOffsetFlagEnType CloseType; // 平仓值
    typedef std::shared_ptr<OrderSlice> Ptr;

    std::string         direction; // buy , sell
    std::string         openclose;  // open , close 
    int                 quantity;
    float               price = 0;  
    CloseType           closetype = THOST_FTDC_OFEN_Close;  // 平仓值
    std::time_t         start = 0;
    OrderReturn         order_ret;
    InstrumentConfig*   instcfg = NULL;
    int                 timeout = 30;
};

struct TradingTime{
    std::string id;
    std::vector< std::tuple<int,int> > times;
};

// 商品配置信息
struct CommodityConfig{
    typedef std::shared_ptr<CommodityConfig> Ptr;

    std::string     name;          // A,RB,C  商品种类
    InstrumentId    instrument;    // 交易合约
    float           weight;     // 权重 0 :  无效
    std::string     exchange;
    TradingTime     trading_time;
    int             multiplier;
    int           tick_size;
    float           tick_price ;    // 最小变动价格

};

#define MAX_POSITION_VALUE  0xffff00

// 交易合约配置信息
struct InstrumentConfig{
    typedef std::shared_ptr<InstrumentConfig> Ptr;

    std::string                     name;
    int                             expect = MAX_POSITION_VALUE; // 接收的仓位
    int                             target  = 0 ;   // 目标
    int                             timeout = 30 ;
    // std::string     price_type; // limit, market 
    int                             price_type;
    int                             ticks;      // up / down ticks 
    std::time_t                     start = 0 ;
    std::list< OrderSlice::Ptr >    slices;
    std::recursive_mutex            mtx;
    CommodityConfig *               comm = NULL;
    CThostFtdcInstrumentField      * detail = NULL;
    CThostFtdcInstrumentCommissionRateField * commission = NULL;
    CThostFtdcInstrumentMarginRateField*  margin = NULL;
    // bool                            done = false;
    std::string                     closemode; //平仓模式 margin:保证金模式 , fee:手续费模式
};

// 持仓
struct DirectionPosition{
    TThostFtdcPosiDirectionType direction ;
    int     volume  =0 ; 
    int     td = 0 ;
    int     yd = 0 ;  
    float   avg_price = 0;
    std::shared_ptr<CThostFtdcInvestorPositionField> underlying;
    
    int  getTd(){
        int td = 0 ;
        if( underlying){
            td = underlying->TodayPosition;
        }
        return td  ;
    }

    // 昨仓，不变
    int getYd(){
        int yd = 0 ;
        if( underlying){
            yd = underlying->YdPosition;
        }
        return yd ;
    }

    // 可平的昨仓
    int getYdAvaiable(){
        int yd = 0 ;
        if( underlying){
            yd = underlying->YdPosition - underlying->CloseVolume; // 当前规则：不平今，平的都是昨仓
        }
        return yd ;
    }

    // 当前开仓数量
    int getOpenVol(){
        int vol =0 ;
        if( underlying){
            vol = underlying->OpenVolume;
        }
        return vol ;
    }
};

struct Metrics{

};


#endif /* DC580FF9_6B3C_4F18_B405_75CD022EDB20 */
