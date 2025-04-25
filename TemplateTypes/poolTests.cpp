#include <iostream>

#include "TemplateTypes/MemPoolVector.h"
#include "TemplateTypes/MemPoolVector.cpp"

void testMemPool();// pool of 200 int with pointer arrays supplied
void testMemPoolEx();// pool of 200 int with pointer arrays baked into pool
void testCharMemPool();// pool of 1000 char with baked in pointer arrays

int main()
{
    testMemPool();
    testMemPoolEx();
    testCharMemPool();
    return 0;
}

void testCharMemPool()
{
    // a pool of 1000 char
    const unsigned int charPoolSz = 1000;
    char charArr[charPoolSz];
    MemoryPool<char> charPool;
    const unsigned int maxUsers = 10;
    charPool.initBaked( charArr, charPoolSz, maxUsers );
    std::cout << "\n charPool.poolSz = " << charPool.poolSz;// 920 @ 4 bytes per pointer = 80 bytes for * storage
    // verify bounds
    std::cout << "\n charArr begin                 = " << (int)charArr;// char* aarg!
    std::cout << "\n charArr end                   = " << (int)( charArr + charPoolSz );
    std::cout << "\n charPool.pBlockSz[ maxUsers ] = " << (int)&( charPool.pBlockSz[ maxUsers ] );
    std::cout << "\n charPool.pBase                = " << (int)charPool.pBase;

    // an array
    // variable size. Call PA.pPool->Alloc( PA.poolIdx, newSize );
    PoolArray<char> PA( &charPool, 100 );// claim a pair of pointers ( poolIdx = 0 ) and Alloc 100 x char
    if( !PA.pBase ) { std::cout << "\n PA.Bind() fail"; return; }
    std::cout << "\n PA.poolIdx = " << PA.poolIdx;
    // echo
    const char* msgA = "Hello from charPool!";// 20 + '\0' = 21 char
    for( unsigned int k = 0; k < PA.Capacity; ++k )
    {
        *( PA.pBase + k ) = msgA[k];
        if( !msgA[k] ) break;// got the null terminator
    }
    // echo
    std::cout << "\n\n PA.pBase = " << PA.pBase;

    charPool.Report( std::cout, "char" );

    PoolArray<char> PB( PA );// claim a pair of pointers ( poolIdx = 1 ) and Alloc 100 x char
    if( !PB.pBase ) { std::cout << "\n PB.Bind() fail"; return; }
    std::cout << "\n PB.poolIdx = " << PB.poolIdx;
    // echo
    const char* msgB = "Block #2 asks How do you do?";// 28 + '\0' = 29 char
    for( unsigned int k = 0; k < PB.Capacity; ++k )
    {
        *( PB.pBase + k ) = msgB[k];
        if( !msgB[k] ) break;// got the null terminator
    }
    // echo
    std::cout << "\n\n PB.pBase = " << PB.pBase;

    charPool.Report( std::cout, "char" );

    // an array of 4 PoolArray<char>
    const unsigned int numArrays = 4;
    PoolArray<char> msgPA_array[numArrays];// poolIdx = 2, 3, 4, 5
    for( unsigned int k = 0; k < numArrays; ++k )
        if( !msgPA_array[k].Bind( &charPool, 60 ) )
        {
            std::cout << "\n charPoolArray[" << k << "].Bind() fail";
            return;
        }
    const char* msgArr[numArrays] = { "msgArr[0] here. Good day!",
                            "msgArr[1] here. I sure hope this works.",
                            "msgArr[2] here. This pool concept is working well",
                            "msgArr[3] here. Let us find some edge cases!" };

    for( unsigned int n = 0; n < numArrays; ++n )
        for( unsigned int k = 0; k < msgPA_array[n].Capacity; ++k )
        {
            *( msgPA_array[n].pBase + k ) = msgArr[n][k];
            if( !msgArr[n][k] ) break;// got the null terminator
        }
    // echo
    for( unsigned int n = 0; n < numArrays; ++n )
        std::cout << "\n msgPA_array[" << n << "].pBase = " << msgPA_array[n].pBase;

    charPool.Report( std::cout, "char" );
    // free 2 blocks
    msgPA_array[0].unBind();
    msgPA_array[2].unBind();
    std::cout << "\n unBind() poolIdx = 2, 4";
    charPool.Report( std::cout, "char" );

    std::cout << "\n\n*** Defrag ***";
    charPool.DeFrag();
    charPool.Report( std::cout, "char" );

    // what is left?
    for( unsigned int n = 0; n < charPool.numUsers; ++n )
        if( charPool.ppBlock[n] )
            std::cout << "\n *( charPool.ppBlock[" << n << "] ) = " << *( charPool.ppBlock[n] );
}

