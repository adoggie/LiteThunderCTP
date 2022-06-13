

#include "orderslice.h"
#include <tuple>
#include <type_traits>

// 手续费模式(默认)
std::list< OrderSlice::Ptr > createOrderSlice(int target, int present){
    OrderSlice::Ptr slice;
    int delta = 0 ;
    std::list< OrderSlice::Ptr > slices;

    if( target == present){
        return slices;
    }
    //  -1,-3 or 1 ,3 
    if ( target * present > 0 ){
        delta =  std::abs(present) - std::abs(target);
        slice = std::make_shared<OrderSlice>();        
        if( target >0){ // 1,3  or 3,1            
            slice->direction = "buy";
            if( delta < 0 ){ // 1,3   open long , present:1 , target:3                
                slice->openclose = "open";                
            }else{ // 3, 1  close long , present:3 , target: 1 
                slice->direction = "sell";
                slice->openclose = "close";
            }
            
        }else{  // -3 , -1  or  -1,-3
            slice->direction = "sell";
            if( delta < 0 ){ //  present: -1 , target: -3
                slice->openclose = "open";
            }else{ // present:-3 , target:-1 
                slice->direction = "buy";
                slice->openclose = "close";
            }
        }
        slice->quantity = std::abs(delta);
        slice->start = std::time(NULL);
        // slice->price_type = SLICE_PRICE_LIMIT;
        slices.push_back(slice);

    }else if( target * present < 0 ){ // -1,3 or 1,-3   split two slices
        if( present < 0 ){  // present: -1 , target:3
            // step 1.  close present
            slice = std::make_shared<OrderSlice>();
            slice->direction = "buy";
            slice->openclose = "close";  // close abs(present)
            slice->quantity = std::abs(present); // 
            slices.push_back(slice);
            // step 2. open target
            slice = std::make_shared<OrderSlice>();

            slice->direction = "buy";
            slice->openclose = "open";  // close abs(present)
            slice->quantity = std::abs(target); // 
            slices.push_back(slice);
        }else{ // present > 0  , present 1 , target:-3
            // step 1.  close present
            slice = std::make_shared<OrderSlice>();
            slice->direction = "sell";
            slice->openclose = "close";  // close abs(present)
            slice->quantity = std::abs(present); // 
            slices.push_back(slice);
            // step 2. open target
            slice = std::make_shared<OrderSlice>();
            slice->direction = "sell";
            slice->openclose = "open";  // close abs(present)
            slice->quantity = std::abs(target); // 
            slices.push_back(slice);
        }
    }else{ // target * present == 0 
        if(target == 0 && present == 0){
            return std::list< OrderSlice::Ptr >();
        }
        if( present == 0){ // 0,-3 or 0,3   (present:0 ,target:-3/3)
            if( target < 0){ // 0,-3
                slice = std::make_shared<OrderSlice>();

                slice->direction = "sell";
                slice->openclose = "open";  // close abs(present)
                slice->quantity = std::abs(target); // 
                slices.push_back(slice);
            }else{ // 0,3
                slice = std::make_shared<OrderSlice>();

                slice->direction = "buy";
                slice->openclose = "open";  // close abs(present)
                slice->quantity = std::abs(target); // 
                slices.push_back(slice);
            }
        }else{ // target = 0  // -3,0 or 3,0  (present: -3/3 ,target: 0 )
            if( present < 0 ){ // -3,0
                slice = std::make_shared<OrderSlice>();

                slice->direction = "buy";
                slice->openclose = "close";  // close abs(present)
                slice->quantity = std::abs(present); // 
                slices.push_back(slice);
            }else{ // present > 0 , 3,0
                slice = std::make_shared<OrderSlice>();
                slice->direction = "sell";
                slice->openclose = "close";  // close abs(present)
                slice->quantity = std::abs(present); // 
                slices.push_back(slice);
            }
        }
    }
    return slices;
}

// 保证金模式，对锁仓位 ，反向开仓
std::list< OrderSlice::Ptr > createOrderSliceMargin(int target, int present,  int long_td ,  int short_td ){
    // OrderSlice::Ptr slice;
    auto slices = createOrderSlice(target,present);
    for( auto & slice: slices){
        if( slice->openclose == "close"){
            if( slice->direction == "buy" && short_td > 0 ){ //
                auto newslice = std::make_shared<OrderSlice>();
                *newslice = *slice;
                newslice->openclose = "open";
                newslice->direction = "buy";
                slices.push_back(newslice);
            }
            if (slice->direction == "sell" && long_td > 0 ){
                auto newslice = std::make_shared<OrderSlice>();
                *newslice = *slice;
                newslice->openclose = "open";
                newslice->direction = "sell";
                slices.push_back(newslice);
            }
        }
    }
    return slices;
}

// 保证金模式，对锁仓位 ，反向开仓

