#ifndef BYTEPOOL2_H
#define BYTEPOOL2_H

#include<stdint.h>

typedef uint8_t Byte;

class BytePool2
{
    public:
    // ** common to both models **
    // the data pool
    Byte* pByte = nullptr;
    unsigned int poolSz = 0;

    // shared by an array of uint8_t pointers
    Byte*** ppBlock = nullptr;
    unsigned int** pBlockSz = nullptr;// each block size
    unsigned int numUsers = 0;// size of both arrays above

    // use with external pointer arrays
    void init( Byte* p_Byte, unsigned int numBytes, Byte*** pp_Block, unsigned int** p_BlockSz, unsigned int NumUsers );

    BytePool2( Byte* p_Byte, unsigned int numBytes, Byte*** pp_Block, unsigned int** p_BlockSz, unsigned int NumUsers )
    { init( p_Byte, numBytes, pp_Block, p_BlockSz, NumUsers ); }

    //or bake the pointers into the pool
  //  void initBaked( Byte* p_Byte, unsigned int numBytes, unsigned int NumUsers );
  //  BytePool2( Byte* p_Byte, unsigned int numBytes, unsigned int NumUsers ){ initBaked( p_Byte, numBytes, NumUsers ); }

    bool Alloc( Byte*& p_Block, unsigned int& Capacity, int ArrSz )const;
    void Free( Byte*& p_Block, unsigned int& Capacity )const;
    uint8_t DeFrag()const;

    // get info
    unsigned int BytesHeld()const;

    BytePool2(){}
    ~BytePool2(){}

    protected:

    private:
};

#endif // BYTEPOOL2_H
