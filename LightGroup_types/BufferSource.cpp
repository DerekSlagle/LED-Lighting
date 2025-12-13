#include "BufferSource.h"

bool BufferSource::update( float dt )// true when frame changes
{
    if( isMoving ) updatePosition(dt);
    if( !isPlaying ) return false;
    if( numFrames == 1 ) return false;// just displaying 1 image

    tElapFrame += dt;
    if( tElapFrame >= tFrame )
    {
        tElapFrame = 0.0f;
        if( playForward ) nextFrame();
        else prevFrame();
        return true;
    }
    return false;
}

void BufferSource::prevFrame()
{
    if( frameIter > 0 ) --frameIter;
    else frameIter = numFrames - 1;
}

Light BufferSource::getLt( int r, int c )const
{
    if( !pSrcBuff ) return Light();
    const LightGrid* pLG = pSrcBuff + frameIter;
    if( r < 0 || r >= pLG->rows ) return Light();// r out of range
    if( c < 0 || r >= pLG->cols ) return Light();// c out of range

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
        nextFrIter = frameIter - 1;
        if( nextFrIter < 0 ) nextFrIter = numFrames - 1;
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