std::list< OrderSlice::Ptr > createOrderSliceMargin(const std::string& exchange,
        int target, int long_td ,int long_yd,  int short_td,int short_yd ){
    // OrderSlice::Ptr slice;
    /*
    d = target-present 
    if d > 0: 1~3 , -3~-1 , -3~1  买操作 
       buy_close_short_yd = min(short.yd,d) 平空头昨仓
       buy_open_long = d - buy_close_short_yd  开多头今仓
       
    else if d < 0:  卖操作
       sell_close_long_yd = min(long.yd, abs(d) )
       sell_open_short = d - sell_close_long_yd

    POLARIS  北极星
    OUTFIT 
    Who did the man like be?    
    SO MUCH PLACE THAT I WANT TO SHOW YOU 
    */
    int present = (long_td + long_yd - short_td - short_yd);
    int  diff  = target - present ;
    int buy_close,buy_open , sell_close , sell_open;
    buy_close = buy_open = sell_close = sell_open = 0;
    
    if(exchange == "SHFE" || exchange == "INE"){
        if(diff > 0 ){ // buy
            buy_close = std::min(short_yd , diff);
            buy_open = diff - buy_close;
            
        }else if( diff < 0){ // sell
            sell_close = std::min( long_yd,std::abs(diff));
            sell_open = std::abs(diff) - sell_close;
        }
    }else{ // 锁仓模式
        if(diff > 0 ){ // buy
            if (short_td == 0){ //无今仓，买平； 有今仓,不能买平，只能买开  
                buy_close = std::min(short_yd , diff);
            }
            buy_open = diff - buy_close;
        }else if( diff < 0){ // sell
            if( long_td == 0){
                sell_close = std::min( long_yd,std::abs(diff));
            }
            sell_open = std::abs(diff) - sell_close;
        }
    }

    std::list< OrderSlice::Ptr> slices;
    if( buy_open > 0){
        auto slice = std::make_shared<OrderSlice>();                
        slice->openclose = "open";
        slice->direction = "buy";
        slice->quantity = buy_open ;
        slices.push_back(slice);
    }

    if (buy_close > 0 ){
        auto slice = std::make_shared<OrderSlice>();                
        slice->openclose = "close";
        slice->direction = "buy";
        slice->quantity = buy_close ;
        slices.push_back(slice);
    }

    if( sell_open > 0){
        auto slice = std::make_shared<OrderSlice>();                
        slice->openclose = "open";
        slice->direction = "sell";
        slice->quantity = sell_open ;
        slices.push_back(slice);
    }

    if (sell_close > 0 ){
        auto slice = std::make_shared<OrderSlice>();                
        slice->openclose = "close";
        slice->direction = "sell";
        slice->quantity = sell_close ;
        slices.push_back(slice);
    }

    
    return slices;
}

//考虑锁仓情况
std::list< OrderSlice::Ptr > createOrderSlice( InstrumentConfig::Ptr instr, std::tuple<TD,YD> long_pos,std::tuple<TD,YD> short_pos,int target){

    return std::list< OrderSlice::Ptr>();
}

void test_orderslice(){
    int targets[] = {0,12,-4,5,7,-23};
    int presents[] = {4,12,-4,-10,1,9};
    for(int present: presents){
        for(int target : targets){
            std::cout << ">> present:"<< present << " target:" << target << std::endl;
            for ( auto slice : createOrderSlice(target,present) ){
                std::cout << slice->direction << " " << slice->openclose << " " << slice->quantity << std::endl; 
            }    
            std::cout <<"-------------------------"<< std::endl    ;    
        }
    }
    
}

void test_orderslice2(){

    int targets[] = {
            0,
            12,
            -4,
            5,
            7,
            2,
            -23
            };

    // <long_td,long_yd,short_td,short_yd> 
    std::tuple<int,int,int,int> presents[] = { 
            std::make_tuple(5,5,10,0),
            std::make_tuple(5,5,5,5),
            std::make_tuple(5,5,1,5),
            std::make_tuple(5,5,0,5),
            std::make_tuple(0,5,0,5),
            std::make_tuple(6,0,0,5),
            std::make_tuple(5,5,0,0),
            std::make_tuple(2,3,8,0),
            std::make_tuple(2,3,8,3),
        };
    
    std::string exchange = "SHFE";
    std::vector<std::string> exchanges = { "SHFE","DCE"};

    
    for(auto p: presents){
        
        for(int target : targets){
            for( auto exchange : exchanges){
                int long_td = std::get<0>(p);
                int long_yd = std::get<1>(p);
                int short_td = std::get<2>(p);
                int short_yd = std::get<3>(p);
                int present = (long_td + long_yd - short_td - short_yd);
                std::cout << ">> present:"<< present << " target:" << target << std::endl;
                std::cout << "   "<< "long_td:"<< long_td << " long_yd:"<<long_yd << " short_td:" << short_td << " short_yd:" << short_yd << std::endl;
                for ( auto slice : createOrderSliceMargin(exchange,target, 
                            long_td,long_yd,short_td,short_yd ) ){
                    std::cout << exchange << " " << slice->direction << " " << slice->openclose << " " << slice->quantity << std::endl; 
                    if( slice->direction == "buy"){
                        if(slice->openclose == "open"){
                            long_td += slice->quantity;
                        }
                        if(slice->openclose == "close"){
                            short_yd -= slice->quantity;
                        }
                    }
                    if( slice->direction == "sell"){
                        if(slice->direction == "open"){
                            short_td += slice->quantity;
                        }
                        if(slice->direction == "close"){
                            long_yd -= slice->quantity;
                        }
                    }
                }    
                std::cout << "   "<< "long_td:"<< long_td << " long_yd:"<<long_yd << " short_td:" << short_td << " short_yd:" << short_yd << std::endl;
                std::cout <<"-------------------------"<< std::endl    ;    
            }
        }
        std::cout <<"\n---------- EXCHANGE OVER ---------------\n"<< std::endl    ;    
    }
    
}