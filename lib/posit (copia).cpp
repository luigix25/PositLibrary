#include "posit.h"
#include "util.h"
#include "pack.h"
#include "op2.h"

#include <cstdio>
#include <cmath>
#include <iostream>

#include <bitset>

using namespace std;

Posit::Posit(POSIT_UTYPE bits, int nbits, int es, bool nan) :
    mBits(bits),
    mNbits(nbits),
    mEs(es),
    mNan(nan)
{
}

Posit::Posit(int nbits, int es, bool nan) :
    Posit(0, nbits, es, nan)
{
}

Posit::Posit(int nbits, int es) :
    Posit(nbits, es, false)
{
}

bool Posit::isZero()
{
    return util_is_zero(mBits);
}

bool Posit::isOne()
{
    return util_is_one(mBits);
}

bool Posit::isInf()
{
    return util_is_inf(mBits);
}

bool Posit::isNeg()
{
    return util_is_neg(mBits);
}

bool Posit::isNan()
{
    return mNan;
}

int Posit::nbits()
{
    return mNbits;
}

int Posit::ss()
{
    return util_ss();
}

int Posit::rs()
{
    return util_rs(mBits, mNbits);
}

int Posit::es()
{
    return util_es(mBits, mNbits, mEs);
}

int Posit::fs()
{
    return util_fs(mBits, mNbits, mEs);
}

Posit Posit::zero()
{
    return Posit(POSIT_ZERO, mNbits, mEs, false);
}

Posit Posit::one()
{
    return Posit(POSIT_ONE, mNbits, mEs, false);
}

Posit Posit::inf()
{
    return Posit(POSIT_INF, mNbits, mEs, false);
}

Posit Posit::nan()
{
    return Posit(mNbits, mEs, true);
}

Posit Posit::neg()
{
    return Posit(util_neg(mBits, mNbits), mNbits, mEs, false);
}

Posit Posit::rec()
{
    return Posit(POSIT_ONE, mNbits, mEs, false).div(*this);
}

int Posit::regime()
{
    POSIT_UTYPE bits = isNeg() ? neg().mBits : mBits;
    int lz = CLZ(bits << ss());
    int lo = CLZ(~bits << ss());

    return (lz == 0 ? lo - 1 : -lz);
}

Posit Posit::add(Posit& p)
{
    // fast exit
    if (isZero()) {
        return p;
    } else if (p.isZero()) {
        return *this;
    } else if (isInf() && p.isInf()) {
        return nan();
    } else if (isInf() || p.isInf()) {
        return inf();
    } else if (neg().eq(p)) {
        return zero();
    }

    unpacked_t aup = unpack_posit(mBits, mNbits, mEs);
    unpacked_t bup = unpack_posit(p.mBits, p.mNbits, p.mEs);
    unpacked_t up = op2_add(aup, bup);

    return Posit(pack_posit(up, mNbits, mEs), mNbits, mEs, false);
}

Posit Posit::sub(Posit& p)
{
    // fast exit
    if (isZero()) {
        return p.neg();
    } else if (p.isZero()) {
        return *this;
    } else if (isInf() && p.isInf()) {
        return nan();
    } else if (isInf() || p.isInf()) {
        return inf();
    } else if (eq(p)) {
        return zero();
    }

    unpacked_t aup = unpack_posit(mBits, mNbits, mEs);
    unpacked_t bup = unpack_posit(p.mBits, p.mNbits, p.mEs);
    unpacked_t up = op2_sub(aup, bup);

    return Posit(pack_posit(up, mNbits, mEs), mNbits, mEs, false);
}

Posit Posit::mul(Posit& p)
{
    // fast exit
    if (isZero()) {
        return (p.isInf() ? nan() : zero());
    } else if (p.isZero()) {
        return (isInf() ? nan() : zero());
    } else if (isInf() || p.isInf()) {
        return inf();
    }

    unpacked_t aup = unpack_posit(mBits, mNbits, mEs);
    unpacked_t bup = unpack_posit(p.mBits, p.mNbits, p.mEs);
    unpacked_t up = op2_mul(aup, bup);

    return Posit(pack_posit(up, mNbits, mEs), mNbits, mEs, false);
}

