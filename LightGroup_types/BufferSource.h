#ifndef BUFFERSOURCE_H
#define BUFFERSOURCE_H

#include "LightSource.h"

// source and target are a LightGrid
class BufferSource : public LightSource
{
    public:
    LightGrid* pSrcBuff = nullptr;// must point to rows*cols Lights (at least)
    int frameIter = 0, numFrames = 1;
    bool isPlaying = true;// stops animation
    // timing frame rate
    float tFrame = 0.04f, tElapFrame = 0.0f;// 25 fps
    bool update( float dt );// true when frame changes. Also updates position
    void nextFrame()
    {
        frameIter = ( 1 + frameIter )%numFrames;
    }
    void prevFrame();

    virtual Light getLt( int r, int c )const;
    void setSource( LightGrid& rSrcBuff, int NumFrames = 1, float t_Frame = 0.05f );

    // fade into next frame
    bool doBlend = false;
    bool playForward = true;
    Light getBlendedLight( int n )const;

    BufferSource(){}
    virtual ~BufferSource(){}

};

#endif // BUFFERSOURCE_H
