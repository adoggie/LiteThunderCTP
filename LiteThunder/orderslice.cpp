

#include "orderslice.h"
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