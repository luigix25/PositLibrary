g++ lib/posit.cpp -c -std=c++14
gcc lib/pack.c -c -std=c99
gcc lib/op2.c -c -std=c99
gcc lib/util.c -c -std=c99
g++ test.cpp -c -std=c++14 -I lib
g++ -o test test.o posit.o op2.o pack.o util.o
rm *.o
