#include "myLightMethods.h"

/*
void blinkGroup( myLight* pLt0, unsigned int numLts, float& tElap, float onTime, float offTime, float dt, float tRamp )
{
    if( pLt0 == nullptr || numLts == 0 ) return;
    if( tElap > 0.0f )
    {
        tElap -= dt;
        if( tElap <= 0.0f )// switch
        {
            tElap = 0.0f;
            if( pLt0->brightness == 0.0f )// switch on
            {
                tElap = onTime;
                for( unsigned int k = 0; k < numLts; ++k )
                    ( pLt0 + k )->brightness = 1.0f;

            }
            else// switch off
            {
                tElap = offTime;
                for( unsigned int k = 0; k < numLts; ++k )
                    ( pLt0 + k )->brightness = 0.0f;
            }
        }
    }
    else
        tElap = onTime;
}
*/

void blinkGroup( myLight* pLt0, unsigned int numLts, float& tElap, float onTime, float offTime, float dt,
            float tRamp, std::function<float(float)> Y )
{
    if( pLt0 == nullptr || numLts == 0 ) return;

    tElap += dt;// always on
    if( tElap >= onTime + offTime ) tElap = 0.0f;

    float tRel = tElap;
    float u = 1.0f, y = u;

    if( tElap < onTime )// turning on
    {
        if( tRel < tRamp )
            u = tRel/tRamp;
        else
            u = 1.0f;
    }
    else// turning off
    {
        tRel = tElap - onTime;

        if( tRel < tRamp )
            u = 1.0f - tRel/tRamp;
        else
            u = 0.0f;
    }

    if( tRel < tRamp )
    {
        if(Y) y = Y(u);
        else y = u*u*( 3.0f - 2.0f*u );// default: smooth ramp
        for( unsigned int k = 0; k < numLts; ++k )
            ( pLt0 + k )->brightness = y;
    }
    else if( tRel < tRamp + dt )// fully on or off
    {
        for( unsigned int k = 0; k < numLts; ++k )
            ( pLt0 + k )->brightness = u;
    }

 //   tElap += dt;// always on
 //   if( tElap >= onTime + offTime ) tElap = 0.0f;
}


void blinkGroup( lightGroup& LtGrp, float onTime, float offTime, float dt,
            float tRamp, std::function<float(float)> Y )
{
    if( LtGrp.pLt0 == nullptr ) return;
    blinkGroup( LtGrp.pLt0, LtGrp.cols*LtGrp.rows, LtGrp.tElap, onTime, offTime, dt, tRamp, Y );
}


// static: false if fully dimmed already
bool dimGroup( myLight* pLt0, unsigned int numLts, float& tElap, float rate, float dt )
{
    if( pLt0 == nullptr || numLts == 0 ) return false;
    if( pLt0->brightness == 0.0f ) return false;
    float y = 0.0f;

    tElap += dt;
    float u = 1.0f - tElap*rate;
    if( u < 0.0f )
    {
        for( unsigned int k = 0; k < numLts; ++k )
            ( pLt0 + k )->brightness = 0.0f;

        tElap = 0.0f;
        return false;
    }
    else
        y = u*u*( 3.0f - 2.0f*u );

    for( unsigned int k = 0; k < numLts; ++k )
        ( pLt0 + k )->brightness = y;

    return true;
}

bool dimGroup( lightGroup& LtGrp, float rate, float dt )
{
    return dimGroup( LtGrp.pLt0, LtGrp.rows*LtGrp.cols, LtGrp.tElap, rate, dt );
}

// static: false if fully bright already
bool brightenGroup( myLight* pLt0, unsigned int numLts, float& tElap, float rate, float dt )
{
    if( pLt0 == nullptr || numLts == 0 ) return false;
    if( pLt0->brightness == 1.0f ) return false;
    float y = 0.0f;

    tElap += dt;
    float u = tElap*rate;
    if( u > 1.0f )
    {
        for( unsigned int k = 0; k < numLts; ++k )
            ( pLt0 + k )->brightness = 1.0f;

        tElap = 0.0f;
        return false;
    }
    else
        y = u*u*( 3.0f - 2.0f*u );

    for( unsigned int k = 0; k < numLts; ++k )
        ( pLt0 + k )->brightness = y;

    return true;
}


