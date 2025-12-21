#include "IntegerSource.h"

// static members
bitArray IntegerSource::BA;
int IntegerSource::dRows = 5, IntegerSource::dCols = 3;
int IntegerSource::dBits = 15;
uint8_t IntegerSource::digitData[21];

void IntegerSource::init_static()
{
    uint8_t* X = digitData;
    X[0] = 111; X[1] = 251; X[2] = 73; X[3] = 250; X[4] = 249;
    X[5] = 252; X[6] = 60; X[7] = 223; X[8] = 158; X[9] = 124;
    X[10] = 206; X[11] = 39; X[12] = 239; X[13] = 79; X[14] = 146;
    X[15] = 239; X[16] = 251; X[17] = 247; X[18] = 36; X[19] = 112;
    X[20] = 0;
    BA.init( digitData[0], 21 );
    BA.sizeBits = 165;// 11 frames at 15 bits each
}

// regular methods
// includes 1 column between digits
Light IntegerSource::getLt( int r, int c )const
{
    if( r >= 0 && c >= 0 )// upper bound are rows and cols
    {
        int dgtSpace = c/( dCols + 1 );
        c %= dCols + 1;// remainder
        if( c == dCols ) return bgLt;// column between digits

        if( value < 0 )
        {
            if( dgtSpace == 0 )// render minus sign
                return BA.getBit( dBits*10 + r*dCols + c ) ? fgLt : bgLt;
            // else reduce dgtSpace;
            --dgtSpace;// 1st is for minus sign
        }

        // it is a digit space
        int dgtIdx = numDigits - 1 - dgtSpace;
        bool st = BA.getBit( dBits*digitVal[dgtIdx] + r*dCols + c );
        return st ? fgLt : bgLt;
    }

    return Light();
}

bool IntegerSource::setValue( int Value )
{
    int lastCols = cols;
    value = Value;// save
    // working Value to be consumed
    if( Value < 0 ) Value *= -1;// positive
    // ones digit
    digitVal[0] = Value%10;
    numDigits = 1;
    // the rest
    while( Value > 9 && numDigits < 6 )
    {
        Value /= 10;
        digitVal[numDigits] = Value%10;
        ++numDigits;
    }

  //  cols = numDigits*dCols;/// no space between digits
    cols = numDigits*( 1 + dCols ) - 1;// 1 space between digits
    if( value < 0 ) cols += 1 + dCols;// for minus sign
    // return based on change in cols
    return cols < lastCols;// true if area drawn to is reduced
}

bool IntegerSource::update( float dt )
{
    return false;
}
