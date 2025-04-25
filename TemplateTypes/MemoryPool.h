#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <iostream>// for Report()

template<class T>
class MemoryPool
{
    public:
    // the data pool
    T* pBase = nullptr;
    unsigned int poolSz = 0;// # of elements

    // shared by an array of T pointers
    T*** ppBlock = nullptr;
    unsigned int** pBlockSz = nullptr;// each block size
    unsigned int numUsers = 0;// size of both arrays above

    // use with external pointer arrays
    void init( T* p_Base, unsigned int ArrSz, T*** pp_Block, unsigned int** p_BlockSz, unsigned int NumUsers );
    //or bake the pointers into the pool
    void initBaked( T* p_Base, unsigned int ArrSz, unsigned int NumUsers );

    // usage
    // returns true if there is another block
    bool getNextBlockIdx( T* lowBlock, unsigned int& nextIdx )const;
    // find and allocate block
    bool Alloc( unsigned int idx, unsigned int ArrSz )const;
    void Free( unsigned int idx )const;// make block available
    unsigned int DeFrag()const;// returns number of blocks shifted back
    // find an unused pair
    bool getFreeIndex( unsigned int& idx )const;// writes index to idx
    // lookup the poolIdx
    bool getMyPoolIndex( const T*& p_Base, unsigned int& idx )const;// writes index to idx
    // given registered T*
    bool Alloc( T*& p_Base, unsigned int ArrSz )const
    {
        unsigned int idx;
        if( !getMyPoolIndex( p_Base, idx ) ) return false;
        return Alloc( idx, ArrSz );
    }

    void Report( std::ostream& os, const char* typeName )const;

    // external arrays
    MemoryPool( T* p_Base, unsigned int ArrSz, T*** pp_Block, unsigned int** p_BlockSz, unsigned int NumUsers )
    { init( p_Base, ArrSz, pp_Block, p_BlockSz, NumUsers ); }
    // baked into pool
    MemoryPool( T* p_Base, unsigned int ArrSz, unsigned int NumUsers )
    { initBaked( p_Base, ArrSz, NumUsers ); }

    MemoryPool(){}
    ~MemoryPool(){}

    protected:

    private:
};

/*
template<class T>
class MemoryPoolEx
{
    public:
    MemoryPool<T> memPool;
    // 1st portion is for storage of pointers
    T* pArr0 = nullptr;// 1st element in pool
    unsigned int ptrBlockSz = 0;// then memPool.pBase = pArr0 + ptrBlockSz;

    void initBaked( T* p_Base, unsigned int ArrSz, unsigned int NumUsers );
};
*/

template<class T>
class PoolArray
{
     public:
    MemoryPool<T>* pPool = nullptr;
    unsigned int poolIdx = 0;
    // the array data
    T* pBase = nullptr;
    unsigned int Capacity = 0;

    T& operator[]( unsigned int n )const { return *( pBase + n ); }
    T& getElement( unsigned int n )const { return *( pBase + n ); }

    bool Bind( MemoryPool<T>* p_Pool, unsigned int Size = 0 );
    void unBind();

    PoolArray(){}
    PoolArray( MemoryPool<T>* p_Pool, unsigned int Size = 0 ){ Bind( p_Pool, Size ); }

    PoolArray& operator = ( const PoolArray& V );// assign
    PoolArray( const PoolArray& V );// copy

    ~PoolArray()
    { // release the pool pointers for other use
        if( pPool && pBase )
        {
        //  storage is also freed. The blocks are gone
            pPool->ppBlock[ poolIdx ] = nullptr;
            pPool->pBlockSz[ poolIdx ] = nullptr;
        }
    }
};

#endif // MEMORYPOOL_H
