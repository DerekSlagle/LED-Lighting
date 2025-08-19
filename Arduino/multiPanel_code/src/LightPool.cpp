#include "LightPool.h"


void LightPool::init( Light* p_Lt0, unsigned int PoolSz, Light*** pp_Block, unsigned int** p_BlockSz, unsigned int NumUsers )
{
    if( !p_Lt0 ) return;
    pLt0 = p_Lt0;
    poolSz = PoolSz;

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

bool LightPool::Alloc( Light*& p_Block, unsigned int& Capacity, int ArrSz )const
{
    if( ArrSz <= 0 ) return false;
    if( ArrSz > (int)poolSz ) return false;

    Free( p_Block, Capacity );

    // The following code MUST BE simple and compact
    // Target = 20 loc. Hard Limit = 40 loc
    int gap = poolSz;
    Light* pBase = pLt0;
    unsigned int n = 0;// insert user here
    while( n < numUsers )
    {
        if( ppBlock[n] )// find gap behind block n
            gap = (int)( *ppBlock[n] - pBase );
        else// to end of pool
            gap = (int)( pLt0 + poolSz - pBase );

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

void LightPool::Free( Light*& p_Block, unsigned int& Capacity )const
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

void LightPool::DeFrag()const
{
    if( !ppBlock[0] ) return;// entire pool is free
    Light* pBase = pLt0;
    for( unsigned int n = 0; n < numUsers; ++n )
    {
        if( !ppBlock[n] ) return;// Done!
        int gap = (int)( *ppBlock[n] - pBase );
        if( gap > 0 )// shift block back by gap elements to start at pBase
        {
            for( unsigned int k = 0; k < *pBlockSz[n]; ++k )
                *( pBase + k ) = *( *ppBlock[n] + k );
            // write new pByte to user
            *ppBlock[n] = pBase;
        }
        // for next iter
        pBase = *ppBlock[n] + *pBlockSz[n];
    }

    return;
}

unsigned int LightPool::BytesHeld()const
{
    unsigned int BH = 0;
    for( unsigned int n = 0; n < numUsers; ++n )
    {
        if( pBlockSz[n] ) BH += *pBlockSz[n];
        else break;// no more blocks held
    }

    return BH;
}