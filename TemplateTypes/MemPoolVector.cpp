#include "MemPoolVector.h"

// binds by finding pBase == pPool->ppBlock[ poolIdx ]
// will take a spot if any ppBlock == nullptr ( not ib use )
/*
template<class T>
bool MemPoolVector<T>::init( MemoryPool<T>* p_Pool )
{
    if( !p_Pool ) return false;
    pPool = nullptr;// new
    poolIdx = tempIdx = 0;// new
    bool found = false;

    // find my places
    for( unsigned int k = 0; k < p_Pool->numUsers; ++k )
    {
        if( !( p_Pool->ppBlock[k] || p_Pool->pBlockSz[k] ) ) continue;// not in use
        // found the 1st pair
        if( !found )
        {
            if( ( p_Pool->ppBlock[k] == &pBase ) && ( p_Pool->pBlockSz[k] == &Capacity ) )
            {
                poolIdx = k;
                found = true;
            }

        }
        else if( ( p_Pool->ppBlock[k] == &pBaseTemp ) && ( p_Pool->pBlockSz[k] == &tempCap ) )
        {
            pPool = p_Pool;
            tempIdx = k;
            return true;
        }
    }

    // take an open spot if none were assigned
    found = false;
    for( unsigned int k = 0; k < p_Pool->numUsers; ++k )
    {
        if( p_Pool->ppBlock[k] || p_Pool->pBlockSz[k] ) continue;
        // use this one
        if( !found )
        {
            poolIdx = k;
            found = true;
            p_Pool->ppBlock[k] = &pBase;
            p_Pool->pBlockSz[k] = &Capacity;
        }
        else
        {
            pPool = p_Pool;
            tempIdx = k;
            p_Pool->ppBlock[k] = &pBaseTemp;
            p_Pool->pBlockSz[k] = &tempCap;
            return true;
        }

    }
    // out of luck
    return false;
}
*/

template<class T>
bool MemPoolVector<T>::Bind( MemoryPool<T>* p_Pool, unsigned int Size )
{
    if( !p_Pool ) return false;
    pPool = nullptr;// new
    poolIdx = tempIdx = 0;// new
    bool found = false;

    // find my places
    for( unsigned int k = 0; k < p_Pool->numUsers; ++k )
    {
        if( !( p_Pool->ppBlock[k] || p_Pool->pBlockSz[k] ) ) continue;// not in use
        // found the 1st pair
        if( !found )
        {
            if( ( p_Pool->ppBlock[k] == &pBase ) && ( p_Pool->pBlockSz[k] == &Capacity ) )
            {
                poolIdx = k;
                found = true;
            }

        }
        else if( ( p_Pool->ppBlock[k] == &pBaseTemp ) && ( p_Pool->pBlockSz[k] == &tempCap ) )
        {
            pPool = p_Pool;
            tempIdx = k;
            if( Size > 0 ) return p_Pool->Alloc( poolIdx, Size );
            return true;
        }
    }

    // take an open spot if none were assigned
    found = false;
    for( unsigned int k = 0; k < p_Pool->numUsers; ++k )
    {
        if( p_Pool->ppBlock[k] || p_Pool->pBlockSz[k] ) continue;
        // use this one
        if( !found )
        {
            poolIdx = k;
            found = true;
        }
        else
        {
            pPool = p_Pool;
            p_Pool->ppBlock[ poolIdx ] = &pBase;
            p_Pool->pBlockSz[ poolIdx ] = &Capacity;
            tempIdx = k;
            p_Pool->ppBlock[k] = &pBaseTemp;
            p_Pool->pBlockSz[k] = &tempCap;
            if( Size > 0 ) return p_Pool->Alloc( poolIdx, Size );
            return true;
        }
    }
    // out of luck
    return false;
}

template<class T>
void MemPoolVector<T>::unBind()
{
    if( !pPool ) return;
    pPool->ppBlock[ poolIdx ] = nullptr;
    pPool->pBlockSz[ tempIdx ] = nullptr;
    pPool->ppBlock[ poolIdx ] = nullptr;
    pPool->pBlockSz[ tempIdx ] = nullptr;
    pPool = nullptr;
}

