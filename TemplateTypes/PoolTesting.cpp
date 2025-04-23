#include <iostream>

#include "TemplateTypes/MemPoolVector.h"
#include "TemplateTypes/MemPoolVector.cpp"

void testMemPool();

int main()
{
    testMemPool();
    return 0;
}

void testMemPool()
{
    // a pool of 200 int
    const unsigned int intPoolSz = 200;
    int intArr[intPoolSz];
    MemoryPool<int> intPool( intArr, intPoolSz );

    const unsigned int maxUsers = 10;
    int** ppBlock[ maxUsers ];// points to pool users pData = pointer to 1st element
    unsigned int* pBlockSz[ maxUsers ];// points to users Capacity
    // essential to assign nullptr
    for( unsigned int n = 0; n < maxUsers; ++n )
    {
        ppBlock[n] = nullptr;
        pBlockSz[n] = nullptr;
    }

    intPool.bindUsers( ppBlock, pBlockSz, maxUsers );// writes = 0 to pointers != nullptr (users)

    // an array
    PoolArray<int> PA;// variable size. Call PA.pPool->Alloc( PA.poolIdx, newSize );
    if( !PA.Bind( &intPool, 10 ) )// claim a pair of pointers ( poolIdx = 0 ) and Alloc 10 x int
    {
        std::cout << "\n PA.Bind() fail";
        return;
    }
    for( int n = 0; n < PA.Capacity; ++n )
        PA[n] = 100 + n;// assign a value to each element

    // a vector like type
    MemPoolVector<int> MPV_A( &intPool );// will get poolIdx = 1
    // and tempIdx = 2 (which is normally empty so it won't appear in reports)
    if( !MPV_A.pPool )
    {
        std::cout << "\n MPV_A.init() fail";
        return;
    }

    std::cout << "\n MPV_A.poolIdx = " << MPV_A.poolIdx;// should = 1

    // make a held block to work around
    // this one by hand
    int* pBlockA = intPool.pBase + 30;// we say 1st element at 30
    unsigned int capA = 12;// we say there are 12 elements
    ppBlock[4] = &pBlockA;// we choose the 5th pair. **** "poolIdx" = 4
    pBlockSz[4] = &capA;// should verify == nullptr 1st. This pair may be in use

    // test the vector like type
    MPV_A.CapBump = 5;// increase by 5 each time
    unsigned int CapLast = MPV_A.Capacity;// for change detection
    for( int n = 1; n <= 22; ++n )
    {
        if( !MPV_A.push_back(n) )// I haven't seen this message yet. Returns false if no space
            std::cout << "\n************** push_back() fail at n = " << n;

        if( MPV_A.Capacity != CapLast )
        {
            std::cout << "\n MPV_A.Capacity = " << MPV_A.Capacity;
            CapLast = MPV_A.Capacity;// update for next time
            intPool.Report( std::cout, "int" );
        }
    }

    // copy construct from MPV_A
    MemPoolVector<int> MPV_B( MPV_A );// will get poolIdx = 3, tempIdx = 5
    if( !MPV_B.pPool )//                because poolIdx = 4 is taken
    {
        std::cout << "\n MPV_B copy construction fail";
        return;
    }

    intPool.Report( std::cout, "int" );

    std::cout << "\n\nafter: MPV_A.Size = " << MPV_A.Size;
    std::cout << "\n MPV_A.Capacity = " << MPV_A.Capacity << "\n";
    for( unsigned int n = 0; n < MPV_A.Size; ++n )
        std::cout << "\nMPV_A[" << n << "] = " << MPV_A[n];

    std::cout << "\n\nPoolArray:\n";
    for( int n = 0; n < PA.Capacity; ++n )
        std::cout << "\nPA[" << n << "] = " << PA[n];

    // defragment the pool
    intPool.DeFrag();
    intPool.Report( std::cout, "int" );
}
