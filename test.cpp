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

	vector<float> randomNumbers(N);

	auto p = Posit(32, 0);
	float random;
	
	float numero_s = 0;
	float numero_n = 0;	

//	while(1){
		//cin>>random;


		for(float i=-N;i<N;i+=0.001){
			p.set(i);
		
			//cout<<"Iniziale"<<endl;
			//p.print();

			auto p2 = p.div_p2(5);
			float result = p2.subconv();
			//p2.print();

			float proof = i/32;
			//float proof = i*64;
		
			auto p3 = Posit(32,0);
			p3.set(proof);
	//		cout<<"O"<<endl<<bitset<32>(p3.getBits())<<endl<<endl;
			//p3.print();

			//cout<<"diff "<<result-proof<<endl;
			float diff = fabs(result-proof);

			if(result == proof or ((diff) < 0.005)){
				//cout<<"OK"<<endl;
				//cout<<result<<endl;
				numero_s++;
			//	cout<<"OK"<<endl;
			}
			else {
				numero_n++;
				

				cout<<i<<" Posit: "<<result<<" Float: "<<proof<<endl<<endl;
				//p.print();					
				cout<<"FINALE"<<endl;
				p2.print();

				cout<<"UFFICIALE"<<endl;
				p3.print();
		
				break;
			}		

		}


		float n = numero_s + numero_n;
		cout<<"perc si "<<(numero_s/n)<<endl;

		//p2.print();
		//p2.set(0.75);



		//p2 = p.div_p2(4);
		//cout<<random<<"/4 = "<<p2.subconv()<<endl;
		//p2.print();


	//}    
	
	

   

    return 0;
}
