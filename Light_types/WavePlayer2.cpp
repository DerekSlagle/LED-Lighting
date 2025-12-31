#include "WavePlayer2.h"

void WavePlayer2::init( Light& r_Lt0, unsigned int GridRows, unsigned int GridCols, Light HiLt, Light LoLt )
{
    pLt0 = &r_Lt0;
    // default is over entire Grid
    gridRows = GridRows;
    gridCols = GridCols;
    rows = GridRows;
    cols = GridCols;
    numLts = rows*cols;
    setHiLight( HiLt );
    setLoLight( LoLt );
    tElapLt = tElapRt = 0.0f;
    drawMode = 1;    
}

void WavePlayer2::setDrawMode()
{
    if( rows == gridRows && cols == gridCols && row0 == 0 && col0 == 0 )
        drawMode = 1;// is grid
    else if( ( row0 >= 0 && row0 + rows <= gridRows ) && ( col0 >= 0 && col0 + cols <= gridCols ) )
        drawMode = 2;// is all in grid
    else
        drawMode = 3;// is partly in grid
}

void WavePlayer2::bindToGrid( Light& r_Lt0, unsigned int GridRows, unsigned int GridCols )
{
    pLt0 = &r_Lt0;
    // default is over entire Grid
    gridRows = (int)GridRows;
    gridCols = (int)GridCols;
    setDrawMode();
}

void WavePlayer2::setTargetRect( int Rows, int Cols, int Row0, int Col0 )
{
    rows = Rows;
    cols = Cols;
    row0 = Row0;
    col0 = Col0;
    setDrawMode();
}

void WavePlayer2::setWaveData( float ampRt, float wvLen_lt, float wvSpd_lt, float wvLen_rt, float wvSpd_rt )
{
    wvLenLt = wvLen_lt;
    wvLenRt = wvLen_rt;// in array indices
    wvSpdLt = wvSpd_lt;
    wvSpdRt = wvSpd_rt;
    AmpRt = ampRt;
    AmpLt = 1.0f - AmpRt;
    periodLt = wvLenLt/wvSpdLt;
    periodRt = wvLenRt/wvSpdRt;
    tElapLt = 0.0f;
    tElapRt = 0.0f;
}

void WavePlayer2::setSeriesCoeffs( float* C_rt, unsigned int n_TermsRt, float* C_lt, unsigned int n_TermsLt )
{
    C_Rt = C_rt;
    nTermsRt = n_TermsRt;
    C_Lt = C_lt;
    nTermsLt = n_TermsLt;

    // coefficients must sum to 1.0f
    float sum = 0.0f;
    if( C_Rt )
    {
        for( unsigned int k = 0; k < nTermsRt; ++k )
        {
            if( C_Rt[k] > 0.0f ) sum += C_Rt[k];
            else sum -= C_Rt[k];
        }
        for( unsigned int k = 0; k < nTermsRt; ++k )
        {
            C_Rt[k] /= sum;
        }
    }

    if( C_Lt )
    {
        sum = 0.0f;
        for( unsigned int k = 0; k < nTermsLt; ++k )
        {
            if( C_Lt[k] > 0.0f ) sum += C_Lt[k];
            else sum -= C_Lt[k];
        }
        for( unsigned int k = 0; k < nTermsLt; ++k )
        {
            C_Lt[k] /= sum;
        }
    }
}

// allow dt < 0
void WavePlayer2::update_tElapOnly( float dt )
{
    tElapRt += dt;
    if( (dt >= 0.0f) && (tElapRt > periodRt) ) tElapRt -= periodRt;
    else if( (dt < 0.0f) && (tElapRt < 0.0f) ) tElapRt += periodRt;

    tElapLt += dt;
    if( (dt >= 0.0f) && (tElapLt > periodLt) ) tElapLt -= periodLt;
    else if( (dt < 0.0f) && (tElapLt < 0.0f) ) tElapLt += periodLt;
}

/*
void WavePlayer2::update_tElapOnly( float dt )
{
    tElapRt += dt;
    if( tElapRt > periodRt ) tElapRt -= periodRt;
    tElapLt += dt;
    if( tElapLt > periodLt ) tElapLt -= periodLt;
}
*/

