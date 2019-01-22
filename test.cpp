#include "posit.h"

#include <cstdio>
#include <iostream>
#include <cstdlib>

#include <fstream>
#include <math.h>
#include <algorithm>
#include <vector>

#include <bitset>


#define N 100
#define MILLION 1000000


using namespace std;

union my_float{
	float decimale;
	int bits;
};


float sigmoid(float x){

	//return x/sqrtf((float)(x*x+1));
	//return x/(fabs(x)+1);
	return 1/(exp(-x)+1);


}

int segno(int a){

	return a>=0?0:1;

}

void sigmoid(Posit &p){

	int bits = (int)p.getBits();
	
	bits = bits ^ 0x80000000;
	bits = bits >> 2;
	p.setBits(bits);

}

float sigmoid_v2(Posit &p,int x){ //generalizzata su n bit

	my_float result;

	cout<<endl;
	uint32_t p_bits = p.getBits();
	cout<<bitset<32>(p_bits)<<endl;


	if(x <0){
		p_bits *= -1;
		p_bits &= 0x7FFFFFFF;
		p_bits |= 0x80000000;
	} else {
		p_bits &= 0x7FFFFFFF;
	}



	//cout<<bitset<32>(p_bits)<<endl<<endl;

	
	result.bits = p_bits;
	
	cout<<(result.decimale)<<endl;


	return result.decimale;

}


float sigmoid_v3(Posit &p,int x){

	uint32_t p_bits = p.getBits();

	if(x <0){
		p_bits *= -1;
		p_bits &= 0x7FFFFFFF;
		p_bits |= 0x80000000;
	} else {
		p_bits &= 0x7FFFFFFF;
	}


	sigmoid(p);

	my_float result;
	result.bits = p.getBits();

	return result.decimale;

}




float generateNumber(){			//numeri fra -30 e 30

	float tmp = (float)rand()/(float)RAND_MAX;

	return (tmp *60) - 30;	

}


int main(int argc, char *argv[])
{

	//vector<float> randomNumbers(N);

	auto p = Posit(32, 0);
	auto p2 = Posit(32, 0);

//	float random;
	
//	random = 0.3;

	p.set(0.2);
	p2.set(2.5);
	auto p3 = p+p2;

	cout<<p<<endl;
	cout<<p2<<endl;
	cout<<p3<<endl;

//	auto p3 = p*p2;
//	cout<<p3.sigmoid()<<endl;
	//cout<<p+p2<<endl;
	

   

    return 0;
}
