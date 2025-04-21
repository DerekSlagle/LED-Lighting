#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <iostream>// for Report()
#include <string>// for Report()

template<class T>
class MemoryPool
{
    public:
    // the data pool
    T* pBase = nullptr;
    unsigned int poolSz = 0;

    // shared by an array of T pointers
    T*** ppBlock = nullptr;
    unsigned int** pBlockSz = nullptr;// each block size
    unsigned int numUsers = 0;// size of both arrays above

    void init( T* p_Base, unsigned int numBytes );
    bool bindUsers( T*** pp_Block, unsigned int** p_BlockSz, unsigned int NumUsers );

    // usage
    // returns true if there is another block
    bool getNextBlockIdx( T* lowBlock, unsigned int& nextIdx )const;
    // find and allocate block
    bool Alloc( unsigned int idx, unsigned int ArrSz )const;
    void Free( unsigned int idx )const;// make block available
    unsigned int DeFrag()const;// returns number of blocks shifted back

    void Report( std::ostream& os, const char* typeName )const;

    MemoryPool( T* p_Base, unsigned int numBytes ){ init( p_Base, numBytes ); }

    MemoryPool(){}
    ~MemoryPool(){}

    protected:

    private:
};

#endif // MEMORYPOOL_H
