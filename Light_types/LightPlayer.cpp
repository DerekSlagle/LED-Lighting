#include "LightPlayer.h"

LightPlayer::LightPlayer()
{
    //ctor
}

LightPlayer::~LightPlayer()
{
    //dtor
}

void LightPlayer::init( Light& r_Lt0, unsigned int Rows, unsigned int Cols )
{
    pLt0 = &r_Lt0;
    rows = Rows;
    cols = Cols;
    numLts = rows*cols;

    numStates = 5;// 5 active
//    iterMax = numStates*numLts*numLts + numBlinks*blinkPause*numLts + numLts*endPause;
    iterMax = numLts*( numStates*numLts + numBlinks*blinkPause + endPause );
    iter = 0;
}

void LightPlayer::update( const Light& onLt, const Light& offLt )
{
    for( unsigned int n = 0; n < numLts; ++n )
    {
        if( getState() ) *( pLt0 + n ) = onLt;
        else *( pLt0 + n ) = offLt;

        ++iter;
    }

    if( iter >= iterMax ) iter = 0;// restart pattern
}

/*
bool LightPlayer::getState()const
{
//    unsigned int step = iter/numLts;
    unsigned int step = iter/numLts;
    unsigned int n = iter%numLts;

 //   return k == step;


    // across from left
//    if( step < numLts ) return k == step;
    if( step < numLts ) return ( n >= step && n < step + k1 );
    step -= numLts;

    // across from right
 //   if( step < numLts ) return k == numLts - 1 - step;
    if( step < numLts ) return ( n <= numLts - 1 - step ) && ( n + k2 > numLts - 1 - step );
    step -= numLts;

    // fill from left
    if( step < numLts ) return n <= step;
    step -= numLts;

    // criss cross k1 each way
    if( step < numLts )
    {
    //    bool A = (k == step);
    //    bool B = (k == numLts - 1 - step);
        bool A = ( n >= step && n < step + k1 );
        bool B = ( n <= numLts - 1 - step);
        B = B && ( n + k1 > numLts - 1 - step );

        return A || B;
    }
    step -= numLts;

    // fill from right
    if( step < numLts ) return n >= numLts - 1 - step;
    step -= numLts;

    // alternate blink numBlinks times
    if( step < numBlinks*blinkPause ) return ( n + step/blinkPause )%2;
    step -= numBlinks*blinkPause;

    // leftover = delay before restart
    return false;

}
*/

bool LightPlayer::getState()const
{
    unsigned int step = iter/numLts;
    unsigned int n = iter%numLts;
    bool retVal = false;// written to by functions

    // across from left
    if( scrollFromSide( 'L', retVal, step, k1 ) )// true if step in range 0 to numSteps
        return retVal;

    // across from right
    if( scrollFromSide( 'R', retVal, step, k2 ) )// true if step in range 0 to numSteps
        return retVal;

    // fill from left
    if( fillFromSide( 'L', retVal, step ) )// true if step in range 0 to numSteps
        return retVal;

    // criss cross k1 each way
    if( crissCross( retVal, step, k1 ) )// true if step in range 0 to numSteps
        return retVal;

    // fill from right
    if( fillFromSide( 'R', retVal, step ) )// true if step in range 0 to numSteps
        return retVal;

    // alternate blink numBlinks times
    if( alternateBlink( numBlinks, blinkPause, retVal, step ) )
        return retVal;

    // leftover = delay before restart
    return false;

}


bool LightPlayer::scrollRight( bool& state, unsigned int& step , unsigned int numInGroup )const//
{
    unsigned int n = iter%numLts;
    if( step < numLts )
    {
        state = ( n >= step && n < step + numInGroup );
        return true;
    }
    step -= numLts;// deduct my number
    return false;// # of steps in pattern
}

bool LightPlayer::scrollLeft( bool& state, unsigned int& step , unsigned int numInGroup )const//
{
    unsigned int n = iter%numLts;
    if( step < numLts )
    {
        state = ( n <= numLts - 1 - step ) && ( n + numInGroup > numLts - 1 - step );
        return true;
    }
    step -= numLts;// deduct my number
    return false;// # of steps in pattern
}

bool LightPlayer::scrollFromSide( char Side, bool& state, unsigned int& step , unsigned int numInGroup )const
{
    unsigned int n = iter%numLts;
    if( step < numLts )
    {
        if( Side == 'R' )
            state = ( n <= numLts - 1 - step ) && ( n + numInGroup > numLts - 1 - step );
        else
            state = ( n >= step && n < step + numInGroup );

        return true;
    }
    step -= numLts;// deduct my number
    return false;// # of steps in pattern
}

bool LightPlayer::crissCross( bool& state, unsigned int& step , unsigned int numInGroup )const//
{
    unsigned int n = iter%numLts;
    if( step < numLts )
    {
        bool A = ( n >= step && n < step + numInGroup );
        bool B = ( n <= numLts - 1 - step);
        B = B && ( n + numInGroup > numLts - 1 - step );
        state = A || B;
        return true;
    }
    step -= numLts;// deduct my number
    return false;// # of steps in pattern
}

bool LightPlayer::fillFromSide( char Side, bool& state, unsigned int& step )const
{
    unsigned int n = iter%numLts;
    if( step < numLts )
    {
        if( Side == 'L' )
            state = ( n <= step );
        else// Side == 'R' ?
            state = ( n >= numLts - 1 - step );

        return true;
    }
    step -= numLts;// deduct my number
    return false;// # of steps in pattern
}

bool LightPlayer::alternateBlink( unsigned int NumBlinks, unsigned int BlinkPause, bool& state, unsigned int& step )const
{
    unsigned int n = iter%numLts;
    if( step < numBlinks*blinkPause )
    {
        state = ( n + step/blinkPause )%2;
        return true;
    }
    step -= numBlinks*blinkPause;// deduct my number
    return false;// # of steps in pattern
}
