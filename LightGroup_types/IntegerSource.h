#ifndef INTEGERSOURCE_H
#define INTEGERSOURCE_H

#include "LightSource.h"
#include "../utility/bitArray.h"


class IntegerSource : public LightSource
{
    public:
    // members shared by all
    static bitArray BA;
    static uint8_t digitData[21];// last digit is minus sign
    static int dRows, dCols;// 5 x 3
    static int dBits;// 15
    static void init_static();

    // regular members
    Light bgLt, fgLt;// background and fore ground colors

    bool setValue( int Value );
    bool incValue(){ return setValue( value + 1 ); }
    bool decValue(){ return setValue( value - 1 ); }

    virtual Light getLt( int r, int c )const;

    IntegerSource(){ rows = dRows; }
    virtual ~IntegerSource(){}

    protected:
    int value = 0;
    // assigned in setValue()
    int numDigits;// max = 6
    int digitVal[8];

    private:
};

#endif // INTEGERSOURCE_H
