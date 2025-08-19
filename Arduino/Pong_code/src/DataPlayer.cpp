#include "DataPlayer.h"

void DataPlayer::init( Light& r_Lt0, int GridRows, int GridCols, uint8_t& r_StateData, unsigned int DataSz, uint8_t NumColors )
{
    pLt0 = &r_Lt0;
    rows = GridRows;
    cols = GridCols;

    stepTimer = 0;
    stepIter = 0;

    // default is entire grid
    gridRows = GridRows;
    gridCols = GridCols;
    row0 = col0 = 0;

    pStateData = &r_StateData;
    stateDataSz = DataSz;
    numColors = NumColors;
    BA.init( r_StateData, DataSz );

    const int NumLts = rows*cols;

    if( numColors > 8 && numColors <= 16 )
        numSteps = (2*DataSz)/NumLts;
    else if( numColors > 4 && numColors <= 8 )// new
        numSteps = (8*DataSz)/(3*NumLts);
    else if( numColors > 2 )// 3 or 4
        numSteps = (4*DataSz)/NumLts;
    else if( numColors == 2 )
        numSteps = (8*DataSz)/NumLts;
}

void DataPlayer::setGridBounds( int Row0, int Col0, int GridRows, int GridCols )
{
    row0 = Row0;
    col0 = Col0;
    gridRows = GridRows;
    gridCols = GridCols;
}

void DataPlayer::bindToGrid( Light& r_Lt0, int GridRows, int GridCols )
{
    pLt0 = &r_Lt0;
    setGridBounds( row0, col0, GridRows, GridCols );
}

void DataPlayer::setTargetRect( int Rows, int Cols, int Row0, int Col0 )
{
    rows = Rows;
    cols = Cols;
    row0 = Row0;
    col0 = Col0;
}

int DataPlayer::getDrawMode()const
{
    if( rows == gridRows && cols == gridCols && row0 == 0 && col0 == 0 )
        return 1;// is grid
    if( ( row0 >= 0 && row0 + rows <= gridRows ) && ( col0 >= 0 && col0 + cols <= gridCols ) )
        return 2;// is all in grid

    return 3;// is partly in grid
}

void DataPlayer::takeStep()
{
    if( !isPlaying ) return;

    if( ++stepTimer >= stepPause )
    {
        stepTimer = 0;// to next step
        if( ++stepIter >= numSteps )
            stepIter = 0;// start over
    }
}

void DataPlayer::Clear( Light clearLt )const
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
            *( pRow + c ) = clearLt;
        }
    }
}

void DataPlayer::updateIsGrid()// 1
{
    for( int n = 0; n < rows*cols; ++n )
    {
        if( fadeAlong )
            *( pLt0 + n ) = updateFade(n);
        else// regular draw
        {
            Light LtNow = getState(n);
            if( drawOff ) *( pLt0 + n ) = LtNow;// draw all colors
            else if( LtNow != pLight[0] ) *( pLt0 + n ) = LtNow;// draw all but Lt[0]
        }
    }

    takeStep();
}

void DataPlayer::updateAllIn()// 2
{
    Light* pBase = pLt0 + gridCols*row0 + col0;

    for( int r = 0; r < rows; ++r )
    {
        Light* pRow = pBase + r*gridCols;
        for( int c = 0; c < cols; ++c )
        {
            if( fadeAlong ) *( pRow + c ) = updateFade( r*cols + c );
            else// regular draw
            {
                Light LtNow = getState( r*cols + c );
                if( drawOff ) *( pRow + c ) = LtNow;// draw all colors
                else if( LtNow != pLight[0] ) *( pRow + c ) = LtNow;// draw all but Lt[0]
            }
        }
    }

    takeStep();
}

