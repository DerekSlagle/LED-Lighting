#include "LightSource.h"

/*
void LightSource::Clear( Light clearLt )const
{
    Light* pLtBase = pTgt->pLt0 + row0*pTgt->cols + col0;

    for( int r = 0; r < rows; ++r )
    {
        Light* pRowTgt = pLtBase + r*pTgt->cols;
        for( int c = 0; c < cols; ++c )
        {
            *( pRowTgt + c ) = clearLt;
        }
    }
}
*/

void LightSource::Clear( Light clearLt )const
{
    Light* pLtBase = pTgt->pLt0 + row0*pTgt->cols + col0;

    for( int r = 0; r < rows; ++r )
    {
        if( r + row0 < 0 || r + row0 >=  pTgt->rows ) continue;// no wrap

        Light* pRowTgt = pLtBase + r*pTgt->cols;
        for( int c = 0; c < cols; ++c )
        {
            if( c + col0 >= 0 && c + col0 < pTgt->cols )// column bound check no wrap
            {
                Light* pLt = pRowTgt + c;// absolute test. Light must be on grid
                *pLt = clearLt;
            }
        }
    }
}

bool LightSource::isInGrid()const
{
    //                      was >= here. Wrong! the case cols == pTgt->cols was excluded (and for rows)
    if( col0 < 0 || col0 + cols > pTgt->cols ) return false;// partially left or right of grid
    if( row0 < 0  || row0 + rows > pTgt->rows ) return false;// partially above or below grid
    return true;// fully inside of grid
}

bool LightSource::isAllOut()const
{
    if( col0 + cols < 0 || col0 >= pTgt->cols ) return true;// fully left or right of grid
    if( row0 + rows < 0 || row0 >= pTgt->rows ) return true;// fully above or below grid
    return false;// partly or fully in
}

void LightSource::updatePosition( float dt )
{
    if( !isMoving ) return;
    pos += vel*dt;
    col0 = static_cast<int>( pos.x - ctrOfst.x + 0.5f );// rounded
    row0 = static_cast<int>( pos.y - ctrOfst.y + 0.5f );
}

void LightSource::setPosition( float x, float y )
{
    pos.x = x;
    pos.y = y;
    col0 = static_cast<int>( x - ctrOfst.x + 0.5f );// rounded
    row0 = static_cast<int>( y - ctrOfst.y + 0.5f );
}

// true when image moves or frame changes
bool LightSource::update( float dt )
{
    bool retVal = false;

    if( isMoving )
    {
        updatePosition(dt);
        retVal = true;
    }

    if( isPlaying && numFrames > 1 )
    {
        tElapFrame += dt;
        if( tElapFrame >= tFrame )
        {
        //    tElapFrame = 0.0f;
            tElapFrame -= tFrame;// smoother?
            if( playForward ) nextFrame();
            else prevFrame();
            retVal = true;
        }
    }

    return retVal;
}

// draw functions
void LightSource::draw()const
{
    if( outLineThickness > 0 ) drawOutline();
    if( flipX || flipY ) return drawFlipped();
    if( rShift != 0 || cShift != 0 ) return drawShifted();

    if( isInGrid() )
    {
        Light* pLtBase = pTgt->pLt0 + row0*pTgt->cols + col0;
        for( int r = 0; r < rows; ++r )
        {
            Light* pRowTgt = pLtBase + r*pTgt->cols;
            for( int c = 0; c < cols; ++c )
                pRowTgt[c] = getLt( r, c );
        }
        return;
    }

    if( isAllOut() ) return;

    // must bound check
    Light* pLtBase = pTgt->pLt0 + row0*pTgt->cols + col0;

    for( int r = 0; r < rows; ++r )
    {
        if( r + row0 < 0 ) continue;// no wrap
        if( r + row0 >=  pTgt->rows ) break;

        Light* pRowTgt = pLtBase + r*pTgt->cols;
        for( int c = 0; c < cols; ++c )
        {
            if( c + col0 < 0 ) continue;// no wrap
            if( c + col0 >=  pTgt->cols ) break;

            pRowTgt[c] = getLt( r, c );
        }
    }
}

void LightSource::drawFlipped()const
{

    if( isInGrid() )
    {
        Light* pLtBase = pTgt->pLt0 + row0*pTgt->cols + col0;
        for( int r = 0; r < rows; ++r )
        {
            Light* pRowTgt = pLtBase + r*pTgt->cols;
            for( int c = 0; c < cols; ++c )
            {
                if( flipX )
                {
                    if( flipY ) pRowTgt[c] = getLt( rows - 1 - r, cols - 1 - c );// both
                    else pRowTgt[c] = getLt( r, cols - 1 - c );// col only
                }
                else// flipY
                    pRowTgt[c] = getLt( rows - 1 - r, c );// row only
            }
        }
        return;
    }

    if( isAllOut() ) return;

    // must bound check
    Light* pLtBase = pTgt->pLt0 + row0*pTgt->cols + col0;

    for( int r = 0; r < rows; ++r )
    {
        if( r + row0 < 0 ) continue;// no wrap
        if( r + row0 >=  pTgt->rows ) break;

        Light* pRowTgt = pLtBase + r*pTgt->cols;
        for( int c = 0; c < cols; ++c )
        {
            if( c + col0 < 0 ) continue;// no wrap
            if( c + col0 >=  pTgt->cols ) break;

            if( flipX )
            {
                if( flipY ) pRowTgt[c] = getLt( rows - 1 - r, cols - 1 - c );// both
                else pRowTgt[c] = getLt( r, cols - 1 - c );// col only
            }
            else// flipY
                pRowTgt[c] = getLt( rows - 1 - r, c );// row only
        }
    }
}

