#ifndef LIGHTPOOL_H
#define LIGHTPOOL_H

#include <iostream>
#include <string>
#include "Light.h"


class LightPool
{
    public:
    // the data pool
    Light* pBase = nullptr;
    unsigned int poolSz = 0;

    // shared by an array of uint8_t pointers
    Light*** ppBlock = nullptr;
    unsigned int** pBlockSz = nullptr;// each block size
    uint8_t numUsers = 0;// size of both arrays above

    void init( Light* p_Base, unsigned int numBytes );
    bool bindUsers( Light*** pp_Block, unsigned int** p_BlockSz, uint8_t NumUsers );

    // usage
    // returns true if there is another block
    bool getNextBlockIdx( Light* lowBlock, uint8_t& nextIdx )const;
    // find and allocate block
    bool Alloc( uint8_t idx, unsigned int ArrSz )const;
    void Free( uint8_t idx )const;// make block available
    uint8_t DeFrag()const;// returns number of blocks shifted back

    void Report( std::ostream& os )const;

    LightPool( Light* p_Base, unsigned int numBytes ){ init( p_Base, numBytes ); }

    LightPool();
    ~LightPool();

    protected:

    private:
};

#endif // LIGHTPOOL_H
