#include "LightPlayer2.h"

void LightPlayer2::init( Light& r_Lt0, int Rows, int Cols, const patternData& rPattData, unsigned int NumPatterns )
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

    // default is entire grid
    gridRows = rows;
    gridCols = cols;
    row0 = col0 = 0;
    drawMode = 1;
}

void LightPlayer2::bindToGrid( Light& r_Lt0, int GridRows, int GridCols )
{
    pLt0 = &r_Lt0;
    setGridBounds( row0, col0, GridRows, GridCols );
}

void LightPlayer2::setDrawMode()
{
    if( rows == gridRows && cols == gridCols && row0 == 0 && col0 == 0 )
        drawMode = 1;// is grid
    else if( ( row0 >= 0 && row0 + rows <= gridRows ) && ( col0 >= 0 && col0 + cols <= gridCols ) )
        drawMode = 2;// is all in grid
    else
        drawMode = 3;// is partly in grid
}

void LightPlayer2::firePattern( unsigned int pattIdx )
{
    if( pattIdx >= numPatterns ) return;
    patternIter = pattIdx;
    stepIter = stepTimer = 0;
}

void LightPlayer2::setToPlaySinglePattern( bool playSingle )
{
    playSinglePattern = playSingle;
    if( playSingle )
    {
        patternIter = 0;
        stepIter = getPattLength();// so update() returns
    }
    else
    {
        stepIter = stepTimer = 0;
    }
}

void LightPlayer2::takeStep()
{
    if( ++stepTimer >= pattData[ patternIter ].stepPause )
    {
        stepTimer = 0;// to next step
        if( ++stepIter >= getPattLength() )
        {
            if( playSinglePattern ) return;// reset stepIter to replay pattern
            stepIter = 0;// to next pattern
            if( ++patternIter >= numPatterns && doRepeatSeq )
                patternIter = 0;// reset cycle
        }
    }
}

void LightPlayer2::update()// assign as required
{
    if( patternIter >= numPatterns ) return;
    if( playSinglePattern && stepIter >= getPattLength() ) return;

    if( drawMode == 1 )
    {
        if( drawOffLt ) updateIsGrid();
        else updateIsGridOnOnly();
    }
    else if( drawMode == 2 || drawMode == 3 )
    {
        if( drawOffLt ) updateSub();
        else updateSubOnOnly();
    }
    else updateSub();// default

    takeStep();
}

void LightPlayer2::updateIsGrid()// assign as desired
{
    for( unsigned int n = 0; n < numLts; ++n )
    {
        if( getState(n) ) *( pLt0 + n ) = onLt;
        else *( pLt0 + n ) = offLt;
    }

 //   takeStep();
}

void LightPlayer2::updateIsGridOnOnly()// for drawing after another player
{
    for( unsigned int n = 0; n < numLts; ++n )
        if( getState(n) ) *( pLt0 + n ) = onLt;

 //   takeStep();
}

void LightPlayer2::updateSub()
{
    Light* pBase = pLt0 + gridCols*row0 + col0;
 //   if( col0 < 0 ) std::cout << "\n top: col0 < 0";

    for( int r = 0; r < rows; ++r )
    {
        if( r + row0 < 0 ) continue;
        if( r + row0 >= gridRows ) break;

        Light* pRow = pBase + r*gridCols;
        for( int c = 0; c < cols; ++c )
        {
            if( c + col0 < 0 ) continue;
            if( c + col0 >= gridCols ) break;
            if( getState( r*cols + c ) ) *( pRow + c ) = onLt;
            else *( pRow + c ) = offLt;
        }
    }

 //   takeStep();
}

void LightPlayer2::updateSubOnOnly()// writes only to onLt
{
    Light* pBase = pLt0 + gridCols*row0 + col0;
    for( int r = 0; r < rows; ++r )
    {
        if( r + row0 < 0 ) continue;
        if( r + row0 >= gridRows ) break;

        Light* pRow = pBase + r*gridCols;
        for( int c = 0; c < cols; ++c )
        {
            if( c + col0 < 0 ) continue;
            if( c + col0 >= gridCols ) break;
            if( getState( r*cols + c ) )
                *( pRow + c ) = onLt;
        }
    }

//   takeStep();
}

void LightPlayer2::update_toDoubleGrid()
{
    if( patternIter >= numPatterns ) return;
    if( playSinglePattern && stepIter >= getPattLength() ) return;

    // classify for draw
    if( col0 < gridCols )// to 1st grid
    {
        if( col0 + cols <= gridCols )// entirely in 1st grid
        {
            update();// normally
            return;
        }
        // draw is to both panels        
        update_toSplitGrid();
        
    }
    else// 2nd panel only
    {
        pLt0 += gridRows*gridCols;
        col0 -= gridCols;
        update();// normally
        pLt0 -= gridRows*gridCols;
        col0 += gridCols;
        return;
    }

    takeStep();
}

