#include "BufferSource.h"

/*
// true when image moves or frame changes
bool BufferSource::update( float dt )
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
*/

Light BufferSource::getLt( int r, int c )const
{
    if( !pSrcBuff ) return Light();
    const LightGrid* pLG = pSrcBuff + frameIter;
    if( r < 0 || r >= pLG->rows ) return Light();// r out of range
    if( c < 0 || c >= pLG->cols ) return Light();// c out of range

    if( doBlend ) return getBlendedLight( r*cols + c );
    // else
    return pLG->pLt0[ r*cols + c ];
}

void BufferSource::setSource( LightGrid& rSrcBuff, int NumFrames, float t_Frame )
{
    pSrcBuff = &rSrcBuff;
    rows = rSrcBuff.rows;
    cols = rSrcBuff.cols;
    numFrames = NumFrames;// still image or animated
    frameIter = 0;
    tElapFrame = 0.0f;
    tFrame = t_Frame;
    isPlaying = numFrames > 1;
}

Light BufferSource::getBlendedLight( int n )const
{
    int nextFrIter = ( 1 + frameIter )%numFrames;// playForward true
    if( !playForward )
    {
        if( frameIter > 0 ) nextFrIter = frameIter - 1;
        else nextFrIter = numFrames - 1;
    }

    float U = tElapFrame/tFrame;
    if( U > 1.0f ) U = 1.0f;// next: 0 to 1
    float V = 1.0f - U;// current: 1 to 0

    // blend = next*U + current*V
    float fr = ( pSrcBuff[ nextFrIter ].pLt0[n].r )*U + ( pSrcBuff[ frameIter ].pLt0[n].r )*V;
    float fg = ( pSrcBuff[ nextFrIter ].pLt0[n].g )*U + ( pSrcBuff[ frameIter ].pLt0[n].g )*V;
    float fb = ( pSrcBuff[ nextFrIter ].pLt0[n].b )*U + ( pSrcBuff[ frameIter ].pLt0[n].b )*V;
    return Light( fr, fg, fb );
}
//                                   col          row
void BufferSource::setupScroll( float SpeedX, float SpeedY )
{
    fillShifted = true;
    // from Left
    vel.x = -SpeedX;// cShift decreases for scroll left
    if( SpeedX == 0.0f )// scrolling vertical
    {
        cShift = 0;
        pos.x = 0.0f;
    }
    pos.x = cShift;

    // from Up
    vel.y = -SpeedY;// rShift decreases for scroll up
    if( SpeedY == 0.0f )// scrolling horizontal
    {
        rShift = 0;
        pos.y = 0.0f;
    }
    pos.y = rShift;
}

void BufferSource::updateScroll( float dt )
{
    // from Left
    if( vel.x != 0.0f )
    {
        pos.x += vel.x*dt;// cShift
        if( pos.x + cols < 0.0f )// Left then next frame. vel.x < 0.0f
        {
            nextFrame();
            pos.x += cols;//  pos.y = 0;
        }
        else if( pos.x >= cols )// Right then previous frame. vel.x > 0.0f
        {
            prevFrame();
            pos.x -= cols;//   pos.y = 0;
        }
        cShift = (int)pos.x;
    }
    // from Up
    if( vel.y != 0.0f )
    {
        pos.y += vel.y*dt;// rShift
        if( pos.y + rows < 0.0f )// up then next frame
        {
            nextFrame();
            pos.y += rows;//  pos.y = 0;
        }
        else if( pos.y >= rows )// down then previous frame
        {
            prevFrame();
            pos.y -= rows;//   pos.y = 0;
        }
        rShift = (int)pos.y;
    }
}

// smooth shifting with fill. Uses vec2f members pos and vel
void BufferSource::setupScroll_Left( float Speed )// in columns per second. Speed < 0 <-> scroll right
{
    pos.x = cShift;
    vel.x = -Speed;// cShift decreases for scroll left
    fillShifted = true;
    rShift = 0;
    pos.y = 0.0f;
}

