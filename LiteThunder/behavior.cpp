//
// Created by scott on 2021/11/4.
//

#include "behavior.h"
#include "Controller.h"

TdApi *  Behavior::getTradeApi() const{
    return Controller::instance()->getTradeApi();
}

MdApi *  Behavior::getMarketApi() const{
    return Controller::instance()->getMarketApi();
}
