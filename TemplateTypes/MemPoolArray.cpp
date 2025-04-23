#include "MemPoolArray.h"

// binds by finding pBase == pPool->ppBlock[ poolIdx ]
// will take a spot if any ppBlock == nullptr ( not ib use )
template<class T>
bool MemPoolArray<T>::init( MemoryPool<T>* p_Pool, unsigned int Size )
{
    if( !p_Pool ) return false;
    if( pBase ) return true;// already holding

    pPool = nullptr;// new
    poolIdx = 0;// new

    // find my place
    for( unsigned int k = 0; k < p_Pool->numUsers; ++k )
    {
        if( !( p_Pool->ppBlock[k] || p_Pool->pBlockSz[k] ) ) continue;// not in use

        if( ( p_Pool->ppBlock[k] == &pBase ) && ( p_Pool->pBlockSz[k] == &Capacity ) )
        {
            poolIdx = k;
            pPool = p_Pool;
            return pPool->Alloc( poolIdx, Size );
        }
    }

    // take an open spot if none were assigned
    for( unsigned int k = 0; k < p_Pool->numUsers; ++k )
    {
        if( p_Pool->ppBlock[k] || p_Pool->pBlockSz[k] ) continue;
        // use this one
        poolIdx = k;
        p_Pool->ppBlock[k] = &pBase;
        p_Pool->pBlockSz[k] = &Capacity;
        return pPool->Alloc( poolIdx, Size );
    }
    // out of luck
    return false;
}

template<class T>
MemPoolArray<T>& MemPoolArray<T>::operator = ( const MemPoolArray& V )// assign
{
    if( !( pPool && V.pPool ) ) return *this;// ??
    // is a block held?
    if( pBase )
    {
        if( Capacity >= V.Size )// room enough in the current block
        {
            for( unsigned int n = 0; n < V.Size; ++n )
                *( pBase + n ) = *( V.pBase + n );
            return *this;
        }
        pPool->Free( poolIdx );// too small
    }

    // pBase should = nullptr after call to Free()
    pPool->Alloc( poolIdx, V.Size );// may not succeed

    return *this;
}

template<class T>
MemPoolArray<T>::MemPoolArray( const MemPoolArray& V )// copy
{
    if( !V.pPool ) return;
     init( V.pPool );
    *this = V;
}

// move assign and copy ctor
template<class T>// V must be in same pool as *this
MemPoolArray<T>& MemPoolArray<T>::operator = ( MemPoolArray&& V )// move assign
{
    if( !( pPool && V.pPool ) || ( pPool != V.pPool ) ) return *this;// ??

    if( pBase ) pPool->Free( poolIdx );// release held block, if any

    // take ownership of Vs block
    pBase = V.pBase;
    Capacity = V.Capacity;
    // Free Vs block
    // free Vs pool pointers
    if( poolIdx == V.poolIdx )// here from move copy
    {
        V.pPool->ppBlock[ poolIdx ] = &pBase;
        V.pPool->pBlockSz[ poolIdx ] = &Capacity;
    }
    else
    {
        V.pPool->ppBlock[ V.poolIdx ] = nullptr;
        V.pPool->pBlockSz[ V.poolIdx ] = nullptr;
    }

    return *this;
}

template<class T>
MemPoolArray<T>::MemPoolArray( MemPoolArray&& V )// move copy
{
     if( !V.pPool ) return;
     pPool = V.pPool;
     poolIdx = V.poolIdx;// taking Vs
     *this = V;
}
