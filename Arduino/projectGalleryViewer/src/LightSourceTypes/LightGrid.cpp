#include "LightGrid.h"

LightGrid::LightGrid()
{
    //ctor
}

LightGrid::~LightGrid()
{
    if( pLt0 && ownsLights )
    {
        if( rows*cols == 1 ) delete pLt0;
        else delete [] pLt0;
    }
}

void LightGrid::Clear( Light clearLt )const
{
    for( int r = 0; r < rows; ++r )
    {
        Light* pRow = pLt0 + r*cols;
        for( int c = 0; c < cols; ++c )
            *( pRow + c ) = clearLt;
    }
}

// Correct!!
void LightGrid::RotateCW()const// square Grid only
{
    if( rows != cols ) return;
    const int& W = cols;// common dimension
    for( int r = 0; r < W/2; ++r )
    {
     //   for( int c = 0; c < W; ++c )// wrong
     //   for( int c = r; c < W - r; ++c )// still wrong
        for( int c = r; c < W - r - 1; ++c )// Bingo! stop 1 short. Last was mapped 1st
        {
            // A 4 point tour
            Light* pLt1 = pLt0 + W*r + c;// start at Lt1
            Light* pLt2 = pLt0 + W*(1+c) - 1 - r;// 2nd clockwise
            Light* pLt3 = pLt0 + W*(W-r) - 1 - c;// 3rd clockwise
            Light* pLt4 = pLt0 + W*(W-1-c) + r;// 4th clockwise
            // backtracking is simpler
            Light saveLt = *pLt1;// save Lt1
            *pLt1 = *pLt4;
            *pLt4 = *pLt3;
            *pLt3 = *pLt2;
            *pLt2 = saveLt;
        }
    }
}

void LightGrid::RotateCCW()const// square Grid only
{
    if( rows != cols ) return;
    const int& W = cols;// common dimension
    for( int r = 0; r < W/2; ++r )
    {
        for( int c = r; c < W - r - 1; ++c )// stop 1 short. Last was mapped 1st
        {
            // A 4 point circuit
            Light* pLt1 = pLt0 + W*r + c;// start at Lt1
            Light* pLt2 = pLt0 + W*(W-1-c) + r;// 2nd ccw
            Light* pLt3 = pLt0 + W*(W-r) - 1 - c;// 3rd ccw
            Light* pLt4 = pLt0 + W*(1+c) - 1 - r;// 4th ccw
            // backtracking is simpler
            Light saveLt = *pLt1;// save Lt1
            *pLt1 = *pLt4;
            *pLt4 = *pLt3;
            *pLt3 = *pLt2;
            *pLt2 = saveLt;
        }
    }
}

// Good
void LightGrid::FlipX()const
{
    for( int r = 0; r < rows; ++r )
    {
        Light* itLt = pLt0 + r*cols;// first element
        Light* itRt = itLt + cols - 1;// last element
        while( itLt < itRt )
        {
            // swap values
            Light temp = *itLt;
            *itLt = *itRt;
            *itRt = temp;
            // move iterators
            ++itLt;
            --itRt;
        }
    }
}

// good
void LightGrid::FlipY()const
{
    for( int c = 0; c < cols; ++c )
    {
        Light* itLo = pLt0 + c;// top element
        Light* itHi = itLo + cols*( rows - 1 );// bottom element
        while( itLo < itHi )
        {
            // swap values
            Light temp = *itLo;
            *itLo = *itHi;
            *itHi = temp;
            // move iterators
            itLo += cols;
            itHi -= cols;
        }
    }
}

void LightGrid::Transpose()const
{
    if( rows != cols ) return;// square only
    const int W = rows;
    // iterate along top (A) and left side (B)
    // start 1 space from 0,0
    Light* iterA = pLt0;// + 1;// across top
    Light* iterB = pLt0;// + W;// down left side
    for( int n = 0; n < W; ++n )
    {
        iterA += 1;// across top
        iterB += W;// down left side
        // iterators from A and from B
        Light* iterUp = iterB;// higher
        Light* iterDn = iterA;
        while( iterUp > iterDn )
        {
            Light save = *iterUp;
            *iterUp = *iterDn;
            *iterDn = save;
            iterUp += 1 - W;// up 1 row and right 1
            iterDn += W - 1;// down 1 row and left 1
        }
      //  iterA += 1;// across top
      //  iterB += W;// down left side
    }

    // iterate along bottom (B) and right side (A)
    // start 1 space from leave off above
  //  iterA += W;// down right side
  //  iterB += 1;// across bottom
    for( int n = 1; n < W; ++n )
    {
        iterA += W;// down right side
        iterB += 1;// across bottom
        // iterators from A and from B
        Light* iterUp = iterB;// higher
        Light* iterDn = iterA;
        while( iterUp > iterDn )
        {
            Light save = *iterUp;
            *iterUp = *iterDn;
            *iterDn = save;
            iterUp += 1 - W;// up 1 row and right 1
            iterDn += W - 1;// down 1 row and left 1
        }
      //  iterA += W;// down right side
      //  iterB += 1;// across bottom
    }
}


// for 1d array
// static allocation
void LightGrid::init( Light& rLt, unsigned int numLts )
{
    pLt0 = &rLt;
    cols = static_cast<int>( numLts );
    rows = 1;
    ownsLights = false;
}
// dynamic allocation
void LightGrid::init( unsigned int numLts )
{
    // allow for re init
    if( pLt0 )// already owns an array
    {
        // keep if numLts == rows*cols ?
        if( rows*cols == 1 ) delete pLt0;
        else delete [] pLt0;
        pLt0 = nullptr;
    }
    // a one row array
    cols = static_cast<int>( numLts );
    rows = 1;

    if( numLts == 1 ) pLt0 = new Light();
    else pLt0 = new Light[ numLts ];
    ownsLights = true;
}

// for 2d array
// static allocation
void LightGrid::init( Light& rLt, int Rows, int Cols )
{
    pLt0 = &rLt;
    rows = Rows;
    cols = Cols;
    ownsLights = false;
}

// dynamic allocation
void LightGrid::init( int Rows, int Cols )
{
    ownsLights = true;
    // allow for re init
    if( pLt0 )// already owns an array
    {
        // keep if Rows*Cols == rows*cols ?
        if( rows*cols == 1 ) delete pLt0;
        else delete [] pLt0;
        pLt0 = nullptr;
    }

    rows = Rows;
    cols = Cols;
    if( rows*cols == 1 ) pLt0 = new Light();
    else pLt0 = new Light[ rows*cols ];
}