void testMemPool()
{
    // arrays for up to 10 users
    const unsigned int maxUsers = 10;
    int** ppBlock[ maxUsers ];// points to pool users pData = pointer to 1st element
    unsigned int* pBlockSz[ maxUsers ];// points to users Capacity

    // a pool of 200 int
    const unsigned int intPoolSz = 200;
    int intArr[intPoolSz];
    MemoryPool<int> intPool;
    intPool.init( intArr, intPoolSz, ppBlock, pBlockSz, maxUsers );

    // an array
    // variable size. Call PA.pPool->Alloc( PA.poolIdx, newSize );
    PoolArray<int> PA( &intPool, 10 );// claim a pair of pointers ( poolIdx = 0 ) and Alloc 10 x int
    if( !PA.pBase )
    {
        std::cout << "\n PA.Bind() fail";
        return;
    }
    std::cout << "\n PA.poolIdx = " << PA.poolIdx;
    for( unsigned int n = 0; n < PA.Capacity; ++n )
        PA[n] = 100 + n;// assign a value to each element

    // a vector like type
    MemPoolVector<int> MPV_A( &intPool );// will get poolIdx = 1
    // and tempIdx = 2 (which is normally empty so it won't appear in reports)
    std::cout << "\n MPV_A.poolIdx = " << MPV_A.poolIdx << "  MPV_A.tempIdx = " << MPV_A.tempIdx;
    if( !MPV_A.pPool )
    {
        std::cout << "\n MPV_A.init() fail";
        return;
    }

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

    // allocate a new array so I can invoke the destructor then Report
    PoolArray<int>* pPA1 = new PoolArray<int>( &intPool, 23 );// poolIdx = 3
    std::cout << "\n pPA1->poolIdx = " << pPA1->poolIdx;// should = 3
    for( unsigned int n = 0; n < pPA1->Capacity; ++n )
        pPA1->getElement(n) = 200 + n;// assign a value to each element
     //   *( pPA1->pBase + n ) = 200 + n;// ok but messy
     //   *( pPA1 )[n] = 200 + n;// error

    // copy construct a vector from MPV_A
    MemPoolVector<int> MPV_B( MPV_A );// will get poolIdx = 5, tempIdx = 6 (nope 6,7 where is 5?)
    std::cout << "\n MPV_B.poolIdx = " << MPV_B.poolIdx << "  MPV_B.tempIdx = " << MPV_B.tempIdx;
    if( !MPV_B.pPool )
        { std::cout << "\n MPV_B copy construction fail"; return; }

    intPool.Report( std::cout, "int" );

    std::cout << "\n\n*** pPA1 array poolIdx = 3 destroyed ***";
    delete pPA1;
    intPool.Report( std::cout, "int" );

    std::cout << "\n\n*** hand array poolIdx = 4 released ***";
    ppBlock[4] = nullptr;// release the 5th pair. **** "poolIdx" = 4
    pBlockSz[4] = nullptr;
    intPool.Report( std::cout, "int" );

    // show array values
    std::cout << "\n\nafter: MPV_A.Size = " << MPV_A.Size;
    std::cout << "\n MPV_A.Capacity = " << MPV_A.Capacity << "\n";
    for( unsigned int n = 0; n < MPV_A.Size; ++n )
        std::cout << "\nMPV_A[" << n << "] = " << MPV_A[n];

    std::cout << "\n\nPoolArray:\n";
    for( unsigned int n = 0; n < PA.Capacity; ++n )
        std::cout << "\nPA[" << n << "] = " << PA[n];

    std::cout << "\n\nafter: MPV_B.Size = " << MPV_B.Size;
    std::cout << "\n MPV_B.Capacity = " << MPV_B.Capacity << "\n";
    for( unsigned int n = 0; n < MPV_B.Size; ++n )
        std::cout << "\nMPV_B[" << n << "] = " << MPV_B[n];

    // defragment the pool
    std::cout << "\n\n*** Defrag ***";
    intPool.DeFrag();
    intPool.Report( std::cout, "int" );

}

