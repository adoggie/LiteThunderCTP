#ifndef FFAF57BE_2141_4AA2_AA6C_B1F9C2068CFD
#define FFAF57BE_2141_4AA2_AA6C_B1F9C2068CFD

#include "datastruct.h"

std::list< OrderSlice::Ptr > createOrderSlice(int target, int present);
std::list< OrderSlice::Ptr > createOrderSliceMargin(int target, int present, int td , int yd );

void test_orderslice();

#endif /* FFAF57BE_2141_4AA2_AA6C_B1F9C2068CFD */
