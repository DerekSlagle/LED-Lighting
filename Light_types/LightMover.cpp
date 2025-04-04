#include "LightMover.h"

LightMover::LightMover()
{
    //ctor
}

LightMover::~LightMover()
{
    //dtor
}

void LightMover::init( Light& rLt0, unsigned int NumLts )
{
    pLt0 = &rLt0;
    numLts = NumLts;
    funcIdx = 0;
    tElap = 0.0f;
}

// animate position
void LightMover::update( float dt )
{
    if( tElap < tLimit )
    {
        tElap += dt;
        if( tElap < 0.0f ) return;// waiting to go

        if( doRepeat && tElap >= tLimit ) tElap = 0.0f;
        *( pLt0 + getOnIndex( tElap/tLimit ) ) = onLt;
    }
}

void LightMover::update( Light offLt, float dt )
{
    for( unsigned int n = 0; n < numLts; ++n )
        *( pLt0 + n ) = offLt;

    update(dt);
}

// assign position
void LightMover::moveTo( float u )const// u: 0 to 1
{
    *( pLt0 + getOnIndex(u) ) = onLt;
}

void LightMover::moveTo( Light offLt, float u )const// u: 0 to 1
{
    for( unsigned int n = 0; n < numLts; ++n )
        *( pLt0 + n ) = offLt;

    moveTo(u);
}

void LightMover::Clear( Light offLt )const
{
    for( unsigned int n = 0; n < numLts; ++n )
        *( pLt0 + n ) = offLt;
}

int LightMover::getOnIndex( float u )const
{
    if( u < 0.0f ) return 0;
    if( u > 1.0f ) return numLts - 1;
    float y = u;
    switch( funcIdx )
    {
        // linear
        case 1 :
        y = u;
        break;
        // quadratic
        case 20 :
        y = u*u;// y(0)=0, yp(0)=0, y(1)=1, yp(1)=2
        break;
        case 21 :
        y = 4.0f*u*(1.0f-u);// y(0)=0, y(1/2)=1, y(1)=0
        break;
        // cubic
        case 30 :
        y = u*u*u;// y(0)=0, yp(0)=0, ypp(0)=0, y(1)=1
        break;
        case 31 :// yp(0)=0, yp(1)=0
        y = u*u*( 3.0f - 2.0f*u );//  y(0)=0, yp(0)=0, y(1)=1, yp(1)=0
        break;
        // quartic
        case 41 :// yp(0)=0, yp(1/2)=0 yp(1)=0
        y = 16.0f*u*u*(1.0f-u)*(1.0f-u);//  y(0)=0, yp(0)=0, y(1)=0, yp(1)=0
        break;

        default: return 0;
    }

    int retVal = y*(float)( numLts );
    if( retVal < 0 ) return 0;
    if( retVal > numLts - 1 ) return numLts - 1;
 //   return y*(float)( numLts - 1 );
    return retVal;
}