void DataPlayer::flipX_AllIn()
{
    Light* pBase = pLt0 + gridCols*row0 + col0;
    for( int r = 0; r < rows; ++r )
    {
        Light* pRow = pBase + r*gridCols;
        for( int c = 0; c < cols; ++c )
        {
            if( fadeAlong ) *( pRow + c ) = updateFade( r*cols + cols - 1 - c );
            else// regular draw
            {
                Light LtNow = getState( r*cols + cols - 1 - c );
                if( drawOff ) *( pRow + c ) = LtNow;// draw all colors
                else if( LtNow != pLight[0] ) *( pRow + c ) = LtNow;// draw all but Lt[0]
            }
        }
    }

    takeStep();
}

void DataPlayer::flipY_AllIn()
{
    Light* pBase = pLt0 + gridCols*row0 + col0;
    for( int r = 0; r < rows; ++r )
    {
        Light* pRow = pBase + r*gridCols;
        for( int c = 0; c < cols; ++c )
        {
            if( fadeAlong ) *( pRow + c ) = updateFade( ( rows - 1 - r )*cols + c );
            else// regular draw
            {
                Light LtNow = getState( ( rows - 1 - r )*cols + c );
                if( drawOff ) *( pRow + c ) = LtNow;// draw all colors
                else if( LtNow != pLight[0] ) *( pRow + c ) = LtNow;// draw all but Lt[0]
            }
        }
    }

    takeStep();
}

void DataPlayer::updatePartlyIn()// 3
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

            if( fadeAlong ) *( pRow + c ) = updateFade( r*cols + c );
            else// regular draw
            {
                Light LtNow = getState( r*cols + c );
                if( drawOff ) *( pRow + c ) = LtNow;// draw all colors
                else if( LtNow != pLight[0] ) *( pRow + c ) = LtNow;// draw all but Lt[0]
            }
        }
    }

    takeStep();
}

void DataPlayer::update()
{
    int DrawMode = getDrawMode();
    if( flipX )
    {
        if( DrawMode == 1 || DrawMode == 2 ) flipX_AllIn();
        else updatePartlyIn();// no flip is done here
    }
    else if( flipY )
    {
        if( DrawMode == 1 || DrawMode == 2 ) flipY_AllIn();
        else updatePartlyIn();// no flip is done here
    }
    else
    {
        if( DrawMode == 1 ) updateIsGrid();
        else if( DrawMode == 2 ) updateAllIn();
        else updatePartlyIn();
    }

    // after process. Adjust brightness
    if( pBrtFactor )
        updateBrigtness( *pBrtFactor );
}

Light DataPlayer::updateFade( unsigned int n )const
{
    if( !pLight ) return Light(0,200,0);

    Light LtNow = pLight[0], LtNext = pLight[0];
    unsigned int iterNext = ( stepIter + 1 )%numSteps;
    const int NumLts = rows*cols;

    if( numColors > 4 && numColors <= 16 )
    {
        LtNow = pLight[ BA.getQuadBit( stepIter*NumLts + n ) ];
        LtNext = pLight[ BA.getQuadBit( iterNext*NumLts + n ) ];
    }
    else if( numColors > 2 )// 3 or 4
    {
        LtNow = pLight[ BA.getDblBit( stepIter*NumLts + n ) ];
        LtNext = pLight[ BA.getDblBit( iterNext*NumLts + n ) ];
    }
    else if( numColors == 2 )// 2 color
    {
        LtNow = BA.getBit( stepIter*NumLts + n ) ? pLight[1] : pLight[0];
        LtNext = BA.getBit( iterNext*NumLts + n ) ? pLight[1] : pLight[0];
    }

    if( LtNow == LtNext ) return LtNow;

    float u = (float)stepTimer/(float)stepPause;
    u = u*u*( 3.0f - 2.0f*u );

    float fr = u*LtNext.r + ( 1.0f - u )*LtNow.r;
    float fg = u*LtNext.g + ( 1.0f - u )*LtNow.g;
    float fb = u*LtNext.b + ( 1.0f - u )*LtNow.b;

    return Light( fr, fg, fb );
}