bool brightenGroup( lightGroup& LtGrp, float rate, float dt )
{
    return brightenGroup( LtGrp.pLt0, LtGrp.rows*LtGrp.cols, LtGrp.tElap, rate, dt );
}

void shiftGroup( std::vector<myLight*>& pLtVec, float& tElap, float tShift, float dt )
{
    if( pLtVec.empty() ) return;
    const unsigned int numLts = pLtVec.size();

    if( tElap > 0.0f )
    {
        tElap -= dt;
        if( tElap <= 0.0f )// shift
        {
        //    std::cout << " shift";
            tElap = tShift;
            myLight Lt0 = *pLtVec[0];
            for( unsigned int j = 0; j + 1 < numLts; ++j )
                *( pLtVec[j] ) = *( pLtVec[j+1] );

            *( pLtVec[numLts-1] ) = Lt0;
        }
    }
    else
        tElap = tShift;
}

/*
void shiftGroup( myLight* pLt0, unsigned int numLts, float& tElap, float tShift, float dt )
{
    if( pLt0 == nullptr || numLts == 0 ) return;
    if( tElap > 0.0f )
    {
        tElap -= dt;
        if( tElap <= 0.0f )// shift
        {
       //     std::cout << "\n numLts: " << numLts;
            tElap = tShift;
            myLight Lt0 = *pLt0;
            for( unsigned int j = 0; j + 1 < numLts; ++j )
                *( pLt0 + j ) = *( pLt0 + j + 1 );

            *( pLt0 + numLts - 1 ) = Lt0;
        }
    }
    else
        tElap = tShift;// to get started?
}
*/

void shiftGroup( myLight* pLt0, unsigned int numLts, float& tElap, float tShift, float dt )
{
    if( pLt0 == nullptr || numLts == 0 ) return;

    char fwd = false;
    if( tShift < 0.0f )
    {
        tShift *= -1.0f;
        fwd = true;
    }

    if( tElap > 0.0f )
    {
        tElap -= dt;
        if( tElap <= 0.0f )// shift
        {
       //     std::cout << "\n numLts: " << numLts;
            tElap = tShift;
            myLight Lt0 = fwd ? *( pLt0 + numLts - 1 ) : *pLt0;

            if( fwd )
                for( unsigned int j = numLts - 1; j > 0; --j )
                    *( pLt0 + j ) = *( pLt0 + j - 1 );
            else
                for( unsigned int j = 0; j + 1 < numLts; ++j )
                    *( pLt0 + j ) = *( pLt0 + j + 1 );

            if( fwd ) *pLt0 = Lt0;
            else *( pLt0 + numLts - 1 ) = Lt0;
        }
    }
    else
        tElap = tShift;// to get started?
}

void shiftGroup( lightGroup& LtGrp, float dt )
{
    shiftGroup( LtGrp.pLt0, LtGrp.rows*LtGrp.cols, LtGrp.tElap, LtGrp.tLimit, dt );
}

