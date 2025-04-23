#ifndef MEMPOOLARRAY_H
#define MEMPOOLARRAY_H

#include "MemoryPool.h"
#include "MemoryPool.cpp"

template<class T>
class MemPoolArray
{
    public:
    MemoryPool<T>* pPool = nullptr;
    unsigned int poolIdx = 0;

    T* pBase = nullptr;// block owner
    unsigned int Capacity = 0;// allocated block size

    T& operator[]( unsigned int n )const { return *( pBase + n ); }

    MemPoolArray& operator = ( const MemPoolArray& V );// assign
    MemPoolArray( const MemPoolArray& V );// copy
    // move assign and copy ctor
    MemPoolArray& operator = ( MemPoolArray&& V );// move assign
    MemPoolArray( MemPoolArray&& V );// move copy

    // binds by finding pBase == pPool->ppBlock[ poolIdx ]
    // will take a spot if any ppBlock == nullptr ( not ib use )
    bool init( MemoryPool<T>* p_Pool, unsigned int Size );// fixed size
    MemPoolArray( MemoryPool<T>* p_Pool ){ init( p_Pool ); }
    ~MemPoolArray(){ if( pPool && pBase ) pPool->Free( poolIdx ); }

    protected:
    private:
};

#endif // MEMPOOLARRAY_H