template<class T>
bool MemPoolVector<T>::push_back( const T& X )
{
    if( !pPool ) return false;

    if( !pBase )// 1st element
    {
        if( pPool->Alloc( poolIdx, CapBump ) )// initial Capacity
        {
            *pBase = X;
            Size = 1;
            return true;
        }
        return false;
    }

    if( Size < Capacity )
    {
        *( pBase + Size ) = X;
        ++Size;
        return true;
    }

    // increase Capacity
    if( tempIdx != poolIdx )// found a block* to use
    {
        if( pPool->Alloc( tempIdx, Capacity + CapBump ) )
        {
            for( unsigned int k = 0; k < Size; ++k )
                *( pBaseTemp + k ) = *( pBase + k );// copy existing values
            // add the new element and increment Size
            *( pBaseTemp + Size ) = X;
            ++Size;
            // keep same poolIdx
            pBase = pBaseTemp;// this releases the previous block
            Capacity = tempCap;

            // release block hold
            pBaseTemp = nullptr;
            tempCap = 0;

            return true;
        }

        // no room. alloc fail
        pBaseTemp = nullptr;
        tempCap = 0;
        return false;
    }

    return false;
}

template<class T>
bool MemPoolVector<T>::pop_back()// false if Size == 0 0r pBase == nullptr
{
    if( !( pPool && pBase ) ) return false;
    if( Size == 0 ) return false;
    --Size;
    return true;
}

// block allocation
template<class T>
bool MemPoolVector<T>::reserve( unsigned int Cap )
{
    if( !pPool ) return false;

    if( pBase )
    {
        if( Capacity >= Cap ) return true;// already good
        return pPool->Alloc( poolIdx, Cap );
    }
    // else
    return pPool->Alloc( poolIdx, Cap );
}

template<class T>
bool MemPoolVector<T>::resize( unsigned int newSize )
{
    if( !pPool ) return false;

    if( pBase && newSize <= Capacity )
    {
        Size = newSize;
        return true;
    }

    // else
    if( pPool->Alloc( poolIdx, newSize ) )
    {
        Size = newSize;
        return true;
    }

    return false;
}

template<class T>
MemPoolVector<T>& MemPoolVector<T>::operator = ( const MemPoolVector& V )// assign
{
    if( !( pPool && V.pPool ) ) return *this;// ??
    // is a block held?
    if( pBase )
    {
        if( Capacity >= V.Size )// room enough in the current block
        {
            Size = V.Size;
            CapBump = V.CapBump;
            for( unsigned int n = 0; n < V.Size; ++n )
                *( pBase + n ) = *( V.pBase + n );
            return *this;
        }
        pPool->Free( poolIdx );// too small
    }

    // pBase should = nullptr after call to Free()
    if( pPool->Alloc( poolIdx, V.Capacity ) )// may not succeed
    {
        CapBump = V.CapBump;
        Size = V.Size;
        for( unsigned int n = 0; n < V.Size; ++n )
            *( pBase + n ) = *( V.pBase + n );
    }

    return *this;
}

template<class T>
MemPoolVector<T>::MemPoolVector( const MemPoolVector& V )// copy
{
    if( !V.pPool ) return;
  //   init( V.pPool );
     Bind( V.pPool );
    *this = V;
}

// move assign and copy ctor
template<class T>// V must be in same pool as *this
MemPoolVector<T>& MemPoolVector<T>::operator = ( MemPoolVector&& V )// move assign
{
    if( !( pPool && V.pPool ) || ( pPool != V.pPool ) ) return *this;// ??

    if( pBase ) pPool->Free( poolIdx );// release held block, if any

    // take ownership of Vs block
    pBase = V.pBase;
    Capacity = V.Capacity;
    Size = V.Size;
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
MemPoolVector<T>::MemPoolVector( MemPoolVector&& V )// move copy
{
     if( !V.pPool ) return;
     pPool = V.pPool;
     poolIdx = V.poolIdx;// taking Vs
     tempIdx = V.tempIdx;// block
     *this = V;
}
