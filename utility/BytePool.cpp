#include "BytePool.h"

// good for both methods
void BytePool::init( uint8_t* p_Byte, unsigned int numBytes, uint8_t*** pp_Block, unsigned int** p_BlockSz, unsigned int NumUsers )
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

// good for both methods. The pool arrays are stored in the pool.
void BytePool::initBaked( uint8_t* p_Byte, unsigned int numBytes, unsigned int NumUsers )
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
    ppBlock = new ( p_Byte ) uint8_t**[ NumUsers ];// pointer to user pBase
    if( ppBlock )
        for( unsigned int n = 0; n < NumUsers; ++n ) ppBlock[n] = nullptr;
    // bake the 2nd array in
    pBlockSz = new ( p_Byte + hfBlockSz ) unsigned int*[ NumUsers ];// pointer to user Capacity
    if( pBlockSz )
        for( unsigned int n = 0; n < NumUsers; ++n ) pBlockSz[n] = nullptr;
}

// 1st method
// find an unused pair
bool BytePool::getFreeIndex( unsigned int& idx )const// writes index to idx
{
    for( unsigned int n = 0; n < numUsers; ++n )
    {
        if( !( ppBlock[n] || pBlockSz[n] ) )
        {
            idx = n;
            return true;
        }
    }

    return false;
}

// lookup the poolIdx
bool BytePool::getMyPoolIndex( const uint8_t*& p_Byte, unsigned int& idx )const// writes index to idx
{
    if( !p_Byte ) return false;
    for( unsigned int n = 0; n < numUsers; ++n )
    {
        if( ppBlock[n] == &p_Byte )
        {
            idx = n;
            return true;
        }
    }

    return false;
}

bool BytePool::getNextBlockIdx( uint8_t* lowBlock, uint8_t& nextIdx )const
{
    uint8_t* nextBlock = nullptr;

    for( uint8_t n = 0; n < numUsers; ++n )
    {
        if( !ppBlock[n] ) continue;// = nullptr is now normal
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
    if( idx >= numUsers ) return false;
    if( !ppBlock[ idx ] ) return false;// should not happen

    // already holding?
    if( *ppBlock[ idx ] )
    {
        if( *pBlockSz[ idx ] >= ArrSz )// Capacity is enough
        {
            *pBlockSz[ idx ] = ArrSz;// reduce Capacity
            return true;
        }
        // free it
    //    Free( idx );
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
            std::cerr << "\n Alloc() 1st check";
            return true;
        }
        return false;
    }
    else// look deeper
    {
        while( *ppBlock[ nextIdx ] )
        {
            // 1 past end of given low block
            uint8_t* pBlockLow = *ppBlock[ nextIdx ] + *pBlockSz[ nextIdx ];
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
            else if( pBlockLow - pByte + ArrSz <= poolSz )// is remainder of pool enough?
            {
                *ppBlock[ idx ] = pBlockLow;
                *pBlockSz[ idx ] = ArrSz;
                std::cerr << "\n Alloc() end " << (unsigned int)nextIdx << " check";
                return true;
            }
            else// not enough
            {
                return false;
            }
        }// end while
    }

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

        std::cout << "\nDeFrag() gap in front";
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
        if( !ppBlock[n] ) continue;

        if( *ppBlock[n] )
        {
            ++numHolding;
            szHeld += *pBlockSz[n];
            os << " n: " << (unsigned int)n;
        }
    }

    os << '\n' << numHolding << " numHolding " << szHeld << " of " << poolSz << " elements";
    if( numHolding == 0 ) return;

    uint8_t nextIdx = 0;
    bool isAnotherBlock = getNextBlockIdx( nullptr, nextIdx );
    if( isAnotherBlock ) os << "\n Held Blocks";
    while( isAnotherBlock )
    {

        os << '\n' << *pBlockSz[ nextIdx ] << " ele from " << (int)( *ppBlock[ nextIdx ] - pByte );
        isAnotherBlock = getNextBlockIdx( *ppBlock[ nextIdx ], nextIdx );
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
            os << '\n' << (int)( *ppBlock[ nextIdx ] - pByte ) << " elements from 0";
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
                os << '\n' << gapSz << " elements from " << (int)( pBlockLow - pByte );
        }
    }

    os << '\n';
}

// 2nd model
bool BytePool::Alloc_ordered( uint8_t*& p_Block, unsigned int& Capacity, int ArrSz )const
{
    if( ArrSz <= 0 ) return false;
    if( ArrSz > (int)poolSz ) return false;

    Free2( p_Block, Capacity );

    // The following code MUST BE simple and compact
    // Target = 20 loc. Hard Limit = 40 loc
    int gap = poolSz;
    uint8_t* pBase = pByte;
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
            std::cout << "\nAlloc2(): n = " << n;
            return true;
        }

        pBase = *ppBlock[n] + *pBlockSz[n];
        ++n;
    }// 29 loc. OK

    return false;
}

void BytePool::Free2( uint8_t*& p_Block, unsigned int& Capacity )const
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