Posit Posit::div(Posit& p)
{
    // fast exit
    if (isInf()) {
        return (p.isInf() ? nan() : inf());
    } else if (p.isZero()) {
        return (isZero() ? nan() : inf());
    } else if (isZero() || p.isInf()) {
        return zero();
    }

    unpacked_t aup = unpack_posit(mBits, mNbits, mEs);
    unpacked_t bup = unpack_posit(p.mBits, p.mNbits, p.mEs);
    unpacked_t up = op2_div(aup, bup);

    return Posit(pack_posit(up, mNbits, mEs), mNbits, mEs, false);
}

Posit Posit::mul_p2(int k){

	int sign = mBits;
	
	if((mBits & 0xFFFFFFFF)==0)				//se è 0 termino 
		return Posit(0,mNbits,0,false);

	sign &= 0x80000000;
	unsigned int bit = mBits;

	if(sign == 0x80000000){						//se negaivo ne faccio not x +1
		bit *= -1; 						//nota: si potrebbe fare ^= 0xFFFF..F +1 
	}
	

	int regime_bits = regime();

	//cout<<"REGIME: "<<regime_bits<<endl;
	
	if((regime_bits < 0) && (regime_bits +k >=0)){					// vi è un cambio segno nel regime
		//cout<<"CASO CON CAMBIO REGIME"<<endl;
		
		//considero i regimi negativi
		int eccesso = regime_bits +k;
		//cout<<eccesso<<endl;

		int quanti = -regime_bits;
		if(regime_bits == 0)
			quanti = 2;
	
		//annullo il regime

		bit = bit<<1;					//elimino il segno;

		//for(int i=quanti;quanti >0;quanti--){	
		bit=bit<<(quanti+1);				//l'1 per il tappo						
		//}
		
		bit=bit>>1;						//inserisco lo 0
		bit |= 0x80000000;					//tappo per regime 0 
							
		int bit_s = (signed int)bit;
		bit_s = bit_s >> (eccesso);

		bit = (unsigned int)bit_s;
		


		
		bit=bit>>1;							//segno

		int numero_zeri = eccesso+2-quanti-1;
		//cout<<"numero zeri "<<numero_zeri<<endl;


	

		unsigned int maschera = 0xFFFFFFFF;
		int n_bit_reg = rs();

		maschera = maschera>>(n_bit_reg+1);			//maschero il regime e segno
		
		unsigned int fraz = bit & maschera;		//prendo solo la frazione
		fraz = fraz >> numero_zeri;

		bit = bit & (~maschera);
		bit |= fraz;

		if(sign == 0x80000000){						//se negaivo ne faccio not x +1
			bit *= -1; 						//nota: si potrebbe fare ^= 0xFFFF..F +1 
		}	

		/*int n_bit_reg = rs();
		int bit_fraz = 32-1-n_bit_reg;

		setBits(bit);
		print();
		cout<<"Regime Finale "<<regime()<<endl;

		int n_bit_reg_new = rs();
		int bit_fraz_new = 32-1-n_bit_reg_new;

		cout<<"FRAZIONE PRIMA: "<<bit_fraz<<" FRAZ DOPO: "<<bit_fraz_new<<endl;
*/
		//setBits(bit);
		//cout<<"REGIME FINALE "<<regime()<<endl;

		return Posit(bit,mNbits, 0, false);
	}	

	

	if(regime_bits>=0){
		bit=bit<<1;						//elimino il segno;
		int bit_s = bit;					//così inserisco 1
		
		//for(int i=k;i >0;i--){
		bit_s=bit_s>>k;
		//}
		bit = (unsigned int)bit_s;
		
		bit=bit>>1;						//segno
		

	} else{
		bit=bit<<1;
		
		bit=bit<<k;

		bit=bit>>1;						//segno;	

		
	}

	if(sign == 0x80000000){						//se negaivo ne faccio not x +1
		bit *= -1; 						//nota: si potrebbe fare ^= 0xFFFF..F +1 
	}

		//setBits(bit);
		//cout<<"REGIME FINALE "<<regime()<<endl;


    return Posit(bit,mNbits, 0, false);

}

POSIT_UTYPE Posit::lfraction()
{
    return mBits << (ss() + rs() + mEs);
}


