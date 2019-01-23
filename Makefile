posit.o: lib/posit.h lib/posit_types.h lib/posit.cpp  lib/util.h lib/pack.h lib/op2.h
	g++ lib/posit.cpp -c -std=c++14 -o posit.o

pack.o: lib/pack.c lib/pack.h lib/posit_types.h lib/util.h
	gcc lib/pack.c -c -std=c99 -o pack.o

op2.o:  lib/op2.c lib/op2.h lib/pack.h lib/util.h lib/pack.h
	gcc lib/op2.c -c -std=c99 -o op2.o

util.o: lib/util.c lib/util.h lib/posit_types.h
	gcc lib/util.c -c -std=c99 -o util.o

test.o: test.cpp
	g++ test.cpp -c -std=c++14 -I lib

test: test.o posit.o op2.o pack.o util.o
	g++ -o test test.o posit.o op2.o pack.o util.o

clean:
	rm -f *.o