// Zoom by 1/2. even rows and cols only. Averages 4 Lights into 1. No fade effect
void DataPlayer::updateZoomOut()// 3
{
    if( rows%2 || cols%2 ) return;// even rows and cols only.

    Light* pBase = pLt0 + gridCols*row0 + col0;
    Light cellLt;
    uint16_t rd = 0, gn = 0, bu = 0;

    for( int r = 0; r < rows; r += 2 )
    {
        if( r + row0 < 0 ) continue;
        if( r + 1 + row0 >= gridRows ) break;

        Light* pRow = pBase + (r/2)*gridCols;
        for( int c = 0; c < cols; c += 2 )
        {
            if( c + col0 < 0 ) continue;
            if( c + 1 + col0 >= gridCols ) break;

            // sum over 4 Lights
            rd = 0; gn = 0; bu = 0;
            for( unsigned int k = 0; k < 4; ++k )
            {
                cellLt = getState( ( r + k/2 )*cols + c + k%2 );
                rd += cellLt.r;
                gn += cellLt.g;
                bu += cellLt.b;
            }

            Light LtNow( rd/4.0f, gn/4.0f, bu/4.0f );
            if( drawOff ) *( pRow + c/2 ) = LtNow;// draw all colors
            else if( LtNow != pLight[0] ) *( pRow + c/2 ) = LtNow;// draw all but Lt[0]
        }
    }

    takeStep();
}

// safe. Good for partly in
void DataPlayer::updateBrigtness( float brtFactor )
{
    if( brtFactor <= 0.0f ) return;// nonsense value

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

            Light LtNow = getState( r*cols + c );
            if( LtNow != pLight[0] )// fade only non background colors
            {
                float fCol = LtNow.r*brtFactor;
                if( fCol > 255.0f ) fCol = 255.0f;// clamp value
                LtNow.r = fCol;
                fCol = LtNow.g*brtFactor;
                if( fCol > 255.0f ) fCol = 255.0f;
                LtNow.g = fCol;
                fCol = LtNow.b*brtFactor;
                if( fCol > 255.0f ) fCol = 255.0f;
                LtNow.b = fCol;
                *( pRow + c ) = LtNow;// draw all but Lt[0]
            }
        }
    }
}

Light DataPlayer::getState( unsigned int n )const
{
    if( !pLight ) return Light(200,0,0);
    const int NumLts = rows*cols;

    if( numColors > 8 && numColors <= 16 )
        return pLight[ BA.getQuadBit( stepIter*NumLts + n ) ];

    if( numColors > 4 && numColors <= 8 )// new case
        return pLight[ BA.getTriBit( stepIter*NumLts + n ) ];

    if( numColors > 2 )// 3 or 4
        return pLight[ BA.getDblBit( stepIter*NumLts + n ) ];

    if( numColors == 2 )
    {
        if( BA.getBit( stepIter*NumLts + n ) ) return pLight[1];
        return pLight[0];
    }

    return pLight[0];
}

void DataPlayer::prevImage()
{
    stepTimer = 0;
    if( stepIter > 0 ) --stepIter;
    else stepIter = numSteps - 1;
}

void DataPlayer::nextImage()
{
    stepTimer = 0;
    if( ++stepIter >= numSteps )
        stepIter = 0;
}

void DataPlayer::showImage( unsigned int n )// 0 to numSteps - 1
{
    stepTimer = 0;
    stepIter = n%numSteps;// keep in range
}

// For testing. on grid only
void DataPlayer::showColors()const
{
    for( int n = 0; n < rows*cols; ++n )
    {
        if( n < (int)numColors ) *( pLt0 + n ) = pLight[n];
        else *( pLt0 + n ) = pLight[0];
    }
}

