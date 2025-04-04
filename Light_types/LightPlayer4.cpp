#include "LightPlayer4.h"

LightPlayer4::LightPlayer4()
{
    //ctor
}

LightPlayer4::~LightPlayer4()
{
    //dtor
}

void LightPlayer4::init( Light& r_Lt0, int Rows, int Cols, const patternData4& rPattData, unsigned int NumPatterns )
{
    pLt0 = &r_Lt0;
    rows = Rows;
    cols = Cols;
    numLts = rows*cols;

    stepTimer = 0;
    stepIter = 0;
    patternIter = 0;

    pattData = &rPattData;
    numPatterns = NumPatterns;
}

void LightPlayer4::setStateData( uint8_t* p_StateData, unsigned int DataSz )
{
    pStateData = p_StateData;
    stateDataSz = DataSz;
    BA.init( p_StateData[0], DataSz );
}

void LightPlayer4::update()// assign as desired
{
    for( unsigned int n = 0; n < numLts; ++n )
        *( pLt0 + n ) = Lt[ getState(n) ];

    if( ++stepTimer >= pattData[ patternIter ].stepPause )
    {
        stepTimer = 0;// to next step
        if( ++stepIter >= getPattLength() )
        {
            stepIter = 0;// to next pattern
            if( ++patternIter >= numPatterns )
                patternIter = 0;// reset cycle
        }
    }
}

void LightPlayer4::updateAfter()// writes only to onLt
{
    for( unsigned int n = 0; n < numLts; ++n )
    {
        uint8_t state = getState(n);
        if( state != 0 )
            *( pLt0 + n ) = Lt[ state ];
    }

    if( ++stepTimer >= pattData[ patternIter ].stepPause )
    {
        stepTimer = 0;// to next step
        if( ++stepIter >= getPattLength() )
        {
            stepIter = 0;// to next pattern
            if( ++patternIter >= numPatterns )
                patternIter = 0;// reset cycle
        }
    }
}

unsigned int LightPlayer4::getPattLength()const
{
    const unsigned int funcIdx = pattData[ patternIter ].funcIndex;

    if( funcIdx == 0 ) return 1;
    if( funcIdx >= 1 && funcIdx <= 5 ) return numLts;
    if( funcIdx == 6 ) return pattData[ patternIter ].param%100;// alternateBlink
    if( funcIdx == 7 ) return pattData[ patternIter ].param%100;// checkerBlink
    if( funcIdx == 10 || funcIdx == 11 ) return cols;// scrollCol
    if( funcIdx == 12 || funcIdx == 13 ) return rows;// scrollRow
    if( funcIdx == 14 || funcIdx == 15 ) return cols/2;// BoxIn, BoxOut
    if( funcIdx == 16 ) return rows + cols;// scrollDiagonal
    if( funcIdx == 17 ) return pattData[ patternIter ].param;// cWiseRing
 //   if( funcIdx == 100 ) return stateDataSz/numLts;// from pStateData[]
    if( funcIdx == 100 ) return BA.bitSize()/(2*numLts);// from pStateData[]
    return 1;
}

uint8_t LightPlayer4::getState( unsigned int n )const
{
    unsigned int funcIdx = pattData[ patternIter ].funcIndex;
    unsigned int param = pattData[ patternIter ].param;

    switch( funcIdx )
    {
        case 0 : return 0;// a "pause" between patterns
        case 1 : return scrollToRight( n, param );
        case 2 : return scrollToLeft( n, param );
        case 3 : return fillFromRight( n, param );
        case 4 : return fillFromLeft( n, param );
        case 5 : return crissCross( n, param );
        case 6 : return alternateBlink( n, param/100 );
        case 7 : return checkerBlink( n, param/100 );
        // 2d patterns
        case 17 : return cWiseRing( n, param );
        case 10 : return scrollColToRight( n, param );
        case 11 : return scrollColToLeft( n, param );
        case 12 : return scrollRowToBottom( n, param );
        case 13 : return scrollRowToTop( n, param );
        case 14 : return scrollBoxIn( n, param );
        case 15 : return scrollBoxOut( n, param );
        case 16 : return scrollDiagonal( n, param );

        case 100 :
        if( pStateData )
        {
            uint8_t retVal = BA.getBit( 2*( stepIter*numLts + n ) ) ? 1 : 0;
            retVal = retVal << 1;
            retVal += BA.getBit( 2*( stepIter*numLts + n ) + 1 ) ? 1 : 0;

            return retVal;
        }
        return 0;

        default: return 0;// offLight
    }

    return 0;// offLight
}

uint8_t LightPlayer4::scrollToRight( unsigned int n, unsigned int numInGroup )const// returns state assignment
{
    if( n >= stepIter && n < stepIter + numInGroup )
    {
        return 1 + ( stepIter - n )%3;// cycle colors
    }

    return 0;
}