// fills 1 by 1 across group. LtF "moves" across others in state Lt0. du = period between motion starts
bool fillAcrossGroup( std::vector<myLight*>& pLightVec, myLight Lt0, myLight LtF, float& tElap, float tTransit, float du, float dt, char Mode )
{
 //   float tFinal = tTransit + static_cast<float>( pLightVec.size() - 1 )*du;
    static bool isFilling = false;
    if( tElap == 0.0f ) isFilling = true;

    if( isFilling )
    {
        tElap += dt;
        const unsigned int numLts = pLightVec.size();
        // find how many are on
        unsigned int onCount = 0;
        for( unsigned int k = 0; k < numLts; ++k )
            if( *( pLightVec[k] ) == LtF ) ++onCount;

        if( onCount == numLts )
        {
            std::cout << "\n filled";
            isFilling = false;
            return false;
        }
        // turn all off
        for( unsigned int k = 0; k < numLts; ++k )
            *( pLightVec[k] ) = Lt0;

        // turn some back on
        for( unsigned int k = 0; k < numLts; ++k )
        {
            float u1 = tElap - k*du;
            if( u1 < 0.0f ) break;// it will only get lower as k increases

            float tTran2 = ( tTransit/numLts )*( numLts - onCount );// proportional to travel distance
            if( u1 > 0.0f && u1 <= tTran2 )
            {
                float u2 = u1/tTran2;
                u2 = u2*( 2.0f - u2 );// easing

            //    int n = ( u1/tTransit )*( numLts - 1 );
                int n = ( u2 )*( numLts - 1 );
                if( n >= 0 && n < (int)numLts )
                {
                    ++onCount;
                    *( pLightVec[n] ) = LtF;
                }
            }
            else if( u1 > tTran2 )// getting close?
                *( pLightVec[ numLts - 1 - k] ) = LtF;// this is almost right
        }
    }

    return isFilling;
}

// animate 1 light across group. u: 0 to 1. true when complete
bool moveAcrossGroup( myLight* pLt0, unsigned int numLts, float& tElap, float tShift,
        std::function<float(float)> Y, myLight LtOn, myLight LtOff, float dt )
{
    if( tElap < tShift )
    {
        // all off
        for( unsigned int k = 0; k < numLts; ++k )
            *( pLt0 + k ) = LtOff;

     //   unsigned int n = Y( tElap/tShift )*( numLts - 1 );
     //   *( pLt0 + n ) = LtOff;// off at last position
        tElap += dt;
        float u = tElap/tShift;
        int n = Y(u)*( numLts - 1 );
        if( n >= (int)(numLts) - 1 ) n = (int)(numLts) - 1;
        else if( n < 0 ) n = 0;

        if( tElap < tShift )
        {
            *( pLt0 + n ) = LtOn;// on at new position
            return false;
        }
        else
        {
            n = Y(1.0f)*( numLts - 1 );
            *( pLt0 + n ) = LtOn;// on at new position
            return true;
        }


        return tElap >= tShift;
    }

    return true;//
}

bool moveAcrossGroup( lightGroup& LtGrp, std::function<float(float)> Y, myLight LtOn, myLight LtOff, float dt )
{
    return moveAcrossGroup( LtGrp.pLt0, LtGrp.rows*LtGrp.cols, LtGrp.tElap, LtGrp.tLimit, Y, LtOn, LtOff, dt );
}

// instant response
// contiguous lights only
void vuMeter( myLight* pLt0, unsigned int numLts, myLight Lt0, myLight LtF, float u )
{
    if( !pLt0 || numLts < 2 ) return;// too few

    for( unsigned int k = 0; k < numLts; ++k ) *( pLt0 + k ) = Lt0;// reset all
    // clamp u
    if( u < 0.0f ) u = 0.0f;
    else if( u > 1.0f ) u = 1.0f;

    unsigned int iMax = numLts - 1;
    unsigned int iOn = u*iMax;
    if( iOn == 0 ) return;// zero input
    for( unsigned int k = 0; k <= iOn; ++k )
        *( pLt0 + k ) = LtF;
}

void vuMeter( std::vector<myLight*>& pLightVec, myLight Lt0, myLight LtF, float u )// u: 0.0f to 1.0f
{
    if( pLightVec.size() < 2 ) return;// too few

    const unsigned int numLts = pLightVec.size();
    for( unsigned int k = 0; k < numLts; ++k ) *( pLightVec[k] ) = Lt0;// reset all
    // clamp u
    if( u < 0.0f ) u = 0.0f;
    else if( u > 1.0f ) u = 1.0f;

    unsigned int iMax = numLts - 1;
    unsigned int iOn = u*iMax;
    if( iOn == 0 ) return;// zero input
    for( unsigned int k = 0; k <= iOn; ++k )
        *( pLightVec[k] ) = LtF;
}



