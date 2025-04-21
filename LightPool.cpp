#include "LightPool.h"

LightPool::LightPool()
{
    //ctor
}

LightPool::~LightPool()
{
    //dtor
}

void LightPool::init( Light* p_Base, unsigned int numBytes )
{
    pBase = p_Base;
    poolSz = numBytes;
}

// verifies all pointers are != nullptr
bool LightPool::bindUsers( Light*** pp_Block, unsigned int** p_BlockSz, uint8_t NumUsers )
{
    if( !pp_Block ) return false;
    ppBlock = pp_Block;

    if( !p_BlockSz ) return false;
    pBlockSz = p_BlockSz;
    numUsers = NumUsers;

    // init with no storage
    for( uint8_t n = 0; n < numUsers; ++n )
    {
        if( ppBlock[n] ) *ppBlock[n] = nullptr;
        else return false;

        if( pBlockSz[n] ) *pBlockSz[n] = 0;
        else return false;
    }

 //   std::cerr << "\n bindUsers() check";
    return true;
}

bool LightPool::getNextBlockIdx( Light* lowBlock, uint8_t& nextIdx )const
{
    Light* nextBlock = nullptr;

    for( uint8_t n = 0; n < numUsers; ++n )
    {
   //     if( !pBlock[n] ) continue;
        if( !*ppBlock[n] ) continue;

        if( nextBlock )// is this one less?
        {
            if( lowBlock )
            {
                if( ( lowBlock < *ppBlock[n] ) && ( *ppBlock[n] < nextBlock ) )
                {
                    nextIdx = n;
                    nextBlock = *ppBlock[n];// 1st found greater was not least greater
                }
            }
            else if( *ppBlock[n] < nextBlock )
            {
                nextIdx = n;
                nextBlock = *ppBlock[n];// seeking lowest
            }
        }
        else
        {
            if( lowBlock )
            {
                if( lowBlock < *ppBlock[n] )
                {
                    nextIdx = n;
                    nextBlock = *ppBlock[n];// 1st found greater
                }
            }
            else// 1st found
            {
                nextIdx = n;
                nextBlock = *ppBlock[n];// 1st found
            }
        }
    }

 //   std::cerr << "\n getNextBlockIdx() check";
    return nextBlock != nullptr;
}

// find and allocate block
bool LightPool::Alloc( uint8_t idx, unsigned int ArrSz )const
{
    if( idx >= numUsers )
    {
        std::cerr << "\n LightPool::Alloc() idx >= numUsers: idx = " << idx << " numUsers = " << numUsers;
        return false;
    }

    // already holding?
    if( *ppBlock[ idx ] )
    {
        if( *pBlockSz[ idx ] >= ArrSz )// big enough
        {
            *pBlockSz[ idx ] = ArrSz;
            return true;
        }
        // free it
        Free( idx );
    }

    // find lowest pointer
    uint8_t nextIdx = 0;
    bool isAnotherBlock = getNextBlockIdx( nullptr, nextIdx );

    if( !isAnotherBlock )// entire array is free
    {
        if( ArrSz <= poolSz )// there is room
        {
            *ppBlock[ idx ] = pBase;
            *pBlockSz[ idx ] = ArrSz;
            std::cerr << "\n Alloc() 1st check";
            return true;
        }
        std::cerr << "\n Alloc() ArrSz > poolSz" << " ArrSz = " << ArrSz << " poolSz = " << poolSz;
        return false;
    }
    else// look deeper
    {
    //    while( true )
        while( *ppBlock[ nextIdx ] )
        {
            // 1 past end of given low block
        //    uint8_t* pBlock = lowBlock + low_pArr->arrSz;
            Light* pBlockLow = *ppBlock[ nextIdx ] + *pBlockSz[ nextIdx ];
            isAnotherBlock = getNextBlockIdx( *ppBlock[ nextIdx ], nextIdx );// 1st ele in next lowest block

            if( isAnotherBlock )// is there space between blocks?
            {
                int gapSz = *ppBlock[ nextIdx ] - pBlockLow;
                if( gapSz >= (int)ArrSz )// there is
                {
                    *ppBlock[ idx ] = pBlockLow;
                    *pBlockSz[ idx ] = ArrSz;
                    std::cerr << "\n Alloc() " << (unsigned int)nextIdx << " check";
                    return true;
                }

             //   low_pArr = next_pArr;// update for next iteration
            }
            else if( pBlockLow - pBase + ArrSz <= poolSz )// is remainder of pool enough?
            {
                *ppBlock[ idx ] = pBlockLow;
                *pBlockSz[ idx ] = ArrSz;
                std::cerr << "\n Alloc() end " << (unsigned int)nextIdx << " check";
                return true;
            }
            else// not enough
            {
                std::cerr << "\n Alloc() not enough";
                return false;
            }
        }// end while
    }

     std::cerr << "\n Alloc() false return at end of function";
    return false;
}