Light WavePlayer2::getState( int n )const
{
    if( n < 0 || n >= numLts ) return Light();

    float fr = 0.0f, fg = 0.0f, fb = 0.0f;// result
    float yRt = 0.0f;
    float arg = ( (float)n/wvLenRt - tElapRt/periodRt )*6.283f;

    if( C_Rt )
    {
        for( unsigned k = 0; k < nTermsRt; ++k )
        {
            if( C_Rt[k] != 0.0f )
                yRt += C_Rt[k]*sinf( (k+1)*arg );
        }
    }
    else
        yRt = sinf( arg );

    float yLt = 0.0f;
    arg = ( (float)n/wvLenLt + tElapLt/periodLt )*6.283f;
    if( C_Lt )
    {
        for( unsigned k = 0; k < nTermsLt; ++k )
            if( C_Lt[k] != 0.0f )
                yLt += C_Lt[k]*sinf( (k+1)*arg );
    }
    else
        yLt = sinf( arg );

    float y = AmpRt*yRt + AmpLt*yLt;

 //   if( y < -1.0f || y > 1.0f ) y is out of range

    fr = 0.5f*( ( y + 1.0f )*frHi - ( y - 1.0f )*frLo );
    fg = 0.5f*( ( y + 1.0f )*fgHi - ( y - 1.0f )*fgLo );
    fb = 0.5f*( ( y + 1.0f )*fbHi - ( y - 1.0f )*fbLo );
    return Light( fr, fg, fb );
}

Light WavePlayer2::getLight( int r, int c )const
{
    // return default Light if r or c out of range
    if( row0 + r < 0 || row0 +r >= (int)gridRows ) return Light();
    if( col0 + c < 0 || col0 + c >= (int)gridCols ) return Light();

    return getState( r*cols + c );// r, c within target rect. 'local' coordinates
}

// new update methods
void WavePlayer2::update( float dt )
{
    update_tElapOnly(dt);

    if( drawMode == 1 ) updateIsGrid();
    else if( drawMode == 2 ) updateAllIn();
    else updatePartlyIn();
}

void WavePlayer2::draw()const
{
    if( drawMode == 1 ) updateIsGrid();
    else if( drawMode == 2 ) updateAllIn();
    else updatePartlyIn();
}

void WavePlayer2::updateIsGrid()const// 1
{
    for( int n = 0; n < numLts; ++n )
        *( pLt0 + n ) = getState(n);
}

void WavePlayer2::updateAllIn()const// 2
{
    Light* pBase = pLt0 + gridCols*row0 + col0;

    for( int r = 0; r < rows; ++r )
    {
        Light* pRow = pBase + r*gridCols;
        for( int c = 0; c < cols; ++c )
            *( pRow + c ) = getState( r*cols + c );
    }
}

void WavePlayer2::updatePartlyIn()const// 3
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

            *( pRow + c ) = getState( r*cols + c );
        }
    }
}

// new. Plot as function. Color above = HiLight and below = LoLight
void WavePlayer2::drawGraph()const
{
    Light HiLight( frHi, fgHi, fbHi );
    Light LoLight( frLo, fgLo, fbLo );
    Light* pBase = pLt0 + gridCols*row0 + col0;

    for( int c = 0; c < cols; ++c )
    {
        if( c + col0 < 0 ) continue;
        if( c + col0 >= gridCols ) break;

        // find y for this c
        float yRt = 0.0f;
        float arg = ( (float)c/wvLenRt - tElapRt/periodRt )*6.283f;

        if( C_Rt )
        {
            for( unsigned k = 0; k < nTermsRt; ++k )
            {
                if( C_Rt[k] != 0.0f )
                    yRt += C_Rt[k]*sinf( (k+1)*arg );
            }
        }
        else
            yRt = sinf( arg );

        float yLt = 0.0f;
        arg = ( (float)c/wvLenLt + tElapLt/periodLt )*6.283f;
        if( C_Lt )
        {
            for( unsigned k = 0; k < nTermsLt; ++k )
                if( C_Lt[k] != 0.0f )
                    yLt += C_Lt[k]*sinf( (k+1)*arg );
        }
        else
            yLt = sinf( arg );

        float y = AmpRt*yRt + AmpLt*yLt;
        // scale to graph amplitude
        y *= graphAmp;
        // center up
        y += rows/2;
        int Rc = (int)y;
        // down each column
        for( int r = 0; r < Rc; ++r )
        {
            if( r + row0 < 0 ) continue;
            if( r + row0 >= gridRows ) break;
            pBase[ r*gridCols + c ] = HiLight;
        }
        // below
        for( int r = Rc; r < rows; ++r )
        {
            if( r + row0 < 0 ) continue;
            if( r + row0 >= gridRows ) break;
            pBase[ r*gridCols + c ] = LoLight;
        }
    }

    return;
}