void LightPlayer2::update_toSplitGrid()// called in update_toDoubleGrid() to handle split case
{
    // draw to 1st panel
    Light* pBase = pLt0 + row0*gridCols + col0;
    for( int r = 0; r < rows; ++r )
    {
        if( r + row0 < 0 ) continue;
        if( r + row0 >= gridRows ) break;

        Light* pRow = pBase + r*gridCols;
        for( int c = 0; c < gridCols - col0; ++c )
        {
            if( c + col0 < 0 ) continue;
            if( c + col0 >= gridCols ) break;

            if( getState( r*cols + c ) )
                *( pRow + c ) = onLt;
            else if( drawOffLt )
                *( pRow + c ) = offLt;
        }
    }

    // draw to 2nd panel
    pBase += gridRows*gridCols - col0;
    for( int r = 0; r < rows; ++r )
    {
        if( r + row0 < 0 ) continue;
        if( r + row0 >= gridRows ) break;

        Light* pRow = pBase + r*gridCols;
        for( int c = gridCols - col0; c < cols; ++c )
        {
            int c2 = c + col0 - gridCols;// starts from 0
            if( c2 >= gridCols ) break;
      //      if( c + col0 < 0 ) continue;// unsure here
      //      if( c + col0 >= gridCols ) break;

            if( getState( r*cols + c ) )
                *( pRow + c2 ) = onLt;
            else if( drawOffLt )
                *( pRow + c2 ) = offLt;
        }
    }
}

uint16_t LightPlayer2::getPattLength()const
{
    const uint8_t funcIdx = pattData[ patternIter ].funcIndex;

    if( funcIdx == 0 ) return 1;// pause pattern
    if( funcIdx >= 1 && funcIdx <= 5 ) return rows*cols;
    if( funcIdx == 8 || funcIdx == 9 ) return rows*cols;// emptyFromRight, Left
    if( funcIdx == 6 ) return (uint16_t)pattData[ patternIter ].param;// alternateBlink
    if( funcIdx == 7 ) return (uint16_t)pattData[ patternIter ].param;// checkerBlink
    if( funcIdx == 10 || funcIdx == 11 ) return cols;// scrollCol
    if( funcIdx == 12 || funcIdx == 13 ) return rows;// scrollRow
    if( funcIdx == 14 || funcIdx == 15 ) return cols/2;// BoxIn, BoxOut
    if( funcIdx == 16 ) return rows + cols;// scrollDiagonal

    if( funcIdx == 80 ) return static_cast<uint16_t>( ( cols + rows )/4 );// scrollRingOut
    // user defined
    if( funcIdx == 100 && getPatt100Length ) return getPatt100Length();// scrollDiagonal

    return 1;
}

bool LightPlayer2::getState( unsigned int n )const
{
    uint8_t funcIdx = pattData[ patternIter ].funcIndex;
    uint8_t param = pattData[ patternIter ].param;

    switch( funcIdx )
    {
        case 0 : return false;// a "pause" between patterns
        case 1 : return scrollToRight( n, param );
        case 2 : return scrollToLeft( n, param );
        case 3 : return fillFromRight( n );
        case 4 : return fillFromLeft( n );
        case 5 : return crissCross( n, param );
        case 6 : return alternateBlink( n );
        case 7 : return checkerBlink( n );
        case 8 : return emptyFromRight( n );
        case 9 : return emptyFromLeft( n );
        // 2d patterns
        case 10 : return scrollColToRight( n );
        case 11 : return scrollColToLeft( n );
        case 12 : return scrollRowToBottom( n );
        case 13 : return scrollRowToTop( n );
        case 14 : return scrollBoxIn( n );
        case 15 : return scrollBoxOut( n );
        case 16 : return scrollDiagonal( n, param );
        case 80 : return scrollRingOut( n );
        // user defined
        case 100 : if( getState100 ) return getState100( n, param );

        default: return false;// offLight
    }

    return false;// offLight
}

bool LightPlayer2::scrollToRight( unsigned int n, unsigned int numInGroup )const// returns state assignment
{ return ( n >= stepIter && n < stepIter + numInGroup ); }

bool LightPlayer2::scrollToLeft( unsigned int n, unsigned int numInGroup )const// returns state assignment
{ return ( n <= numLts - 1 - stepIter ) && ( n + numInGroup > numLts - 1 - stepIter ); }

bool LightPlayer2::fillFromRight( unsigned int n )const
{ return ( n >= numLts - 1 - stepIter ); }

bool LightPlayer2::fillFromLeft( unsigned int n )const
{ return ( n <= stepIter ); }

