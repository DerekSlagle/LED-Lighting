#ifndef LIGHTPOOL_H
#define LIGHTPOOL_H

#include<stdint.h>
#include "Light.h"

class LightPool
{
    public:
    // the data pool
    Light* pLt0 = nullptr;
    unsigned int poolSz = 0;

    Light*** ppBlock = nullptr;
    unsigned int** pBlockSz = nullptr;// each block size
    unsigned int numUsers = 0;// size of both arrays above

    // use with external pointer arrays
    void init( Light* p_Lt0, unsigned int PoolSz, Light*** pp_Block, unsigned int** p_BlockSz, unsigned int NumUsers );

    LightPool( Light* p_Lt0, unsigned int PoolSz, Light*** pp_Block, unsigned int** p_BlockSz, unsigned int NumUsers )
    { init( p_Lt0, PoolSz, pp_Block, p_BlockSz, NumUsers ); }

    bool Alloc( Light*& p_Block, unsigned int& Capacity, int ArrSz )const;
    void Free( Light*& p_Block, unsigned int& Capacity )const;
    void DeFrag()const;
    unsigned int BytesHeld()const;

    LightPool(){}
    ~LightPool(){}
};

#endif // LIGHTPOOL_H
