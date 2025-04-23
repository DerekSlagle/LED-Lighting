#ifndef MEMPOOLVECTOR_H
#define MEMPOOLVECTOR_H

#include "MemoryPool.h"
#include "MemoryPool.cpp"

// as two registered users of a MemoryPool
template<class T>
class MemPoolVector
{
    public:
    MemoryPool<T>* pPool = nullptr;
    unsigned int poolIdx = 0;

    T* pBase = nullptr;// block owner
    unsigned int Capacity = 0;// allocated block size
    unsigned int CapBump = 10;// Capacity increase on push_back() when out of space
    unsigned int Size = 0;// elements in use

    // the 2nd block for temp allocation
    T* pBaseTemp = nullptr;
    unsigned int tempCap = 0;
    unsigned int tempIdx = 0;// found on init

    bool push_back( const T& X );// false if re allocation fails
    bool pop_back();// false if Size == 0 0r pBase == nullptr
    void clear(){ Size = 0; }

    // block allocation
    bool reserve( unsigned int Cap );
    bool resize( unsigned int newSize );

    // access - not checked. Not safe
    T& front(){ return *pBase; }
    T& back(){ return *( pBase + Size - 1 ); }
    T& operator[]( unsigned int n ) { return *( pBase + n ); }// regardless of Size or Capacity


    MemPoolVector& operator = ( const MemPoolVector& V );// assign
    MemPoolVector( const MemPoolVector& V );// copy
    // move assign and copy ctor
    MemPoolVector& operator = ( MemPoolVector&& V );// move assign
    MemPoolVector( MemPoolVector&& V );// move copy

    // binds by finding pBase == pPool->ppBlock[ poolIdx ] and for tempIdx, pBaseTemp
    // will take a spot if any ppBlock == nullptr ( not ib use )
    bool init( MemoryPool<T>* p_Pool );
    MemPoolVector( MemoryPool<T>* p_Pool ){ init( p_Pool ); }
    ~MemPoolVector(){ if( pPool && pBase ) pPool->Free( poolIdx ); }

    protected:

    private:
};

#endif // MEMPOOLVECTOR_H
