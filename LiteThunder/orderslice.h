#ifndef FFAF57BE_2141_4AA2_AA6C_B1F9C2068CFD
#define FFAF57BE_2141_4AA2_AA6C_B1F9C2068CFD

#include "datastruct.h"


typedef int YD;
typedef int TD;
std::list< OrderSlice::Ptr > createOrderSlice(int target, int present);
std::list< OrderSlice::Ptr > createOrderSliceMargin(int target, int present, int td , int yd );
std::list< OrderSlice::Ptr > createOrderSliceMargin(const std::string& exchange,
        int target, int long_td ,int long_yd,  int short_td,int short_yd );

void test_orderslice();
void test_orderslice2();

#endif /* FFAF57BE_2141_4AA2_AA6C_B1F9C2068CFD */
