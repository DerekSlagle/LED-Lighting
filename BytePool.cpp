#include "BytePool.h"

BytePool::BytePool()
{
    //ctor
}

BytePool::~BytePool()
{
    //dtor
}

void BytePool::init( uint8_t* p_Byte, unsigned int numBytes )
{
    pByte = p_Byte;
    poolSz = numBytes;
}

// verifies all pointers are != nullptr
bool BytePool::bindUsers(  uint8_t*** pp_Block, unsigned int** p_BlockSz, uint8_t NumUsers )
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

bool BytePool::getNextBlockIdx( uint8_t* lowBlock, uint8_t& nextIdx )const
{
    uint8_t* nextBlock = nullptr;

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
bool BytePool::Alloc( uint8_t idx, unsigned int ArrSz )const
{
    if( idx >= numUsers )
    {
        std::cerr << "\n BytePool::Alloc() idx >= numUsers: idx = " << idx << " numUsers = " << numUsers;
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
            *ppBlock[ idx ] = pByte;
            *pBlockSz[ idx ] = ArrSz;
            std::cerr << "\n BytePool::Alloc() 1st check";
            return true;
        }
        std::cerr << "\n BytePool::Alloc() ArrSz > poolSz";
        return false;
    }
    else// look deeper
    {
    //    while( true )
        while( *ppBlock[ nextIdx ] )
        {
            // 1 past end of given low block
        //    uint8_t* pBlock = lowBlock + low_pArr->arrSz;
            uint8_t* pBlockLow = *ppBlock[ nextIdx ] + *pBlockSz[ nextIdx ];
            isAnotherBlock = getNextBlockIdx( *ppBlock[ nextIdx ], nextIdx );// 1st ele in next lowest block

            if( isAnotherBlock )// is there space between blocks?
            {
                int gapSz = *ppBlock[ nextIdx ] - pBlockLow;
                if( gapSz >= (int)ArrSz )// there is
                {
                    *ppBlock[ idx ] = pBlockLow;
                    *pBlockSz[ idx ] = ArrSz;
                    std::cerr << "\n BytePool::Alloc() " << (unsigned int)nextIdx << " check";
                    return true;
                }

             //   low_pArr = next_pArr;// update for next iteration
            }
            else if( pBlockLow - pByte + ArrSz <= poolSz )// is remainder of pool enough?
            {
                *ppBlock[ idx ] = pBlockLow;
                *pBlockSz[ idx ] = ArrSz;
                std::cerr << "\n BytePool::Alloc() end " << (unsigned int)nextIdx << " check";
                return true;
            }
            else// not enough
            {
                std::cerr << "\n BytePool::Alloc() not enough";
                return false;
            }
        }// end while
    }

     std::cerr << "\n BytePool::Alloc() false return at end of function";
    return false;
}

void BytePool::Free( uint8_t idx )const// make block available
{
    *ppBlock[idx] = nullptr;
    *pBlockSz[idx] = 0;
}

uint8_t BytePool::DeFrag()const// returns number of blocks shifted back
{

    // find lowest pointer
    uint8_t nextIdx = 0;
    bool isBlock = getNextBlockIdx( nullptr, nextIdx );

    if( !isBlock ) return 0;// entire pool is free
    uint8_t shiftCount = 0;
    int gapSz = 0;

    // there may be a block behind lowDP
    if( *ppBlock[ nextIdx ] > pByte )
    {
        gapSz = (int)( *ppBlock[ nextIdx ] - pByte );
        // copy back nextDP array data
        for( unsigned int n = 0; n < *pBlockSz[ nextIdx ]; ++n )
            *( pByte + n ) = *( *ppBlock[ nextIdx ] + n );
        // assign to 1st
        *ppBlock[ nextIdx ] = pByte;
        shiftCount = 1;
    }

    while( isBlock )
    {
        uint8_t* pBlock = *ppBlock[ nextIdx ] + *pBlockSz[ nextIdx ];// start of next block

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

void BytePool::Report( std::ostream& os )const
{
    os << "\n\n** report **  pByte = " << (int)pByte << '\n';

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

    os << '\n' << numHolding << " holding " << szHeld << " of " << poolSz << " Bytes";
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
        os << *pBlockSz[ nextIdx ] << " Bytes from " << (int)( *ppBlock[ nextIdx ] - pByte );
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
        if( isAnotherBlock && *ppBlock[ nextIdx ] > pByte )// gap in front
        {
            os << '\n' << (int)( *ppBlock[ nextIdx ] - pByte ) << " Bytes from 0";
        }

        while( isAnotherBlock )
        {

            const uint8_t* pBlockLow = *ppBlock[ nextIdx ] + *pBlockSz[ nextIdx ];
            int gapSz = poolSz - (int)( pBlockLow - pByte );

            isAnotherBlock = getNextBlockIdx( *ppBlock[ nextIdx ], nextIdx );
            if( isAnotherBlock )
            {
                gapSz = (int)( *ppBlock[ nextIdx ] - pBlockLow );
            }

            if( gapSz > 0 )// free block between
                os << '\n' << gapSz << " Bytes from " << (int)( pBlockLow - pByte );
        }
    }

    os << '\n';
}