void BufferSource::updateScroll_Left( float dt )
{
    if( vel.x == 0.0f ) return;
    pos.x += vel.x*dt;// cShift
    if( pos.x + cols < 0.0f )// Left then next frame. vel.x < 0.0f
    {
        nextFrame();
        pos.x += cols;
      //  pos.y = 0;
    }
    else if( pos.x >= cols )// Right then previous frame. vel.x > 0.0f
    {
        prevFrame();
        pos.x -= cols;
     //   pos.y = 0;
    }

    cShift = (int)pos.x;
}

void BufferSource::setupScroll_Up( float Speed )// in columns per second. Speed < 0 <-> scroll down
{
    pos.y = rShift;
    vel.y = -Speed;// rShift decreases for scroll up
    fillShifted = true;
    cShift = 0;
}

void BufferSource::updateScroll_Up( float dt )
{
    if( vel.y == 0.0f ) return;
    pos.y += vel.y*dt;// rShift
    if( pos.y + rows < 0.0f )// up then next frame
    {
        nextFrame();
        pos.y += rows;
      //  pos.y = 0;
    }
    else if( pos.y >= rows )// down then previous frame
    {
        prevFrame();
        pos.y -= rows;
     //   pos.y = 0;
    }

    rShift = (int)pos.y;
}

void BufferSource::drawShifted()const
{
    if( isInGrid() )
    {
        Light* pLtBase = pTgt->pLt0 + row0*pTgt->cols + col0;
        for( int r = 0; r < rows; ++r )
        {
            int R = r;
            int frItR = frameIter;

            // working
            if( r - rShift < 0 )// rShift > 0 = shifted down. Show previous frame
            {
                if( fillShifted )// && numFrames > 1 )
                {
                    R += rows;
                    if( R - rShift < 0 ) continue;
                    frItR = getPrevFrameIter();
                }
                else continue;// may become positive
            }// works
            else if( r - rShift >= rows )// rShift < 0 = shifted up. Show next frame
            {
           //     if( !fillShifted || numFrames == 1 ) break;// no fill available
           //     if( r + rows - rShift >= rows ) break;// too far
           //     R -= rows;
           //     frIt = getNextFrameIter();

                if( fillShifted )// && numFrames > 1 )
                {
                    R -= rows;
                    if( R - rShift >= rows ) break;
                    frItR = getNextFrameIter();
                }
                else break;// will only get higher
            }

            Light* pRowTgt = pLtBase + r*pTgt->cols;
        //    int cCount = 0;// desperate tshoot measure
            for( int c = 0; c < cols; ++c )
            {
                int C = c;
             //   if( rShift == 0 ) frIt = frameIter;// reset each iteration
                int frIt = frItR;// reset each iteration
                // working
                if( c - cShift < 0 )// shifted right ( cShift > 0 ). Show previous frame
                {
                    if( fillShifted )// && numFrames > 1 )
                    {
                        C += cols;
                    //    frIt = getPrevFrameIter();
                        // prev to frItR
                        frIt = ( frItR == 0 ) ? numFrames - 1 : frItR - 1;
                        if( C - cShift < 0 ) continue;
                    }
                    else continue;// may become positive
                }// working
                else if( c - cShift >= cols )// shifted left ( cShift < 0 ). Show next frame
                {
                    if( fillShifted )// && numFrames > 1 )
                    {
                        C -= cols;
                        if( C - cShift >= cols ) break;
                     //   frIt = getNextFrameIter();
                        // next to frItR
                        frIt = ( 1 + frItR )%numFrames;
                    }
                    else break;// will only get higher
                }

                int N = ( R - rShift )*cols + C - cShift;
                pRowTgt[c] = pSrcBuff[ frIt ].pLt0[N];
             //   if( r == 0 && frIt != frameIter ) std::cout << "\nfrIt: " << frIt << " frameIter: " << frameIter;
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
            int N = ( r - rShift )*cols + c - cShift;
            pRowTgt[c] = pSrcBuff[ frameIter ].pLt0[N];
        }
    }
}
