#ifndef BUFFERSOURCE_H
#define BUFFERSOURCE_H

#include "LightSource.h"

// source and target are a LightGrid
class BufferSource : public LightSource
{
    public:
    LightGrid* pSrcBuff = nullptr;// must point to rows*cols Lights (at least)
  //  int frameIter = 0, numFrames = 1;
 //   bool isPlaying = true;// stops animation
    // timing frame rate
//    float tFrame = 0.04f, tElapFrame = 0.0f;// 25 fps
// base version does all needed at this time
 //   virtual bool update( float dt );// true when frame changes. Also updates position

    virtual void nextFrame()
    {
        frameIter = ( 1 + frameIter )%numFrames;
        rows = pSrcBuff[frameIter].rows;
        cols = pSrcBuff[frameIter].cols;
    }

    virtual void prevFrame()
    {
        if( frameIter > 0 ) --frameIter;
        else frameIter = numFrames - 1;
        rows = pSrcBuff[frameIter].rows;
        cols = pSrcBuff[frameIter].cols;
    }

    int getNextFrameIter()const { return ( 1 + frameIter )%numFrames; }
    int getPrevFrameIter()const
    {
        if( frameIter > 0 ) return frameIter - 1;
        return numFrames - 1;
    }

    virtual Light getLt( int r, int c )const;
    void setSource( LightGrid& rSrcBuff, int NumFrames = 1, float t_Frame = 0.05f );
    // void setTarget( LightGrid& tgtGrid ) is in base

    // fade into next frame
    bool doBlend = false;
//    bool playForward = true;
    Light getBlendedLight( int n )const;
    // fill shifted frame from next or previous frame
    bool fillShifted = false;
    virtual void drawShifted()const;// called by draw()
    // smooth shifting with fill. Uses vec2f members pos and vel
    
    // new combination functions
    void setupScroll( float SpeedX, float SpeedY );
    void updateScroll( float dt );

    void setupScroll_Left( float Speed );// in columns per second. Speed < 0 <-> scroll right
    void updateScroll_Left( float dt );
    void setupScroll_Up( float Speed );// in columns per second. Speed < 0 <-> scroll down
    void updateScroll_Up( float dt );

    BufferSource(){}
    virtual ~BufferSource(){}

};

#endif // BUFFERSOURCE_H
