#include "CRGB_Player.h"

// define enough to test usage
void CRGB_Player::init( CRGB* p_Lt0, unsigned int NumLts )
{
    pLt0 = p_Lt0;
    numLts = NumLts;
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