bool LightPlayer2::crissCross( unsigned int n, unsigned int numInGroup )const
{
    bool A = ( n >= stepIter && n < stepIter + numInGroup );
    bool B = ( n <= numLts - 1 - stepIter );
    B = B && ( n + numInGroup > numLts - 1 - stepIter );
    return A || B;
}

bool LightPlayer2::alternateBlink( unsigned int n )const
{ return ( n + stepIter )%2; }

bool LightPlayer2::checkerBlink( unsigned int n )const
{ return ( n + n/cols + stepIter )%2; }

// patterns for 2d
bool LightPlayer2::scrollColToRight( unsigned int n )const
{ return stepIter == n%cols; }

bool LightPlayer2::scrollColToLeft( unsigned int n )const
{ return stepIter == ( cols - 1 - n%cols ); }

bool LightPlayer2::scrollRowToBottom( unsigned int n )const
{ return stepIter == n/cols; }

bool LightPlayer2::scrollRowToTop( unsigned int n )const
{ return stepIter == rows - 1 - n/cols; }

bool LightPlayer2::scrollBoxIn( unsigned int n )const
{
    int Cmax = cols - 1 - stepIter;
    int Rmax = rows - 1 - stepIter;
    int r = n/cols, c = n%cols;

    if( ( r == (int)stepIter || r == Rmax ) && ( c >= (int)stepIter && c <= Cmax ) ) return true;
    if( ( c == (int)stepIter || c == Cmax ) && ( r >= (int)stepIter && r <= Rmax ) ) return true;
    return false;
}

bool LightPlayer2::scrollBoxOut( unsigned int n )const
{
    int Cmax = cols/2 + stepIter;
    int Cmin = cols - 1 - Cmax;
    int Rmax = rows/2 + stepIter;
    int Rmin = rows - 1 - Rmax;
    int r = n/cols, c = n%cols;

    if( ( r == Rmin || r == Rmax ) && ( c >= Cmin && c <= Cmax ) ) return true;
    if( ( c == Cmin || c == Cmax ) && ( r >= Rmin && r <= Rmax ) ) return true;
    return false;
}

// 0 = dn rt, 1 = up lt, 2 = dn lt, 3 = up rt
bool LightPlayer2::scrollDiagonal( unsigned int n, unsigned int Mode )const// 16
{
    int r = n/cols, c = n%cols;
    if( Mode == 0 && (int)stepIter >= r ) return c == (int)stepIter - r;// dn rt
    if( Mode == 1 ) return c == cols - 1 - (int)stepIter + rows - 1 - r;// up lt
    if( Mode == 2 ) return c == cols - 1 - (int)stepIter + r;// dn lt
    if( Mode == 3 ) return c == (int)stepIter + r - rows - 1;// up rt

    return false;
}

bool LightPlayer2::scrollRingOut( unsigned int n )const// 80
{
//    float RmaxSq = ( cols*cols + rows*rows )*0.25f;
    int r = n/cols, c = n%cols;
    const unsigned int& Param = pattData[ patternIter ].param;

 //   float Ry = ( rows - 1 + stepIter - r ), Rx = ( cols - 1 + stepIter - c );
    float Ry = ( rows/2 - r ), Rx = ( cols/2 - c );
    float RnSq = ( Rx*Rx + Ry*Ry )*0.25f;
 //   if( RnSq > RmaxSq ) return false;// radius too large
    if( RnSq >= ( stepIter )*( stepIter ) && RnSq < ( stepIter + Param )*( stepIter + Param ) )
        return true;

    return false;
}

// alternate display
void LightPlayer2::updateAsEq( float* pVal )const// cols elements is assumed
{
    int numOn = 0;
    Light* pBase = pLt0 + gridCols*row0 + col0;

    for( int c = 0; c < cols; ++c )
    {
        numOn = pVal[c]*( rows );
        if( numOn < 0 ) numOn *= -1;// amplitude only
        if( numOn > rows ) numOn = rows;// limit

        Light* pLt = pBase + ( rows - 1 )*gridCols + c;// start at bottom of column
        for( int n = 0; n < numOn; ++n )
        {
            *pLt = onLt;
            pLt -= gridCols;// up 1 row
        }
        // fraction of a light?
        if( pLt >= pBase )
        {
       //     float valPerRow = pVal[c]/rows;
            float X = pVal[c]*rows - numOn;// fraction remaining
            Light lastLt = onLt;
            lastLt.r = X*onLt.r + ( 1.0f - X )*offLt.r;
            lastLt.g = X*onLt.g + ( 1.0f - X )*offLt.g;
            lastLt.b = X*onLt.b + ( 1.0f - X )*offLt.b;
            *pLt = lastLt;
            pLt -= gridCols;// up 1 row
        }
        // remaining rows upward if offLt drawn
        while( drawOffLt && pLt >= pBase )
        {
            *pLt = offLt;
            pLt -= gridCols;// up 1 row
        }
    }
}