void LightSource::drawShifted()const
{
    if( isInGrid() )
    {
        Light* pLtBase = pTgt->pLt0 + row0*pTgt->cols + col0;
        for( int r = 0; r < rows; ++r )
        {
            // no wrap
            if( r - rShift < 0 ) continue;// may become positive
            if( r - rShift >= rows ) break;// will only get higher

            Light* pRowTgt = pLtBase + r*pTgt->cols;
            for( int c = 0; c < cols; ++c )
            {
                // no wrap
                if( c - cShift < 0 ) continue;// may become positive
                if( c - cShift >= rows ) break;// will only get higher
                pRowTgt[c] = getLt( r - rShift, c - cShift );
            }
        }
        return;
    }

    if( isAllOut() ) return;

    // must bound check
    Light* pLtBase = pTgt->pLt0 + row0*pTgt->cols + col0;

    for( int r = 0; r < rows; ++r )
    {
        if( r + row0 < 0 ) continue;
        if( r + row0 >=  pTgt->rows ) break;
        // no wrap
        if( r - rShift < 0 ) continue;// may become positive
        if( r - rShift >= rows ) break;// will only get higher

        Light* pRowTgt = pLtBase + r*pTgt->cols;
        for( int c = 0; c < cols; ++c )
        {
            if( c + col0 < 0 ) continue;// no wrap
            if( c + col0 >=  pTgt->cols ) break;
            // no wrap
            if( c - cShift < 0 ) continue;// may become positive
            if( c - cShift >= rows ) break;// will only get higher
            pRowTgt[c] = getLt( r - rShift, c - cShift );
        }
    }
}

void LightSource::drawOutline()const
{
    // be safe just bound check
    Light* pLtBase = pTgt->pLt0 + row0*pTgt->cols + col0;
    const int& OT = outLineThickness;// sub convenience

    // top
    for( int r = -OT; r < 0; ++r )
    {
        if( r + row0 < 0 ) continue;// no wrap
        if( r + row0 >=  pTgt->rows ) break;

        Light* pRowTgt = pLtBase + r*pTgt->cols;
        for( int c = -OT; c < cols + OT; ++c )
        {
            if( c + col0 < 0 ) continue;// no wrap
            if( c + col0 >=  pTgt->cols ) break;
            pRowTgt[c] = outlineLight;
        }
    }
    // bottom
    for( int r = rows; r < rows + OT; ++r )
    {
        if( r + row0 < 0 ) continue;// no wrap
        if( r + row0 >=  pTgt->rows ) break;

        Light* pRowTgt = pLtBase + r*pTgt->cols;
        for( int c = -OT; c < cols + OT; ++c )
        {
            if( c + col0 < 0 ) continue;// no wrap
            if( c + col0 >=  pTgt->cols ) break;
            pRowTgt[c] = outlineLight;
        }
    }

    // left
    for( int r = 0; r < rows; ++r )
    {
        if( r + row0 < 0 ) continue;// no wrap
        if( r + row0 >=  pTgt->rows ) break;

        Light* pRowTgt = pLtBase + r*pTgt->cols;
        for( int c = -OT; c < 0; ++c )
        {
            if( c + col0 < 0 ) continue;// no wrap
            if( c + col0 >=  pTgt->cols ) break;
            pRowTgt[c] = outlineLight;
        }
    }
    // right
    for( int r = 0; r < rows; ++r )
    {
        if( r + row0 < 0 ) continue;// no wrap
        if( r + row0 >=  pTgt->rows ) break;

        Light* pRowTgt = pLtBase + r*pTgt->cols;
        for( int c = cols; c < cols + OT; ++c )
        {
            if( c + col0 < 0 ) continue;// no wrap
            if( c + col0 >=  pTgt->cols ) break;
            pRowTgt[c] = outlineLight;
        }
    }
}


/*
// transforms. Map r,c in to r,c out. writes transformed r,c to r,c and returns true if in grid
bool LightSource::flipX( int& r, int& c )const
{
    if( r + row0 < 0 || r + row0 >=  pTgt->rows ) return false;// above or below grid
    c = cols - 1 - c;// transformed c
    if( c + col0 < 0 || c + col0 >= pTgt->cols ) return false;// left or right of grid
    return true;
}
*/