Posit Posit::div_p2(int k){				//non funzionante al 100%

	int sign = mBits;
	
	if((mBits & 0xFFFFFFFF)==0)				//se è 0 termino 
		return Posit(0,mNbits,0,false);

	sign &= 0x80000000;
	unsigned int bit = mBits;

	if(sign == 0x80000000){						//se negaivo ne faccio not x +1
		bit *= -1; 						//nota: si potrebbe fare ^= 0xFFFF..F +1 
	}

	//cout<<bitset<32>(sign)<<endl<<endl;
	


	//print();

	int regime_bits = regime();

	cout<<"REGIME: "<<regime_bits<<endl;
	
	if((regime_bits > 0) && (regime_bits -k <=0)){					// vi è un cambio segno nel regime
		bit = bit<<1;						//elimino il bit di segno;
	
		cout<<"CASO CON CAMBIO "<<endl;

		//considero i regimi positivi
		int eccesso = k - regime_bits;
		int quanti = regime_bits;
		//moltiplico per quanti (annullo il regime)	NOTA: possibile usare mul_p2

		//for(int i=quanti;quanti >0;quanti--){	
			bit=bit<<quanti;						
		//}
		bit |= 0x80000000;				//tappo
		bit=bit>>1;					//SISTEMARE SEGNO OK		

		//regime portato a 0;

		setBits(bit);
		cout<<"REGIME ( 0 ): "<<regime()<<endl;

		if(eccesso == 0){
			if(sign == 0x80000000){						//se negaivo ne faccio not x +1
				bit *= -1; 						//nota: si potrebbe fare ^= 0xFFFF..F +1 
			}
			
			return Posit(bit,mNbits,0,false); 
		}
		
		//for(int i=eccesso-1;i>0;i--){						//modificato qua
		bit = bit>>(eccesso+1);
		//}

		//bit=bit>>1;					//bit di segno;
		
		if(sign == 0x80000000){						//se negaivo ne faccio not x +1
			bit *= -1; 						//nota: si potrebbe fare ^= 0xFFFF..F +1 
		}


		setBits(bit);
		cout<<"REGIME_FINALE: "<<regime()<<endl;
 
		//NOTA: SISTEMARE SEGNO OK
		return Posit(bit,mNbits,0,false);

	}

	cout<<"CASO GENERALE"<<endl;
	
	//unpacked_t aup = unpack_posit(bit, 32, 0);
	//cout<<bitset<32>(bit)<<" COMPLETO"<<endl;
	//cout<<"FRAZIONE "<<bitset<32>(aup.frac)<<endl;



	if(regime_bits>0){
		bit=bit<<1;				//elimino il segno;
		
		//for(int i=k;i >0;i--)	
		bit=bit<<k;
		
		bit=bit>>1;						//segno
		

	} else if(regime_bits == 0){

		

	} else {
	
		int fs_iniziale = fs();
		unsigned int maschera = 0xFFFFFFFF;
		int n_bit_reg = rs();

		maschera = maschera>>(n_bit_reg+1);			//maschero il regime
		
		unsigned int frac_iniziale = bit & maschera;		//prendo solo la frazione

		//cout<<"FRAZIONE INIZIALE "<<bitset<32>(frac_iniziale)<<endl;		

		bit = bit<<1;						//elimino il segno
	
		//for(int i=k;i >0;i--){
		bit=bit>>k;
		//}
		
		bit=bit>>1;				//TEMP
		setBits(bit);
		
		int fs_finale = fs();
		
		int dif_frac = fs_finale - fs_iniziale;
		
		//if(dif_frac > 0){
		//	frac_iniziale = frac_iniziale << dif_frac;
		/*} else */
		if(dif_frac < 0){
			dif_frac *=-1;
			//cout<<"DIF FRAC "<<dif_frac<<endl;
			frac_iniziale = frac_iniziale << (1);
		}

	
		maschera = 0xFFFFFFFF;
		n_bit_reg = rs();

		maschera = maschera>>(n_bit_reg+1);			//maschero il regime
		maschera = ~maschera;		

		unsigned int frac = bit & maschera;			//elimino la frazione
		//cout<<"FRACTION "<<bitset<32>(frac)<<endl;

		bit |= frac_iniziale;
		
		setBits(bit);
	}

	if(sign == 0x80000000){						//se negaivo ne faccio not x +1
		bit *= -1; 						//nota: si potrebbe fare ^= 0xFFFF..F +1 
	}

	/*if(regime_bits == 0){
		bit |= 0x80000000;				//tappo
		//bit=bit>>1;
	}*/

	setBits(bit);
	//cout<<"REGIME: "<<regime()<<endl;
	//print();



	//while(k>0){

		
	//}

	
	//bit |= sign;


	//cout<<bitset<32>(bit)<<endl<<endl;

	//int reg = regime();
	cout<<bitset<32>(bit)<<endl<<endl;
	return Posit(bit,mNbits,0,false);

}

