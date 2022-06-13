#include "tradetime.h"
#include "utils/os.h"

// 当前时间是否在交易时间段内 
// 处理 凌晨时间问题  , 21:00 - 02:00 ( 2100,0200 )

bool isInTradingTime(TradingTime * tt){
    std::tm  now = osutil::now();
    int tv  ; 
    tv = now.tm_hour * 100 + now.tm_min ;
    for( auto ts : tt->times){
        int start = std::get<0>(ts);
        int end = std::get<1>(ts);
        if( tv >= start && tv < end){
            return true;
        }
        if( end < start ){
            // 跨日 21:00 - 2:30
            if ( tv >= end || tv < start){
                return true;
            }
        }
    }

    return false;
}