void DataPlayer::updateRainfall( int (*pGetCol)(void) )
{
    if( stepTimer == 0 )// shift states down 1 row
    {
        for( int c = 0; c < cols; ++c )
        {
            for( int r = rows - 1; r > 0; --r )// up from bottom
            {
                BA.setBit( r*cols + c, BA.getBit( (r-1)*cols + c ) );// from row above to row below
            }
            // write all 0's to row 0
            BA.setBit( c, false );
        }
        // write new raindrops to row 0
        if( pGetCol() )
        {
            int numDrops = pGetCol()%3;
            for( int d = 0; d < numDrops; ++d )
            {
                int C = pGetCol()%cols;
                if( !BA.getBit( cols + C ) )// is 0 one row down
                    BA.setBit( C, true );
            }
        }
    }

    update();// normally
}

void DataPlayer::updateRainfall_fade( int (*pGetCol)(void), unsigned int easeMode )
{
    takeStep();

    if( stepTimer == 0 )// shift states down 1 row
    {
        for( int c = 0; c < cols; ++c )
        {
            for( int r = rows - 1; r > 0; --r )// up from bottom
                BA.setBit( r*cols + c, BA.getBit( (r-1)*cols + c ) );// from row above to row below

            // write all 0's to row 0
            BA.setBit( c, false );
        }
        // write new raindrops to row 0
        if( pGetCol() )
        {
            int numDrops = pGetCol()%3;
            for( int d = 0; d < numDrops; ++d )
            {
                int C = pGetCol()%cols;
                if( !BA.getBit( cols + C ) )// is 0 one row down
                    BA.setBit( C, true );
            }
        }
    }

    // fade each row into the next row
    float U = (float)stepTimer/(float)stepPause;// interpolation factor

    if( easeMode > 0 )
    {
        if( easeMode == 1 ) U = U*U*( 3.0f - 2.0f*U );// s curve. yp(0) = yp(1) = 0
        else if( easeMode == 2 ) U = U*( 2.0f - U );// concave upward
        else if( easeMode == 3 ) 0.5f*U*( 1.0f + U );// concave down
        else U = U*U;// concave down more
    }

    Light* pBase = pLt0 + row0*gridCols + col0;

    for( int c = 0; c < cols; ++c )
    {
        if( drawOff )
        {
            for( int r = 0; r < rows; ++r )
                *( pBase + r*gridCols + c ) = pLight[0];
        }

        for( int r = 0; r + 1 < rows; ++r )// down from top
        {
            if( !BA.getBit( r*cols + c ) ) continue;

            // a raindrop is at r,c
            int ofst_0 = (r+row0)*gridCols + col0 + c;// row r
            Light Lt_0 = drawOff ? pLight[0] : pLt0[ ofst_0 ];// bkgd or existing Light
            // write to Light in row r
            pLt0[ ofst_0 ].r = U*Lt_0.r + ( 1.0f - U )*pLight[1].r;
            pLt0[ ofst_0 ].g = U*Lt_0.g + ( 1.0f - U )*pLight[1].g;
            pLt0[ ofst_0 ].b = U*Lt_0.b + ( 1.0f - U )*pLight[1].b;

            // write to Light in row r+1
            int ofst_1 = (r+1+row0)*gridCols + col0 + c;// row r+1
            Light Lt_1 =  drawOff ? pLight[0] : pLt0[ ofst_1 ];// current assigned state
            pLt0[ ofst_1 ].r = ( 1.0f - U )*Lt_1.r + U*pLight[1].r;
            pLt0[ ofst_1 ].g = ( 1.0f - U )*Lt_1.g + U*pLight[1].g;
            pLt0[ ofst_1 ].b = ( 1.0f - U )*Lt_1.b + U*pLight[1].b;
        }


        // fade the last row drops ?
        int ofst = (rows-1+row0)*gridCols + col0 + c;
        Light Lt_2 =  drawOff ? pLight[0] : pLt0[ ofst ];
        if( BA.getBit( (rows - 1)*cols + c ) )
        {
            pLt0[ ofst ].r = U*Lt_2.r + ( 1.0f - U )*pLight[1].r;
            pLt0[ ofst ].g = U*Lt_2.g + ( 1.0f - U )*pLight[1].g;
            pLt0[ ofst ].b = U*Lt_2.b + ( 1.0f - U )*pLight[1].b;
        }
    }
}
