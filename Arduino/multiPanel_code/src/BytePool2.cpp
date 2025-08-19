#include "BytePool2.h"


void BytePool2::init( Byte* p_Byte, unsigned int numBytes, Byte*** pp_Block, unsigned int** p_BlockSz, unsigned int NumUsers )
{
    if( !p_Byte ) return;
    pByte = p_Byte;
    poolSz = numBytes;

    if( !pp_Block ) return;
    ppBlock = pp_Block;

    if( !p_BlockSz ) return;
    pBlockSz = p_BlockSz;
    numUsers = NumUsers;

    // init all to nullptr
    for( unsigned int n = 0; n < numUsers; ++n )
    {
        ppBlock[n] = nullptr;
        pBlockSz[n] = nullptr;
    }
}

/*
// The pool arrays are stored in the pool.
void BytePool2::initBaked( Byte* p_Byte, unsigned int numBytes, unsigned int NumUsers )
{
    unsigned int ptrSz = sizeof( int* );// all are same size
 //   unsigned int Tsz = 1;//sizeof( T );//
    unsigned int hfBlockSz = (ptrSz*NumUsers);// # of elements to store NumUsers pointers
 //   if( ( ptrSz*NumUsers )%Tsz > 0 ) ++hfBlockSz;// for the remainder

    unsigned int ptrBlockSz = 2*hfBlockSz;
 //   init( p_Base + ptrBlockSz, ArrSz - ptrBlockSz );// Capacity for T storage = ArrSz -  ptrBlockSz
    pByte = p_Byte + ptrBlockSz;
    poolSz = numBytes - ptrBlockSz;
    numUsers = NumUsers;

    // bake the 1st array in
    ppBlock = new ( p_Byte ) Byte**[ NumUsers ];// pointer to user pBase
    if( ppBlock )
        for( unsigned int n = 0; n < NumUsers; ++n ) ppBlock[n] = nullptr;
    // bake the 2nd array in
    pBlockSz = new ( p_Byte + hfBlockSz ) unsigned int*[ NumUsers ];// pointer to user Capacity
    if( pBlockSz )
        for( unsigned int n = 0; n < NumUsers; ++n ) pBlockSz[n] = nullptr;
}
*/

bool BytePool2::Alloc( Byte*& p_Block, unsigned int& Capacity, int ArrSz )const
{
    if( ArrSz <= 0 ) return false;
    if( ArrSz > (int)poolSz ) return false;

    Free( p_Block, Capacity );

    // The following code MUST BE simple and compact
    // Target = 20 loc. Hard Limit = 40 loc
    int gap = poolSz;
    Byte* pBase = pByte;
    unsigned int n = 0;// insert user here
    while( n < numUsers )
    {
        if( ppBlock[n] )// find gap behind block n
            gap = (int)( *ppBlock[n] - pBase );
        else// to end of pool
            gap = (int)( pByte + poolSz - pBase );

        if( ArrSz <= gap )// a fit
        {
            p_Block = pBase;
            Capacity = ArrSz;
            // insert as element n
            for( unsigned int m = numUsers - 1; m > n; --m )
            {
                ppBlock[m] = ppBlock[m-1];
                pBlockSz[m] = pBlockSz[m-1];
            }
            // register
            ppBlock[n] = &p_Block;
            pBlockSz[n] = &Capacity;
            return true;
        }

        pBase = *ppBlock[n] + *pBlockSz[n];
        ++n;
    }// 29 loc. OK

    return false;
}

void BytePool2::Free( Byte*& p_Block, unsigned int& Capacity )const
{
    // free if held
    unsigned int n = 0;
    for( n = 0; n < numUsers; ++n )
        if( ppBlock[n] == &p_Block )// found
        {
            ppBlock[n] = nullptr;
            pBlockSz[n] = nullptr;
            // "extract" element n
            for( unsigned int m = n; m+1 < numUsers; ++m )
            {
                ppBlock[m] = ppBlock[m+1];
                pBlockSz[m] = pBlockSz[m+1];
            }
            // last should = nullptr
            ppBlock[ numUsers - 1 ] = nullptr;
            pBlockSz[ numUsers - 1 ] = nullptr;
        }

    p_Block = nullptr;
    Capacity = 0;
}

Byte BytePool2::DeFrag()const// returns ?
{
    if( !ppBlock[0] ) return 0;// entire pool is free
    Byte shiftCount = 0;
    Byte* pBase = pByte;
    for( unsigned int n = 0; n < numUsers; ++n )
    {
        if( !ppBlock[n] ) return shiftCount;// Done!
        int gap = (int)( *ppBlock[n] - pBase );
        if( gap > 0 )// shift block back by gap elements to start at pBase
        {
            ++shiftCount;
            for( unsigned int k = 0; k < *pBlockSz[n]; ++k )
                *( pBase + k ) = *( *ppBlock[n] + k );
            // write new pByte to user
            *ppBlock[n] = pBase;
        }
        // for next iter
        pBase = *ppBlock[n] + *pBlockSz[n];
    }

    return shiftCount;
}

unsigned int BytePool2::BytesHeld()const
{
    unsigned int BH = 0;
    for( unsigned int n = 0; n < numUsers; ++n )
    {
        if( pBlockSz[n] ) BH += *pBlockSz[n];
        else break;// no more blocks held
    }

    return BH;
}