void testMemPoolEx()
{
    // a pool of 200 int
    const unsigned int intPoolSz = 200;
    int intArr[intPoolSz];
    MemoryPool<int> intPool;
    const unsigned int maxUsers = 10;
    intPool.initBaked( intArr, intPoolSz, maxUsers );
    std::cout << "\n intPool.poolSz = " << intPool.poolSz;
    // verify bounds
    std::cout << "\n intArr begin                 = " << intArr;
    std::cout << "\n intArr end                   = " << intArr + intPoolSz;
    std::cout << "\n intPool.pBlockSz[ maxUsers ] = " << &( intPool.pBlockSz[ maxUsers ] );
    std::cout << "\n intPool.pBase                = " << intPool.pBase;

    // an array
    // variable size. Call PA.pPool->Alloc( PA.poolIdx, newSize );
    PoolArray<int> PA( &intPool, 10 );// claim a pair of pointers ( poolIdx = 0 ) and Alloc 10 x int
    if( !PA.pBase )
    {
        std::cout << "\n PA.Bind() fail";
        return;
    }
    std::cout << "\n PA.poolIdx = " << PA.poolIdx;
    for( unsigned int n = 0; n < PA.Capacity; ++n )
        PA[n] = 100 + n;// assign a value to each element

    // a vector like type
    MemPoolVector<int> MPV_A( &intPool );// will get poolIdx = 1
    // and tempIdx = 2 (which is normally empty so it won't appear in reports)
    std::cout << "\n MPV_A.poolIdx = " << MPV_A.poolIdx << "  MPV_A.tempIdx = " << MPV_A.tempIdx;
    if( !MPV_A.pPool )
    {
        std::cout << "\n MPV_A.init() fail";
        return;
    }

    // make a held block to work around
    // this one by hand
    int* pBlockA = intPool.pBase + 30;// we say 1st element at 30
    unsigned int capA = 12;// we say there are 12 elements
    unsigned int handPoolIdx = 7;// we choose the 8th pair. **** "poolIdx" = 7
    intPool.ppBlock[ handPoolIdx ] = &pBlockA;//
    intPool.pBlockSz[ handPoolIdx ] = &capA;// should verify == nullptr 1st. This pair may be in use

    intPool.Report( std::cout, "int" );

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

    // copy construct an array from MPV_A
    PoolArray<int> PoolArrB( PA );// will get poolIdx = 3
    std::cout << "\n PoolArrB.poolIdx = " << PoolArrB.poolIdx;
    if( !PoolArrB.pPool )
    {
        std::cout << "\n PoolArrB copy construction fail";
        return;
    }

    intPool.Report( std::cout, "int" );

    std::cout << "\n\n*** hand array poolIdx = 7 released ***";
    intPool.ppBlock[ handPoolIdx ] = nullptr;// release the 5th pair. **** "poolIdx" = 4
    intPool.pBlockSz[ handPoolIdx ] = nullptr;
    intPool.Report( std::cout, "int" );

    std::cout << "\n\nPoolArray PA:\n";
    for( unsigned int n = 0; n < PA.Capacity; ++n )
        std::cout << "\nPA[" << n << "] = " << PA[n];

    std::cout << "\n\nPoolArray: PoolArrB\n";
    for( unsigned int n = 0; n < PoolArrB.Capacity; ++n )
        std::cout << "\nPoolArrB[" << n << "] = " << PoolArrB[n];
}