void LightPool::Free( uint8_t idx )const// make block available
{
    *ppBlock[idx] = nullptr;
    *pBlockSz[idx] = 0;
}

uint8_t LightPool::DeFrag()const// returns number of blocks shifted back
{

    // find lowest pointer
    uint8_t nextIdx = 0;
    bool isBlock = getNextBlockIdx( nullptr, nextIdx );

    if( !isBlock ) return 0;// entire pool is free
    uint8_t shiftCount = 0;
    int gapSz = 0;

    // there may be a block behind lowDP
    if( *ppBlock[ nextIdx ] > pBase )
    {
        gapSz = (int)( *ppBlock[ nextIdx ] - pBase );
        // copy back nextDP array data
        for( unsigned int n = 0; n < *pBlockSz[ nextIdx ]; ++n )
            *( pBase + n ) = *( *ppBlock[ nextIdx ] + n );
        // assign to 1st
        *ppBlock[ nextIdx ] = pBase;
        shiftCount = 1;
    }

    while( isBlock )
    {
        Light* pBlock = *ppBlock[ nextIdx ] + *pBlockSz[ nextIdx ];// start of next block

        isBlock = getNextBlockIdx( *ppBlock[ nextIdx ], nextIdx );
        if( !isBlock ) return shiftCount;// free to the end

        gapSz = (int)( *ppBlock[ nextIdx ] - pBlock );
        if( gapSz > 0 )
        {
            // copy back nextDP array data
            for( unsigned int n = 0; n < *pBlockSz[ nextIdx ]; ++n )
                *( pBlock + n ) = *( *ppBlock[ nextIdx ] + n );
            // assign to new array origin
            *ppBlock[ nextIdx ] = pBlock;
            ++shiftCount;
        }
    }

    return shiftCount;
}

void LightPool::Report( std::ostream& os )const
{
    os << "\n\n** report **  pBase = " << (int)pBase << '\n';

    int numHolding = 0;
    unsigned int szHeld = 0;
    for( uint8_t n = 0; n < numUsers; ++n )
    {
        if( *ppBlock[n] )
        {
            ++numHolding;
            szHeld += *pBlockSz[n];
    //        os << " n: " << (unsigned int)n;
        }
    }

    os << '\n' << numHolding << " holding " << szHeld << " of " << poolSz << " Lights";
    if( numHolding == 0 ) return;

    uint8_t nextIdx = 0;
    bool isAnotherBlock = getNextBlockIdx( nullptr, nextIdx );
    if( isAnotherBlock )
    {
        os << "\n Held Blocks";
    }

    while( isAnotherBlock )
    {
        os << "\n n: " << (unsigned int)nextIdx << '\t';
        os << *pBlockSz[ nextIdx ] << " Lights from " << (int)( *ppBlock[ nextIdx ] - pBase );
        isAnotherBlock = getNextBlockIdx( *ppBlock[ nextIdx ], nextIdx );
     //   if( --DPholding == 0 ) return;
    }

    if( szHeld == 0 )
    {
        os << "\n Entire pool is free";
    }
    else if( szHeld < poolSz )// there is at least 1 free block
    {
        os << "\n Free Blocks";
        isAnotherBlock = getNextBlockIdx( nullptr, nextIdx );
        if( isAnotherBlock && *ppBlock[ nextIdx ] > pBase )// gap in front
        {
            os << '\n' << (int)( *ppBlock[ nextIdx ] - pBase ) << " Lights from 0";
        }

        while( isAnotherBlock )
        {

            const Light* pBlockLow = *ppBlock[ nextIdx ] + *pBlockSz[ nextIdx ];
            int gapSz = poolSz - (int)( pBlockLow - pBase );

            isAnotherBlock = getNextBlockIdx( *ppBlock[ nextIdx ], nextIdx );
            if( isAnotherBlock )
            {
                gapSz = (int)( *ppBlock[ nextIdx ] - pBlockLow );
            }

            if( gapSz > 0 )// free block between
                os << '\n' << gapSz << " Lights from " << (int)( pBlockLow - pBase );
        }
    }

    os << '\n';
}