uint8_t LightPlayer4::scrollToLeft( unsigned int n, unsigned int numInGroup )const// returns state assignment
{
    if( ( n <= numLts - 1 - stepIter ) && ( n + numInGroup > numLts - 1 - stepIter ) )
        return 1 + ( numLts - 1 - stepIter - n )%3;

    return 0;
}

uint8_t LightPlayer4::fillFromRight( unsigned int n, unsigned int LtIdx )const
{
    if( n >= numLts - 1 - stepIter ) return LtIdx;
    return 0;
}

uint8_t LightPlayer4::fillFromLeft( unsigned int n, unsigned int LtIdx )const
{
    if( n <= stepIter ) return LtIdx;
    return 0;
}

// numInGroup = numPlusLts%100. hiLt = numPlusLts/1000, loLt = numPlusLts/100
uint8_t LightPlayer4::crissCross( unsigned int n, unsigned int numPlusLts )const
{
    bool A = ( n >= stepIter && n < stepIter + numPlusLts%100 );
    bool B = ( n <= numLts - 1 - stepIter );
    B = B && ( n + numPlusLts%100 > numLts - 1 - stepIter );
    if( A || B ) return numPlusLts/1000;
    return ( numPlusLts/100 )%10;
}

// patternLength = param%100. LtPair = param/100. hiLt = LtPair/10, loLt = LtPair%10
uint8_t LightPlayer4::alternateBlink( unsigned int n, unsigned int LtPair )const
{
    if( ( n + stepIter )%2 == 1 ) return LtPair/10;
    return LtPair%10;
}

// patternLength = param%100. LtPair = param/100. hiLt = LtPair/10, loLt = LtPair%10
uint8_t LightPlayer4::checkerBlink( unsigned int n, unsigned int LtPair )const
{
    if( ( n + n/cols + stepIter )%2 == 1 ) return LtPair/10;
    return LtPair%10;
}

// case 10
uint8_t LightPlayer4::scrollColToRight( unsigned int n, unsigned int LtIdx )const
{
    if( stepIter == n%cols ) return LtIdx%4;
    return 0;
}

// case 11
uint8_t LightPlayer4::scrollColToLeft( unsigned int n, unsigned int LtIdx )const
{
    if( stepIter == ( cols - 1 - n%cols ) ) return LtIdx%4;
    return 0;
}
// case 12
uint8_t LightPlayer4::scrollRowToBottom( unsigned int n, unsigned int LtIdx )const
{
    if( stepIter == n/cols ) return LtIdx%4;
    return 0;
}
// case 13
uint8_t LightPlayer4::scrollRowToTop( unsigned int n, unsigned int LtIdx )const
{
    if( stepIter == rows - 1 - n/cols ) return LtIdx%4;
    return 0;
}

// case 17
uint8_t LightPlayer4::cWiseRing( int n, unsigned int LtPack )const
{
    int r = n/cols, c = n%cols;
    if( ( r > 0 && r < rows - 1 ) && ( c > 0 && c < cols - 1 ) ) return 0;

    return 1 + ( r + c + stepIter )%3;// 1st row
 //   return 0;
}

// case 14
uint8_t LightPlayer4::scrollBoxIn( unsigned int n, unsigned int LtIdx )const
{
    int Cmax = cols - 1 - stepIter;
    int Rmax = rows - 1 - stepIter;
    int r = n/cols, c = n%cols;

    if( ( r == (int)stepIter || r == Rmax ) && ( c >= (int)stepIter && c <= Cmax ) ) return LtIdx%4;
    if( ( c == (int)stepIter || c == Cmax ) && ( r >= (int)stepIter && r <= Rmax ) ) return LtIdx%4;
    return 0;
}

// case 15
uint8_t LightPlayer4::scrollBoxOut( unsigned int n, unsigned int LtIdx )const
{
    int Cmax = cols/2 + stepIter;
    int Cmin = cols - 1 - Cmax;
    int Rmax = rows/2 + stepIter;
    int Rmin = rows - 1 - Rmax;
    int r = n/cols, c = n%cols;

    if( ( r == Rmin || r == Rmax ) && ( c >= Cmin && c <= Cmax ) ) return LtIdx%4;
    if( ( c == Cmin || c == Cmax ) && ( r >= Rmin && r <= Rmax ) ) return LtIdx%4;
    return 0;
}

// case 16  Mode: 0 = dn rt, 1 = up lt, 2 = dn lt, 3 = up rt
uint8_t LightPlayer4::scrollDiagonal( unsigned int n, unsigned int Mode )const
{
    int r = n/cols, c = n%cols;
    if( Mode == 0 && (int)stepIter >= r && c == (int)stepIter - r ) return 1 + c%3;// dn rt
    if( Mode == 1 && c == ( cols - 2 - (int)stepIter + rows - r ) ) return 1 + c%3;// up lt
    if( Mode == 2 && c == ( cols - 1 - (int)stepIter + r ) ) return 1 + c%3;// dn lt
    if( Mode == 3 && c == ( (int)stepIter + r - rows + 1 ) ) return 1 + c%3;// up rt
    return 0;
}
