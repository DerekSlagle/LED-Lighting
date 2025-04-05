#include "CRGB_Player.h"

// define enough to test usage
void CRGB_Player::init( CRGB* p_Lt0, unsigned int NumLts )
{
    pLt0 = p_Lt0;
    numLts = NumLts;
    // test. What usage will compile?
    CRGB LtA;
    LtA.r = 1;
    LtA.g = 2;
    LtA.b = 3;
    CRGB LtB(4,5,6);// legal? If not assign members explicitly.
 //   CRGB LtB;
    LtB.r = 4;
    LtB.g = 5;
    LtB.b = 6;

    uint8_t x = 3;
    // operators defined?
    if( LtA == LtB ) x = 4;
    if( LtA != LtB ) x = 5;
}

void CRGB_Player::update( CRGB onLt, CRGB offLt )const
{
    const unsigned int k = 3;// fake condition
    for( unsigned int n = 0; n < numLts; ++n )
    {
        if( n == k ) *( pLt0 + n ) = onLt;
        else *( pLt0 + n ) = offLt;
    }
}
