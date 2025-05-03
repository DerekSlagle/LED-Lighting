#ifndef BYTEPOOL_H
#define BYTEPOOL_H

#include <iostream>
#include<stdint.h>

class BytePool
{
    public:
    // ** common to both models **
    // the data pool
    uint8_t* pByte = nullptr;
    unsigned int poolSz = 0;

    // shared by an array of uint8_t pointers
    uint8_t*** ppBlock = nullptr;
    unsigned int** pBlockSz = nullptr;// each block size
    uint8_t numUsers = 0;// size of both arrays above

    // use with external pointer arrays
    void init( uint8_t* p_Byte, unsigned int numBytes, uint8_t*** pp_Block, unsigned int** p_BlockSz, unsigned int NumUsers );
    BytePool( uint8_t* p_Byte, unsigned int numBytes, uint8_t*** pp_Block, unsigned int** p_BlockSz, unsigned int NumUsers )
    { init( p_Byte, numBytes, pp_Block, p_BlockSz, NumUsers ); }

    //or bake the pointers into the pool
    void initBaked( uint8_t* p_Byte, unsigned int numBytes, unsigned int NumUsers );
    BytePool( uint8_t* p_Byte, unsigned int numBytes, unsigned int NumUsers ){ initBaked( p_Byte, numBytes, NumUsers ); }

    // *** first model ***
    // find an unused pair
    bool getFreeIndex( unsigned int& idx )const;// writes index to idx
    // lookup the poolIdx
    bool getMyPoolIndex( const uint8_t*& p_Base, unsigned int& idx )const;// writes index to idx
    // returns true if there is another block
    bool getNextBlockIdx( uint8_t* lowBlock, uint8_t& nextIdx )const;
    // find and allocate block
    bool Alloc( uint8_t idx, unsigned int ArrSz )const;
    void Free( uint8_t idx )const;// make block available
    uint8_t DeFrag()const;// returns number of blocks shifted back
    void Report( std::ostream& os )const;

    // *** another model *** no index related functions required
    bool Alloc_ordered( uint8_t*& p_Block, unsigned int& Capacity, int ArrSz )const;
    void Free2( uint8_t*& p_Block, unsigned int& Capacity )const;
    uint8_t DeFrag2()const;
    void Report2( std::ostream& os )const;


    BytePool(){}
    ~BytePool(){}

    protected:

    private:
};

#endif // BYTEPOOL_H
