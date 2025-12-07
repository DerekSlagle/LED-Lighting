#ifndef BUFFERSOURCE_H
#define BUFFERSOURCE_H

#include "LightSource.h"

// source and target are a LightGrid
class BufferSource : public LightSource
{
    public:
    LightGrid* pSrcBuff = nullptr;// must point to rows*cols Lights (at least)
    virtual Light getLt( int r, int c )const
    {
        if( !pSrcBuff ) return Light();
        if( r < 0 || r >= pSrcBuff->rows ) return Light();// r out of range
        if( c < 0 || r >= pSrcBuff->cols ) return Light();// c out of range

        return pSrcBuff->pLt0[ r*cols + c ];
    }

    void setSource( LightGrid& rSrcBuff )
    {
        pSrcBuff = &rSrcBuff;
        rows = rSrcBuff.rows;
        cols = rSrcBuff.cols;
    }

    BufferSource(){}
    virtual ~BufferSource(){}

};

#endif // BUFFERSOURCE_H