uint8_t BytePool::DeFrag2()const// returns ?
{
    if( !ppBlock[0] ) return 0;// entire pool is free
    uint8_t shiftCount = 0;
    uint8_t* pBase = pByte;
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

void BytePool::Report2( std::ostream& os )const
{
    os << "\n\n** report **  pByte0 = " << (int)pByte << '\n';

    int numHolding = 0;
    unsigned int szHeld = 0;
    for( uint8_t n = 0; n < numUsers; ++n )
    {
        if( !ppBlock[n] ) break;
        if( *ppBlock[n] )
        {
            ++numHolding;
            szHeld += *pBlockSz[n];
            os << " n: " << (unsigned int)n;
        }
    }

    if( numHolding > 0 ) os << '\n';
    os << numHolding << " Holding " << szHeld << " of " << poolSz << " Bytes";
    if( numHolding == 0 ) return;

    uint8_t* pBase = pByte;
    for( unsigned int n = 0; n < numUsers; ++n )
    {
        if( !ppBlock[n] )
        {
            int toEnd = (int)( pByte + poolSz - pBase );
            if( toEnd > 0 )// free block
                os << "\n FREE " << toEnd << " Bytes from " << (int)( pBase - pByte ) << " to end";
            break;
        }

        if( *ppBlock[n] )
        {
            int gap = (int)( *ppBlock[n] - pBase );

            if( gap > 0 )// free block
                os << "\n FREE " << gap << " Bytes from " << (int)( pBase - pByte );

            // the following held block
            os << "\n held " << *pBlockSz[n] << " Bytes from " << (int)( *ppBlock[n] - pByte );
            pBase = *ppBlock[n] + *pBlockSz[n];// for next iter
        }
        else// this code should never be reached
        {
            os << "\n ERROR: *ppBlock[" << n << "] is NULL";
        }
    }

    os << '\n';
}

/*
void testBytePool()
{
    std::ifstream fin("testBytePool_data.txt");
    if( !fin ) { std::cout << "\n no testBytePool_data.txt file"; return; }

    // get some file names for bitArray
    std::vector<std::string> fNameVec;
    unsigned int numFiles;
    fin >> numFiles;
    fNameVec.resize( numFiles );
    for( unsigned int n = 0 ; n < numFiles; ++n )
        fin >> fNameVec[n];

    const unsigned int poolSize = 1000;
    uint8_t PoolArr[ poolSize ];
    BytePool BP( PoolArr, poolSize );

    // pool users
 //   pattPlay PP_1, PP_2, PP_3;
 //   dataPlay DP_1, DP_2;
    const unsigned int NumUsers = 5;
    uint8_t** ppBlock[ NumUsers ];// = { &(PP_1.pBytes), &(PP_2.pBytes), &(PP_3.pBytes), &(DP_1.pArr), &(DP_2.pArr) };
    unsigned int* pBlockSz[ NumUsers ];// = { &(PP_1.numBytes), &(PP_2.numBytes), &(PP_3.numBytes), &(DP_1.arrSz), &(DP_2.arrSz) };
    bitArray BA[ NumUsers ];

    std::cerr << "\n *ppBlock: ";
    ppBlock[0] = &( BA[0].pByte );
    ppBlock[1] = &( BA[1].pByte );
    ppBlock[2] = &( BA[2].pByte );
    ppBlock[3] = &( BA[3].pByte );
    ppBlock[4] = &( BA[4].pByte );
    std::cerr << "\n pBlockSz: ";
    pBlockSz[0] = &( BA[0].capBytes );
    pBlockSz[1] = &( BA[1].capBytes );
    pBlockSz[2] = &( BA[2].capBytes );
    pBlockSz[3] = &( BA[3].capBytes );
    pBlockSz[4] = &( BA[4].capBytes );

    if( !BP.bindUsers( ppBlock, pBlockSz, NumUsers ) )
    {
        std::cout << "\n bad bind";
        return;
    }

    for( unsigned int k = 0; k < NumUsers; ++k )
    {
        std::cerr << (int)pBlockSz[k] << ' ';
    }

    // usage
    int AllocCount = 0;
    char Action = 'N';// A = Allocate, F = Free, R = report, D = DeFrag, N = end loop

    while( fin >> Action )
    {
        int Idx = 0;
        if( Action == 'A' )
        {
            fin >> Idx;
            int fileIdx;
            fin >> fileIdx;
            std::ifstream finBA( fNameVec[ fileIdx ].c_str() );
            if( !finBA ) { std::cout << "\n no " << fNameVec[ fileIdx ].c_str() << " file"; return; }
            unsigned int numColors;
            finBA >> numColors;
            unsigned int Rows, Cols;
            finBA >> Rows >> Cols;
            unsigned int numVals;
            finBA >> numVals;
            std::cout << "\n numVals = " << numVals;

            unsigned int ArrSz = 0;
            if( numColors == 2 ){ ArrSz = numVals/8; if( numVals%8 ) ++ArrSz; }
            else if( numColors == 4 ){ ArrSz = numVals/4; if( numVals%4 ) ++ArrSz; }
    //        fin >> ArrSz;

            BA[ Idx ].Clear();
            if( BP.Alloc( Idx, ArrSz ) )
            {
                ++AllocCount;
                std::cerr << "\n** Alloc " << AllocCount << " OK";
                if( !BA[ Idx ].loadBitsFromStream( numColors, finBA, numVals ) )
                { std::cerr << "\n** Alloc loadBitsFromStream FAIL"; return; }
             //   BA[ Idx ].view( numColors, Rows, Cols );
                BA[ Idx ].viewBytes( 8 );
            }
            else
            {
                std::cerr << "\n** Alloc " << AllocCount << " Fail ***";
            }
        }
        else if( Action == 'F' )
        {
            fin >> Idx;
            BP.Free( Idx );
        }
        else if( Action == 'R' )
        {
            BP.Report( std::cout );
        }
        else if( Action == 'D' )
        {
            unsigned int numFree = BP.DeFrag();
            std::cout << "\n** DeFrag: blocks freed = " << numFree << " **";
            for( unsigned int n = 0; n < NumUsers; ++n )
            {
                if( BA[n].pByte )// holds a block
                {
                    std::cout << "\n BA[" << n << "].capBytes = " << BA[n].capBytes;
                    BA[n].viewBytes( 8 );
                }
            }
        }
        else if( Action == 'N' )
        {
            std::cout << "\nBye!";
        }
    }

    return;
}
*/
