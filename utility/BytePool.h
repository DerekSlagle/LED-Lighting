#ifndef BYTEPOOL_H
#define BYTEPOOL_H

#include <iostream>
#include <string>
#include<stdint.h>

class BytePool
{
    public:
    // the data pool
    uint8_t* pByte = nullptr;
    unsigned int poolSz = 0;

    // shared by an array of uint8_t pointers
    uint8_t*** ppBlock = nullptr;
    unsigned int** pBlockSz = nullptr;// each block size
    uint8_t numUsers = 0;// size of both arrays above

    void init( uint8_t* p_Byte, unsigned int numBytes );
    bool bindUsers( uint8_t*** pp_Block, unsigned int** p_BlockSz, uint8_t NumUsers );

    // usage
    // returns true if there is another block
    bool getNextBlockIdx( uint8_t* lowBlock, uint8_t& nextIdx )const;
    // find and allocate block
    bool Alloc( uint8_t idx, unsigned int ArrSz )const;
    void Free( uint8_t idx )const;// make block available
    uint8_t DeFrag()const;// returns number of blocks shifted back

    void Report( std::ostream& os )const;

    BytePool( uint8_t* p_Byte, unsigned int numBytes ){ init( p_Byte, numBytes ); }
    BytePool();
    ~BytePool();

    protected:

    private:
};

#endif // BYTEPOOL_H
