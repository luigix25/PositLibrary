#ifndef __POSIT_H
#define __POSIT_H

#include "posit_types.h"
#include <iostream>
using namespace std;



class Posit {
private:
    POSIT_UTYPE mBits;
    int mNbits;
    int mEs;
    bool mNan;

    Posit(POSIT_UTYPE bits, int nbits, int es, bool nan);
    Posit(int nbits, int es, bool nan);
    Posit add_zero(uint32_t a,uint32_t b);
    Posit sub_zero(uint32_t a,uint32_t b);
    int regime(unsigned int a);
    unsigned int crearegime(int n);
    POSIT_UTYPE lfraction();


    union my_float{
        float decimale;
        int bits;
    };

public:
    Posit(int nbits, int es);

    bool isZero();                  // check for 0
    bool isOne();                   // check for +/- 1
    bool isInf();                   // check for +/- infinity
    bool isNeg();                   // check for negative
    bool isNan();                   // check for NaN

    int nbits();                    // size in bits
    int ss();                       // sign size in bits
    int rs();                       // regime size in bits
    int es();                       // exponent size in bits
    int fs();                       // fraction size in bits

    Posit zero();                   // 0
    Posit one();                    // 1
    Posit inf();                    // +/-inf
    Posit nan();                    // NaN

    Posit neg();                    // -x
    Posit rec();                    // /x
	
    float subconv();                //da posit a float 0 bit esp
    double subconv64();             //da posit a double 0 bit esp

    Posit add(Posit& p);            // x + p
    Posit sub(Posit& p);            // x - p as x + -p
    Posit mul(Posit& p);            // x * p
    Posit div(Posit& p);            // x / p as x * /p
    float sigmoid();            // x / p as x * /p

    Posit operator+(const Posit&op);
    Posit operator-(const Posit&op);
    Posit operator*(const Posit&op);
    Posit operator/(const Posit&op);
    bool operator==(const Posit&op);
    friend ostream& operator<<(ostream&, Posit&);

    operator float();
    operator double();

    Posit mul_p2(int);              //mol potenze del 2
    Posit div_p2(int);              //div potenze del 2

    bool eq(Posit& p);              // x == p
    bool gt(Posit& p);              // x > p
    bool ge(Posit& p);              // x >= p
    bool lt(Posit& p);              // x < p
    bool le(Posit& p);              // x <= p

    void set(float n);              // x = n
    void set(double n);             // x = n

    float getFloat();               // n = x
    double getDouble();             // n = x

    // debug
    void setBits(POSIT_UTYPE bits);
    POSIT_UTYPE getBits() const;
    void print();
};

#endif
