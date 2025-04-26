This simple sketch is for testing the operation of a MemoryPool of 100 type int.
// from the ino file
const unsigned int storeSz = 100;// line 4
int Store[ storeSz ];// the actual storage space
...
MemoryPool<int> intPool(args);// line 9

The testing is done using 4 instances of type PoolArray.
const unsigned int numUsers = 4;// line 6
PoolArray<int> PA[ numUsers ];// line 12

All 4 are given some initial capacity.
// in setup()
PA[0].Bind( &intPool, 15 );// lines 28 - 31
PA[1].Bind( &intPool, 10 );
PA[2].Bind( &intPool, 5 );
PA[3].Bind( &intPool, 12 );

The element values are automatically assigned so we don't have to enter them.
The formula is:
for( int j = 0; j < PA[n].Capacity; ++j )  PA[n][j] = (n+1)*(1+j);// line 38

The initial pool state is reported in the Serial monitor as:
Setup
Good Alloc
Good Alloc
Good Alloc
Good Alloc
Initial 
Pool report pBase = 708
4 holding 42 of 100 int
Held Blocks
n: 0   15 int from 0
n: 1   10 int from 15
n: 2   5 int from 25
n: 3   12 int from 30
Free Blocks
58 int from 42

To use: Enter 1 or 2 integer values
Enter 1 value if defragmenting the pool.
Enter -1 to defragment the pool. A report will follow.
It's not time yet. There are no free blocks yet.
Let's allocate 7 elements to PA[2].
It will need more storage. 
A block of 7 from 42 where the free store begins shall be allocated.
The existing block of 5 from 25 will become free.
Enter: 2 7 + enter. Output in the monitor is:
 
I = 2
num2follow = 7
Alloc good
7 Elements: 2 4 6 8 10 12 14
Pool report pBase = 708
4 holding 44 of 100 int
Held Blocks
n: 0   15 int from 0
n: 1   10 int from 15
n: 3   12 int from 30
n: 2   7 int from 42
Free Blocks
5 int from 25
51 int from 49

Let's produce another free block by freeing all PA[0] storage.
Assign: PA[0].Capacity = 0;
Enter: 0 0 + enter
This frees all PA[0] storage, so it won't appear in the report 
It's not holding a block. PA[0].pBase = nullptr. It's still a bound user though.
The result is:
 
I = 0
num2follow = 0
0 Elements:
Pool report pBase = 708
3 holding 29 of 100 int
Held Blocks
n: 1   10 int from 15
n: 3   12 int from 30
n: 2   7 int from 42
Free Blocks
15 int from 0
5 int from 25
51 int from 49

Now the order of the blocks in the pool is
free(15), held(10), free(5), held(12), held(7), freeStore(51)
NOW we defrag! Enter: -1 + enter
Result:
 
I = -1
** Defrag **

Pool report pBase = 708
3 holding 29 of 100 int
Held Blocks
n: 1   10 int from 0
n: 3   12 int from 10
n: 2   7 int from 22
Free Blocks
71 int from 29

One more for the demo here. Let's get PA[0] back in the game.
Allocate 23 elements to PA[0]
Enter: 0 23 + enter
PA[0].pPool->Alloc( PA[0].poolIdx, 23 );// is called
Result:
 
I = 0
num2follow = 23
Alloc good
23 Elements: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
Pool report pBase = 708
4 holding 52 of 100 int
Held Blocks
n: 1   10 int from 0
n: 3   12 int from 10
n: 2   7 int from 22
n: 0   23 int from 29
Free Blocks
48 int from 52

That covers all usage. 
I hope that you find it useful, or that it inspires some design ideas.
I have not tested it to failure, such as by requesting more elements
than are available.