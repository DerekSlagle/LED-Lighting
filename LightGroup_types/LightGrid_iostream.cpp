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

        std::cout << "\n~LightGrid deleted " << rows*cols << " Lights";
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

// Bad
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

void LightGrid::toFile( std::ostream& os, const std::vector<Light>& LtTone )const
{
    if( !pLt0 || rows <= 0 || cols <= 0 ) return;// invalid state

    os << rows << ' ' << cols << ' ';
    unsigned int numTones = LtTone.size();

    // numTones == 1 means all states are to be given as follows this if
    if( numTones > 1 )
    {
        os << numTones << ' ';
 //       int brt = 255;
        // write each tone in the image. This data is supplied by caller.
        for( unsigned int k = 0; k < numTones; ++k )
        {
     //       brt = 255*LtTone[k].brightness;
            os << LtTone[k].r  << ' '  << LtTone[k].g  << ' '  << LtTone[k].b << ' ';
        }

        os << isTransparent << ' ';
   //     brt = 255*transLt.brightness;
   //     if( isTransparent ) os << transLt.rd  << ' ' << transLt.gn  << ' ' << transLt.bu << ' ' << brt  << ' ';

        Light outLt;// = 'a';// for values 0 to 255

        unsigned int N = static_cast<unsigned int>( rows*cols );
        unsigned int LtCount = 0;
        for( unsigned int k = 0; k < N; ++k )
        {
            // read from Light k
            outLt = *( pLt0 + k );

            /*
            if( outLt == LtTone[0] )
            {
                os << static_cast<unsigned char>( 'a' );
                    ++LtCount;
            }
            else// must be foreground
            {
                os << static_cast<unsigned char>( 'b' );
                    ++LtCount;
            }   */

            bool foundTone = false;
            for( unsigned int tn = 0; tn < LtTone.size(); ++tn )
                if( outLt == LtTone[tn] )// found it!
                {
                    os << static_cast<unsigned char>( 'a' + tn );// write symbol
                    ++LtCount;
                    foundTone = true;
                    break;
                }

            if( !foundTone )
            {
                os << static_cast<unsigned char>( 'a' );// substitute background
                //    ++LtCount;
            }
        }

    //    std::cout << "\nLightGrid::toFile(): LtCount = " << LtCount;

        return;
    }

    // fall through case numTones == 1. Save every state.
    os << (int)isTransparent;
    if( isTransparent ) os << ' ' << transLt.r << ' ' << transLt.g << ' ' << transLt.b;

    for( int k = 0; k < rows*cols; ++k )
    {
        const Light* pLtK = pLt0 + k;
        if( isTransparent && *pLtK == transLt )
            os << ' ' << -1;
        else
        {
    //        unsigned int alpha = pLtK->brightness*255;
            os << ' ' << pLtK->r << ' ' << pLtK->g << ' ' << pLtK->b;
      //      os << ' ' << alpha;
        }
    }
}

// creation from file. Not for single Lights
bool LightGrid::init( std::istream& is )// owner only
{
    is >> rows >> cols;
    if( rows <= 0 || cols <= 0 || rows*cols < 2 ) return false;// invalid state

    // allocate Lights
    pLt0 = new Light[rows*cols];
    if( !pLt0 ) return false;

    unsigned int N = static_cast<unsigned int>( rows*cols );
    int rd = 0, gn = 0, bu = 0;//, brt = 0;

    unsigned int numTones;
    is >> numTones;
    if( numTones > 1 )
    {
        std::vector<Light> lightTone;
        for( unsigned int k = 0; k < numTones; ++k )
        {
            is >> rd >> gn >> bu;
            lightTone.push_back( Light( rd, gn, bu ) );
        }

        is >> isTransparent;
        transLt = lightTone[0];// write as 1st tone
        unsigned char inChar = 'a';// for values 0 to 255
        for( unsigned int k = 0; k < N; ++k )
        {
            if( !is.good() )
                std::cout << "\nLightGrid::init fail k = " << k;

            is >> inChar;
            unsigned int idx = inChar - 'a';// cast?
            // write to Light
            *( pLt0 + k ) = lightTone[ idx ];
        }

        return true;
    }


    // fall through case: every Light state is given
    is >> isTransparent;
    if( isTransparent ) is >> transLt.r >> transLt.g >> transLt.b;
 //   transLt.brightness = 1.0f;

    for( unsigned int k = 0; k < N; ++k )
    {
        if( !is.good() ) return false;

        is >> rd;
        if( rd == -1 )
        {
            *( pLt0 + k ) = transLt;
        }
        else
        {
            is  >> gn >> bu;
            Light temp( rd, gn, bu );
            *( pLt0 + k ) = temp;
        }
    }

    return true;
}

// rows and cols must agree. This changes state only.
bool LightGrid::fromFile( std::istream& is )
{
    if( !pLt0 || rows <= 0 || cols <= 0 || rows*cols < 2 ) return false;// invalid state
    if( rows <= 0 || cols <= 0 || rows*cols < 2 ) return false;// invalid state

    int inRows, inCols;
    is >> inRows >> inCols;
    if( rows != inRows || cols != inCols ) return false;// conflicting data


    unsigned int N = static_cast<unsigned int>( rows*cols );
    int rd = 0, gn = 0, bu = 0;//, brt = 0;

    unsigned int numTones;
    is >> numTones;
    if( numTones > 1 )
    {
        std::vector<Light> lightTone;
        for( unsigned int k = 0; k < numTones; ++k )
        {
            is >> rd >> gn >> bu;
            lightTone.push_back( Light( rd, gn, bu ) );
        }

        is >> rd;
        isTransparent = ( rd == 1 );
      //  is >> isTransparent;
        transLt = lightTone[0];// write as 1st tone
        unsigned char inChar = 'a';// for values 0 to 255
        for( unsigned int k = 0; k < N; ++k )
        {
            if( !is.good() )
                std::cout << "\nLightGrid::fromFile fail k = " << k;

            is >> inChar;
            unsigned int idx = inChar - 'a';// cast?
            // write to Light
            *( pLt0 + k ) = lightTone[ idx ];
        }

        return true;
    }


    // fall through case: every Light state is given
    is >> isTransparent;
    if( isTransparent ) is >> transLt.r >> transLt.g >> transLt.b;
 //   transLt.brightness = 1.0f;

    for( unsigned int k = 0; k < N; ++k )
    {
        if( !is.good() ) return false;

        is >> rd;
        if( rd == -1 )
        {
            *( pLt0 + k ) = transLt;
        }
        else
        {
            is  >> gn >> bu;
            Light temp( rd, gn, bu );
            *( pLt0 + k ) = temp;
        }
    }

    return true;
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
