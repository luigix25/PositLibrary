#include "posit.h"
#include "util.h"
#include "pack.h"
#include "op2.h"

#include <cstdio>
#include <cmath>

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

int Posit::regime(unsigned int a =-1)
{
	if(a == -1)
		a = getBits();

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

	//cout<<"REGIME: "<<regime_bits<<endl;					//DEBUG
	
	if((regime_bits < 0) && (regime_bits +k >=0)){					// vi è un cambio segno nel regime
		//cout<<"CASO CON CAMBIO REGIME"<<endl;					DEBUG
		
		//considero i regimi negativi
		int eccesso = regime_bits +k;
		//cout<<"eccesso "<<eccesso<<endl;

		int quanti = -regime_bits;
		if(regime_bits == 0)// || regime_bits == -1)
			quanti = 2;
	
		//annullo il regime

		bit = bit<<1;					//elimino il segno;

		//for(int i=quanti;quanti >0;quanti--){	
		bit=bit<<(quanti+1);				//l'1 per il tappo						
		//}

		
		bit=bit>>2;						//inserisco lo 0
		bit |= 0x80000000;					//tappo per regime 0 
		//cout<<bitset<32>(bit)<<endl<<endl;				//DEBUG

							
		int bit_s = (signed int)bit;
		bit_s = bit_s >> (eccesso);

		bit = (unsigned int)bit_s;
				
		bit=bit>>1;							//segno

		//cout<<bitset<32>(bit)<<endl<<endl;				DEBUG

		int numero_zeri = eccesso+2-quanti-1;
		
		//cout<<"numero zeri "<<numero_zeri<<endl;			DEBUG



		if(sign == 0x80000000){						//se negaivo ne faccio not x +1
			bit *= -1; 						//nota: si potrebbe fare ^= 0xFFFF..F +1 
		}



		return Posit(bit,mNbits, 0, false);
	}	

	//cout<<"CASO GENERALE"<<endl;				//DEBUG

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



    return Posit(bit,mNbits, 0, false);

}

POSIT_UTYPE Posit::lfraction()
{
    return mBits << (ss() + rs() + mEs);
}


