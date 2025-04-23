#include <iostream>
#include <fstream>
#include <vector>
#include <string>

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
    const unsigned int intPoolSz = 200;
    int intArr[intPoolSz];
    MemoryPool<int> intPool( intArr, intPoolSz );// a pool of 200 int

    const unsigned int maxUsers = 10;
    int** ppBlock[ maxUsers ];// points to users pData
    unsigned int* pBlockSz[ maxUsers ];// points to pool users Capacity
    // essential to assign nullptr
    for( unsigned int n = 0; n < maxUsers; ++n )
    {
        ppBlock[n] = nullptr;// no users
        pBlockSz[n] = nullptr;// bound yet
    }

    intPool.bindUsers( ppBlock, pBlockSz, maxUsers );// writes = 0 to any pointers != nullptr

    // an array
    PoolArray<int> PA;
    if( !PA.Bind( &intPool, 10 ) )// claim a pair of pointers and Alloc 10 x int
    {
        std::cout << "\n PA.Bind() fail";
        return;
    }
    for( int n = 0; n < PA.Capacity; ++n )
        PA[n] = 100 + n;// write a value to each element

    // a vector like type
    MemPoolVector<int> MPV_A( &intPool );// initial capacity = 0
    if( !MPV_A.pPool )
    {
        std::cout << "\n MPV_A.init() fail";
        return;
    }

    std::cout << "\n MPV_A.poolIdx = " << MPV_A.poolIdx;

    // make a held block to work around
    // doing it manually
    int* pBlockA = intPool.pBase + 30;
    unsigned int capA = 12;
    ppBlock[4] = &pBlockA;// claim a pair
    pBlockSz[4] = &capA;// of pointers

    // test
    MPV_A.CapBump = 5;// increase capacity by 5 elements
    unsigned int CapLast = MPV_A.Capacity;// to sense a change in capacity
    for( int n = 1; n <= 22; ++n )
    {
        if( !MPV_A.push_back(n) )
            std::cout << "\n************** push_back() fail at n = " << n;

        if( MPV_A.Capacity != CapLast )
        {
            std::cout << "\n MPV_A.Capacity = " << MPV_A.Capacity;
            CapLast = MPV_A.Capacity;// update for next time
            intPool.Report( std::cout, "int" );
        }
    }

    // copy construct from MPV_A
    MemPoolVector<int> MPV_B( MPV_A );
    if( !MPV_B.pPool )
    {
        std::cout << "\n MPV_B copy construction fail";
        return;
    }

    intPool.Report( std::cout, "int" );

    // output the array values
    std::cout << "\n\nafter: MPV_A.Size = " << MPV_A.Size;
    std::cout << "\n MPV_A.Capacity = " << MPV_A.Capacity << "\n";
    for( unsigned int n = 0; n < MPV_A.Size; ++n )
        std::cout << "\nMPV_A[" << n << "] = " << MPV_A[n];

    std::cout << "\n\nPoolArray:\n";
    for( int n = 0; n < PA.Capacity; ++n )
        std::cout << "\nPA[" << n << "] = " << PA[n];
}
