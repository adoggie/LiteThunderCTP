g++ -c ../orderslice.cpp -std=c++11 
g++ -c ./test_order_slice.cpp -std=c++11 
g++ -o test_order_slice orderslice.o test_order_slice.o -lpthread 
./test_order_slice