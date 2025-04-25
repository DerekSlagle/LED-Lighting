Type MemoryPool<T> is for storage of arrays of T.

It can be defragmented during use. 
The address of the pointer owning each block is stored
so it can be upadated if its block is moved.
Call: pool.Defrag(); to get all free elements in 1 block at the end.
Call: pool.Report(); A report is given via std::cout listing the held and
free block locations and sizes.

The storage space is to be given. Declare for use:
T poolStore[ ArrSz ];

An array (or blocks of elements) can be secured and used by declaring
one each:
T* pBlock;// pointer to 1st element in array
unsigned int Capacity;// # of array elements
unsigned int poolIdx;// optional. A lookup function can find it if needed.

Management of the pool storage is accomplished using 2 arrays of pointers
T** ppBlock[ numUsers ];// points to each users pBlock
unsigned int pBlockSz[ numUsers ];// points to each users Capacity

There is a limit = numUsers to the number of arrays (or blocks of elements)
which can be stored.

A pool can be default constructed
MemoryPool<T> Tpool;
Then initialize in 1 of 2 ways
1. The 2 pointer arrays are supplied as external storage for the pool:
From MemoryPool.h
// use with external pointer arrays
    void init( T* p_Base, unsigned int ArrSz, T*** pp_Block, unsigned int** p_BlockSz, unsigned int NumUsers );
Call here as:
Tpool.init( poolStore, ArrSz, ppBlock, pBlockSz, numUsers );

2. The 2 pointer arrays are "baked" into the pool via placement new.
Pro: Storage space external to  the pool is avoided.
Con: The pointers take up some of the ArrSz elements, so the space for storing T is reduced.
Call instead:
Tpool.init( poolStore, ArrSz, numUsers );// the pointers are stored in the pool.

Constructors are defined for both:
MemoryPool<T> Tpool( poolStore, ArrSz, ppBlock, pBlockSz, numUsers );
or
MemoryPool<T> Tpool( poolStore, ArrSz, numUsers );

The pool is ready for use by up to numUsers block holders.

*** using the pool ***
See poolTests.cpp this folder for working examples of each of the following.

1. Manage a block using local variables. All MemoryPool members are public.
The pool can be used freely if desired. 
This example is 1st to illustrate the basic principles of use.
Declare:
T* pBlock = nullptr;
unsigned int Capacity = 0;
unsigned int poolIdx = 0;
Call:
if( !Tpool.getFreeIndex( poolIdx ) )// writes to poolIdx if pair of pointers is available
{ cout << "fail"; bail? }

There is a pair of pointers. Register the local variables.
Tpool.ppBlock[poolIdx] = &pBlock;
Tpool.pBlockSz[poolIdx] = &Capacity;

Allocate storage for an array of numT type T
if( !Tpool.Alloc( poolIdx, numT ) )// writes to pBlock and Capacity if block found
{ cout << "fail"; bail? }

Use the array:
T& Tn = pBlock[n];// reference to nth element in array of T

"Destroy" by making the pair of pointers available again
Tpool.ppBlock[poolIdx] = nullptr;
Tpool.pBlockSz[poolIdx] = nullptr;
// safe off the local pointer
pBlock = nullptr;

2. Type PoolArray<T>
Declare:
PoolArray<T> PA( &Tpool, numT );
or default construct:
PoolArray<char> PA;
Then Bind:
PA.Bind( &Tpool, numT );
The latter is useful for using arrays of PoolArray.
See void testCharMemPool(); within poolTests.cpp where an array of
character strings is stored:
From the example in testCharMemPool
// declare array of PoolArray<char> and Bind them
PoolArray<char> msgPA_array[numArrays];
for( unsigned int k = 0; k < numArrays; ++k )
    if( !msgPA_array[k].Bind( &charPool, 60 ) ) complain;

Or copy construct:
PoolArray<char> PB( PA );

Usage:
access:
PA[n] or PA.getElement(n);
Both return a T&.
The array can be resized using its pPool member.
if( !PA.pPool->Alloc( PA.poolIdx, newSize ) ) face palm;
Use the array 

"Destruction"
PA.unBind();
Also, the destructor calls unBind() so the pointers get returned for use.

3. Type MemPoolVector<T>
Works like std::vector to increase capacity when a push_back(T) causes
Size > Capacity.
MemPoolVector holds a 2nd block by using members
T* pBaseTemp = nullptr;
unsigned int tempCap = 0;
unsigned int tempIdx = 0;
It uses both during a capacity increase then release the 1st:
pBase = pBaseTemp;// now held by poolIdx pair, not tempIdx pair
Capacity = tempCap;
pBaseTemp = nullptr;// 2nd block now "empty"
tempCap = 0;

Create:
MemPoolVector<T> MPV( &Tpool );// bind but leave empty
or
MemPoolVector<T> MPV( &Tpool, numT );// bind and allocate initial storage

or default construct then Bind:
MemPoolVector<T> MPV;
MPV.Bind( &Tpool, numT );
or
MPV.Bind( &Tpool );
then
MPV.reserve( numT );// no version yet which takes a T to copy from
or
MPV.resize( numT );

or copy construct:
MemPoolVector<T> MPV2( MPV );

Use:
element access:
T& MPV[n];
T& MPV.getElement(n);
T& MPV.front();
T& MPV.back();
add another element
bool pushback(T);// false if Alloc fails increase capacity

"Destroy"
MPV.unBind();// releases both pairs poolIdx and tempIdx
This is called by the destructor.