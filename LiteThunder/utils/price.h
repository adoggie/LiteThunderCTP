#ifndef BFD132A0_40A8_41ED_B267_EA6A2CDCC451
#define BFD132A0_40A8_41ED_B267_EA6A2CDCC451

#include<bits/stdc++.h>
#include <boost/lexical_cast.hpp>

namespace tradeutil{

// 规格化价格
// price  报价 , tick 最小变动价格
inline 
float fixed_price(float price ,float tick){
    float var = ( int(price/tick) * tick );
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << var;
    return boost::lexical_cast<float>(ss.str());    
}

}

#endif /* BFD132A0_40A8_41ED_B267_EA6A2CDCC451 */
