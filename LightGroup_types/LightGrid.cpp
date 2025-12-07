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
    rows = Rows;
    cols = Cols;

    if( rows*cols == 1 ) pLt0 = new Light();
    else pLt0 = new Light[ rows*cols ];
    ownsLights = true;
}