bool Posit::eq(Posit& p)
{
    return mBits == p.mBits;
}

bool Posit::gt(Posit& p)
{
    if (isInf() || p.isInf()) {
        return false;
    }

    return mBits > p.mBits;
}

bool Posit::ge(Posit& p)
{
    return gt(p) || eq(p);
}

bool Posit::lt(Posit& p)
{
    return !gt(p) && !eq(p);
}

bool Posit::le(Posit& p)
{
    return !gt(p);
}

void Posit::set(float n)
{
    switch (fpclassify(n)) {
    case FP_INFINITE:
        mBits = POSIT_INF;
        mNan = false;
        break;
    case FP_NAN:
        mNan = true;
        break;
    case FP_ZERO:
        mBits = POSIT_ZERO;
        mNan = false;
        break;
    default:
        mBits = pack_posit(unpack_float(n), mNbits, mEs);
        mNan = false;
        break;
    }
}

void Posit::set(double n)
{
    switch (fpclassify(n)) {
    case FP_INFINITE:
        mBits = POSIT_INF;
        mNan = false;
        break;
    case FP_NAN:
        mNan = true;
        break;
    case FP_ZERO:
        mBits = POSIT_ZERO;
        mNan = false;
        break;
    default:
        mBits = pack_posit(unpack_double(n), mNbits, mEs);
        mNan = false;
        break;
    }
}

float Posit::getFloat()
{
    if (isZero()) {
        return 0.f;
    } else if (isInf()) {
        return 1.f / 0.f;
    } else if (isNan()) {
        return 0.f / 0.f;
    }

    return pack_float(unpack_posit(mBits, mNbits, mEs));
}

double Posit::getDouble()
{
    if (isZero()) {
        return 0.0;
    } else if (isInf()) {
        return 1.0 / 0.0;
    } else if (isNan()) {
        return 0.0 / 0.0;
    }

    return pack_double(unpack_posit(mBits, mNbits, mEs));
}

void Posit::setBits(POSIT_UTYPE bits)
{
    mBits = bits << (POSIT_SIZE - mNbits);
}

POSIT_UTYPE Posit::getBits()
{
    return mBits >> (POSIT_SIZE - mNbits);
}

void Posit::print()
{

    Posit p = isNeg() || isInf() ? neg() : *this;

    printf("{%d, %d} ", mNbits, mEs);

    for (int i = POSIT_SIZE - 1; i >= POSIT_SIZE - mNbits; i--) {
        printf("%d", (mBits >> i) & 1);
    }

    printf(" -> ");
    printf(isNeg() || isInf() ? "-" : "+");

    for (int i = POSIT_SIZE - ss() - 1; i >= POSIT_SIZE - mNbits; i--) {
        printf("%d", (p.mBits >> i) & 1);

        if (i != POSIT_SIZE - mNbits &&
            ((i == POSIT_SIZE - ss() - p.rs()) ||
             (i == POSIT_SIZE - ss() - p.rs() - mEs))) {
            printf(" ");
        }
    }

    printf(" = %lg\n", getDouble());

		cout<<bitset<32>(mBits)<<endl<<endl;

}


float Posit::subconv(){

	union {
        	float f;
        	uint32_t bits;
    	};


	bits=0;

	if( (mBits & 0xFFFFFFFF) == 0)
		return 0.0f;

	signed char c = (signed char)(regime());
	c+= 127;

	unsigned int esponente = (unsigned int)(c);					//esponente

	esponente = esponente<<23;

	bits = bits | esponente;

   	unpacked_t aup = unpack_posit(mBits, mNbits, mEs);

	unsigned int fr_ = aup.frac;

	fr_ = fr_>>9;

	bits = bits | fr_;

	//bits |= ((aup.neg)& 0x80000000);
	int segno = aup.neg;
	if(segno == 0)
		bits &= 0x7FFFFFFF;
	else
		bits |= 0x80000000;


	//print();

	//cout<<esponente<<endl;
	return f;


	
}