void vuMeter( lightGroup& LtGrp, myLight Lt0, myLight LtF, float u )
{
    vuMeter( LtGrp.pLt0, LtGrp.rows*LtGrp.cols, Lt0, LtF, u );
}

// 3 color fill. LtF1 0 to uCut1, LtF2 uCut1 to uCut2, LtF3 u > uCut2
void vuMeter( myLight* pLt0, unsigned int numLts, myLight Lt0,
    myLight LtF1, float uCut1, myLight LtF2, float uCut2, myLight LtF3, float u )
{
    if( !pLt0 || numLts < 2 ) return;// too few

    for( unsigned int k = 0; k < numLts; ++k )
        *( pLt0 + k ) = Lt0;// reset all
    // clamp u
    if( u < 0.0f ) u = 0.0f;
    else if( u > 1.0f ) u = 1.0f;

    unsigned int iMax = numLts - 1;
    unsigned int iOn = u*iMax;
    unsigned int iOn1 = uCut1*iMax, iOn2 = uCut2*iMax;
    if( iOn == 0 ) return;// zero input
    for( unsigned int k = 0; k <= iOn; ++k )
    {
        if( k < iOn1 ) *( pLt0 + k ) = LtF1;
        else if( k < iOn2 ) *( pLt0 + k ) = LtF2;
        else *( pLt0 + k ) = LtF3;
    }
}

void vuMeter( std::vector<myLight*>& pLightVec, myLight Lt0,
    myLight LtF1, float uCut1, myLight LtF2, float uCut2, myLight LtF3, float u )
{
    if( pLightVec.size() < 2 ) return;// too few
    vuMeter( pLightVec[0], pLightVec.size(), Lt0, LtF1, uCut1, LtF2, uCut2, LtF3, u );
}

void vuMeter( lightGroup& LtGrp, myLight Lt0,
    myLight LtF1, float uCut1, myLight LtF2, float uCut2, myLight LtF3, float u )
{
    vuMeter( LtGrp.pLt0, LtGrp.rows*LtGrp.cols, Lt0, LtF1, uCut1, LtF2, uCut2, LtF3, u );
}

// for individual lights
myLight interpolate( const myLight& Lt0, const myLight& LtF, float u )
{
    float v = 1.0f - u;
    myLight retVal;
    retVal.rd = static_cast<unsigned int>( Lt0.rd*v + LtF.rd*u );
    retVal.gn = static_cast<unsigned int>( Lt0.gn*v + LtF.gn*u );
    retVal.bu = static_cast<unsigned int>( Lt0.bu*v + LtF.bu*u );
    retVal.brightness = Lt0.brightness*v + LtF.brightness*u;
    return retVal;
}

// easing animation. returns true when complete.
bool transit( const myLight& Lt0, const myLight& LtF, myLight& LtNow, float tElap, float tShift, float dt, char Mode )
{
    if( tElap < tShift )
    {
        float u = tElap/tShift;
        if( u > 1.0f ) u = 1.0f;// just to be sure?

        if( Mode == 'B' )
        {
            float y = u*u*( 3.0f - 2.0f*u );
            LtNow = interpolate( Lt0, LtF, y );
        }
        else// linear
            LtNow = interpolate( Lt0, LtF, u );

        return false;// not done yet
    }

    LtNow = LtF;
    return true;// done! / still done!
}

// varies only brightness. true when complete
bool pulse( myLight& LtNow, float tElap, float tShift, float dt, char Mode )
{
    if( tElap < tShift )
    {
        float u = tElap/tShift;
        if( u > 1.0f ) u = 1.0f;

        float y = 0.0f;
        if( Mode == 'P' )// wide pulse
        {
            y = 4.0f*u*( 1.0f - u );
        }
        else// narrow pulse
        {
            float uSq = u*u;
            y = uSq*uSq - 2.0f*uSq*u + uSq;
            y *= 16.0f;
         //   y = 1.0f;
        }

        if( y > 1.0f ) y = 1.0f;
        LtNow.brightness = y;
        return false;
    }

    LtNow.brightness = 0.0f;
    return true;
}