Posit Posit::div_p2(int k){				// funzionante al 100% da testare

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


	//caso in cui si annulla

	//cout<<"REGIME: "<<regime_bits<<endl;

	if((regime_bits > 0) && (regime_bits -k == 0)){					// si annulla il regime
		//CASO IN CUI SI ANNULLA		
		bit = bit<< regime_bits+1;
		bit = bit>>1;								//segno
		
		if(sign == 0x80000000){						//se negaivo ne faccio not x +1
				bit *= -1; 						//nota: si potrebbe fare ^= 0xFFFF..F +1 
		}
	
		return Posit(bit,mNbits,0,false);


	} else if((regime_bits > 0) && (regime_bits -k <0)){					// vi è un cambio segno nel regime
		bit = bit<<1;						//elimino il bit di segno;
	
		//cout<<"CASO CON CAMBIO "<<endl;

		//considero i regimi positivi
		int eccesso = k - regime_bits-1;
		int quanti = regime_bits;
		//moltiplico per quanti (annullo il regime)	NOTA: possibile usare mul_p2
		//cout<<"bit"<<endl;
		//cout<<bitset<32>(bit)<<endl<<endl;

		bit=bit<<quanti+1;						
		
		//cout<<bitset<32>(bit)<<endl<<endl;

		//bit=bit>>1;					//TAPPO 	


		bit |= 0x80000000;				//tappo
		//cout<<bitset<32>(bit)<<endl<<endl;
		bit=bit>>1;					//tappo	

		//cout<<"ECCESSO "<<eccesso<<endl;

		//cout<<bitset<32>(bit)<<endl<<endl;

		//regime portato a -1;

		if(eccesso == 0){
			bit = bit>>1;							//segno
			if(sign == 0x80000000){						//se negaivo ne faccio not x +1
				bit *= -1; 						//nota: si potrebbe fare ^= 0xFFFF..F +1 
			}
			
			return Posit(bit,mNbits,0,false); 
		}
		


		bit = bit>>(eccesso);					//

		bit=bit>>1;						//segno;

		//cout<<"ERRORE DI "<<-quanti+eccesso-1<<endl;
		
		int numero_zeri = -quanti+eccesso-1;

		
		if(sign == 0x80000000){						//se negaivo ne faccio not x +1
			bit *= -1; 						//nota: si potrebbe fare ^= 0xFFFF..F +1 
		}

		return Posit(bit,mNbits,0,false);

	}

	//cout<<"CASO GENERALE"<<endl;

	bit=bit<<1;				//elimino il segno;
	
	if(regime_bits>0){

		
		//for(int i=k;i >0;i--)	
		bit=bit<<k;
		
		bit=bit>>1;						//segno
		

	} else if(regime_bits == 0){
		

		bit = bit<<1;					//elimino il regime
		bit |= 0x80000000;				//tappo 1
		bit = bit >>k;					//tappo 2

		

		bit = bit>>1;					//segno
		//setBits(bit);
	
		

	} else {						//regime già negativo
		
		int bit_s = (signed int)bit;
		bit_s = bit_s >> k;

		bit = (unsigned int)bit_s;
		bit = bit>>1;					//segno
		
	}

	if(sign == 0x80000000){						//se negaivo ne faccio not x +1
		bit *= -1; 						//nota: si potrebbe fare ^= 0xFFFF..F +1 
	}


	//int reg = regime();
	//cout<<bitset<32>(bit)<<endl<<endl;
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

POSIT_UTYPE Posit::getBits() const
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

Posit::operator float(){					//posit a float

	return subconv();	
	
}

double Posit::subconv64(){

	union {
        	double d;
        	uint64_t bits;
    	};


	bits=0;

	if( (mBits & 0xFFFFFFFF) == 0)
		return 0.0f;

	int regime_bits = regime();

	int16_t espo16 = (uint16_t)regime_bits;
	espo16 = espo16 &(0x07FF);

	espo16+=1023;
	
	espo16 = espo16 &(0x07FF);			//elimino eventuale overflow

	uint64_t espo64 = espo16;
	
	
	espo64 = espo64 << (64-12);
	cout<<regime_bits<<endl;

	if(regime_bits < 0 ){
		regime_bits *=-1;
		regime_bits++;
	}
	else if(regime_bits > 0){
		regime_bits+=2;
	} else if(regime_bits == 0){
		regime_bits = 2;	
	}

	regime_bits++; 			//per il segno


	uint64_t mask = -1;


	mask = mask >>regime_bits;
	//cout<<bitset<64>(frac)<<endl;
	//print();

	uint64_t bit_veri = 0;

    	Posit p = isNeg() ? neg() : *this;


    	for (int i = POSIT_SIZE - 1; i >= POSIT_SIZE - mNbits; i--) {
		bit_veri = bit_veri | ((p.mBits >> i) & 1);

		if(i > POSIT_SIZE - p.mNbits)
			bit_veri = bit_veri << 1;
    	}


	uint64_t frazione = bit_veri & mask;
	frazione = frazione << regime_bits;
		

	frazione = frazione >> 12;

	bits = bits | espo64;
	
	cout<<bitset<64>(bits)<<endl;

	bits = bits | frazione;

	if(isNeg()){
		bits = bits | 0x8000000000000000;
	}
	
	return d;
}


Posit::operator double(){					//da posit a double

	
	return subconv64();
	
}

Posit Posit::add_zero(uint32_t a,uint32_t b){				//somma 0 bit esp

	if(a==0)
		return Posit(b,mNbits,0,false);
	if(b==0)
		return *this;

	int regime_a = regime(a);
	int regime_b = regime(b);
	unsigned int bits;

	unpacked_t a_up = unpack_posit(a, 32, 0);
	unpacked_t b_up = unpack_posit(b, 32, 0);

	unsigned int fr_a = a_up.frac;
	unsigned int fr_b = b_up.frac;

	unsigned int segno_a = (a & 0x80000000) >>31;
	unsigned int segno_b = (b & 0x80000000) >>31;

	int segno_f;
	if(segno_a == segno_b)
		segno_f = segno_a;

	uint64_t afrac = HIDDEN_BIT(fr_a);
	uint64_t bfrac = HIDDEN_BIT(fr_b);

	if(segno_a != 0){
		a *= -1;
	}

	if(segno_b != 0){
		b *= -1;
	}

	unsigned int regime;
	unsigned int mask = -1;
	int regime_q;

	int quanti = (regime_a == 0)?3:(regime_a > 0)?regime_a+3:-regime_a+2;
	mask = mask >> quanti;
	mask *=-1;

	regime = a & mask; 		//prendo il segno
	regime_q = regime_a;

	if(regime_a != regime_b){			//caso facile

		int r = regime_a - regime_b;

		if(regime_a > regime_b){ 			// a > b!
			bfrac = bfrac >>r;

			int quanti = (regime_a == 0)?3:(regime_a > 0)?regime_a+3:-regime_a+2;
			mask = mask >> quanti;
			mask *=-1;
			regime = a & mask; 		//prendo il segno
			regime_q = regime_a;


		} else{				// b > a!
			afrac = afrac >>(-r);
			int quanti = (regime_b == 0)?3:(regime_b > 0)?regime_b+3:-regime_b+2;
			mask = mask >> quanti;
			mask *=-1;
			regime = b & mask; 		//prendo il segno
			regime_q = regime_b;

		}

	}

	uint64_t fr_r;

	if(segno_a == segno_b){
		fr_r = afrac + bfrac;
	}


	if (fr_r >> POSIT_SIZE != 0) {			//normalizzazione
		//moltiplico per 2
		regime = crearegime(regime_q+1);
		regime_q++;
		fr_r >>= 1;
	}

	unsigned int mantissa = (unsigned int)(fr_r) << 1;		//butto via l'1 hidden

	int k = (regime_q == 0)?3:(regime_q > 0)?regime_q+3:-regime_q+2;

	mantissa = mantissa >> k;			//allineo con regime e segno;

	bits = regime | mantissa;

	if(segno_f != 0){
		bits *=-1;
	}

	return Posit(bits,mNbits,0,false);


}

Posit Posit::sub_zero(uint32_t a,uint32_t b){				//sottrazione 0 bit esp

	if(a==b)
		return Posit(0,mNbits,0,false);;

	if(a==0)
		return Posit(-b,mNbits,0,false);

	if(b==0)
		return *this;

	int regime_a = regime(a);
	int regime_b = regime(b);
	unsigned int bits;

	unpacked_t a_up = unpack_posit(a, 32, 0);
	unpacked_t b_up = unpack_posit(b, 32, 0);

	unsigned int fr_a = a_up.frac;
	unsigned int fr_b = b_up.frac;

	unsigned int segno_a = (a & 0x80000000) >>31;
	unsigned int segno_b = (b & 0x80000000) >>31;

	int segno_f;
	if(segno_a == segno_b)
		segno_f = segno_a;

	uint64_t afrac = HIDDEN_BIT(fr_a);
	uint64_t bfrac = HIDDEN_BIT(fr_b);

	if(segno_a != 0){
		a *= -1;
	}

	if(segno_b != 0){
		b *= -1;
	}

	int regime_r;
	unsigned int frac;
	unsigned int segno = 0;

	if (regime_a > regime_b || (regime_a == regime_b && fr_a > fr_b)) {
				regime_r = regime_a;
		        bfrac >>= regime_a - regime_b;
		        frac = afrac - bfrac;
	} else {
		segno = 1;
		regime_r = regime_b;
		afrac >>= -(regime_a - regime_b);
		frac = bfrac - afrac;
	}

	regime_r -= CLZ(frac);

	frac = frac << (CLZ(frac) + 1);

	unsigned int regime = crearegime(regime_r);
	int k = (regime_r == 0)?3:(regime_r > 0)?regime_r+3:-regime_r+2;

	frac = frac >> k;

	bits = regime | frac;

	if(segno == 1)
		bits *=-1;

	return Posit(bits,mNbits,0,false);

}

Posit Posit::operator+(const Posit& op){

	unsigned int a = this->getBits();
	unsigned int b = op.getBits();

	if(a==0 && b==0)
		Posit(0,mNbits,0,false);

	unsigned int segno_a = (a & 0x80000000) >>31;
	unsigned int segno_b = (b & 0x80000000) >>31;

	if(segno_a == segno_b)
		return add_zero(a,b);

	if(segno_a == 0 && segno_b ==1)			//a-b
		return sub_zero(a,-b);

	if(segno_a == 1 && segno_b ==0)			//b-a
		return sub_zero(b,-a);

	return Posit(0,mNbits,0,false);	//non ci arrivi mai;
}

Posit Posit::operator-(const Posit& op){

	unsigned int a = getBits();
	unsigned int b = op.getBits();

	if(a==0 && b==0)
		Posit(0,mNbits,0,false);

	unsigned int segno_a = (a & 0x80000000) >>31;
	unsigned int segno_b = (b & 0x80000000) >>31;

	if(segno_a == segno_b)					//a-b
		return sub_zero(a,b);

	if(segno_a == 0 && segno_b ==1)			//a-(-b)
		return add_zero(a,-b);

	if(segno_a == 1 && segno_b ==0)			//-a-b
		return add_zero(b,-a);

	return Posit(0,mNbits,0,false);			//non ci arrivi mai;
}


unsigned int Posit::crearegime(int n){

	unsigned int result = 0;

	if (n==0){
		return 0x40000000;
	}

	if(n>0){
		int result_tmp = 0x80000000;
		result_tmp = result_tmp >> (n);

		result = result_tmp;
		result = result >> 1;	//segno;

	} else{
		result = 0x80000000;
		result = result >> -n;
		result = result >> 1;		//segno;
	}

	return result;

}

Posit Posit::operator*(const Posit &op){

	unsigned int a = getBits();
	unsigned int b = op.getBits();

	if(a==0 || b==0)
		Posit(0,mNbits,0,false);;

	unpacked_t a_up = unpack_posit(a, 32, 0);
	unpacked_t b_up = unpack_posit(b, 32, 0);

	unsigned int fr_a = a_up.frac;
	unsigned int fr_b = b_up.frac;

	unsigned int segno_a = (a & 0x80000000) >>31;
	unsigned int segno_b = (b & 0x80000000) >>31;
	unsigned int segno_r = 0;

	if(segno_a == 1){
		a *= -1;
	}

	if(segno_b == 1){
		b *= -1;
	}

	uint64_t afrac = HIDDEN_BIT(fr_a);
	uint64_t bfrac = HIDDEN_BIT(fr_b);
	uint32_t frac = (afrac * bfrac) >> POSIT_SIZE;

	int regime_a = regime(a);
	int regime_b = regime(b);

	int regime_r = regime_a + regime_b + 1;

	if ((frac & POSIT_MSB) == 0) {
		regime_r--;
		frac <<= 1;
	}

	unsigned int frazione = frac<<1;

	segno_r = segno_a ^ segno_b;

	unsigned int regime = crearegime(regime_r);


	int quanti = (regime_r == 0)?3:(regime_r > 0)?regime_r+3:-regime_r+2;

	unsigned int bits = 0;

	frazione = frazione >> quanti;

	bits = regime | frazione;

	if(segno_r == 1)
		bits *= -1;

	return Posit(bits,mNbits,0,false);;

}

Posit Posit::operator/(const Posit &op){
	
	unsigned int a = getBits();
	unsigned int b = op.getBits();


	if(a==0)
		return Posit(0,mNbits,0,false);

	unpacked_t a_up = unpack_posit(a, 32, 0);
	unpacked_t b_up = unpack_posit(b, 32, 0);

	unsigned int fr_a = a_up.frac;
	unsigned int fr_b = b_up.frac;

	unsigned int segno_a = (a & 0x80000000) >>31;
	unsigned int segno_b = (b & 0x80000000) >>31;
	unsigned int segno_r = 0;

	if(segno_a == 1){
		a *= -1;
	}

	if(segno_b == 1){
		b *= -1;
	}

	uint64_t afrac = HIDDEN_BIT(fr_a);
	uint64_t bfrac = HIDDEN_BIT(fr_b);

	int regime_a = regime(a);
	int regime_b = regime(b);
	int regime_r = regime_a - regime_b;

    if (afrac < bfrac) {
    	regime_r--;
        bfrac >>= 1;
    }


    uint32_t frac = (afrac << POSIT_SIZE) / bfrac;

	segno_r = segno_a ^ segno_b;

	unsigned int regime = crearegime(regime_r);

	int quanti = (regime_r == 0)?3:(regime_r > 0)?regime_r+3:-regime_r+2;

	unsigned int bits = 0;

	frac = frac >> quanti;

	bits = regime | frac;

	if(segno_r == 1)
		bits *= -1;

	return Posit(bits,mNbits,0,false);


}

bool Posit::operator==(const Posit& op){

	unsigned int a = getBits();
	unsigned int b = op.getBits();
	return a==b;

}


ostream& operator<<(ostream& os, Posit& a){   		//col const da errori
   	os<<a.subconv();
    return os;
}

float Posit::sigmoid(){

	int bits = getBits();
	
	bits = bits ^ 0x80000000;
	bits = bits >> 2;
	//p.setBits(bits);
	my_float a;
	a.bits = bits;
	return a.decimale;


}