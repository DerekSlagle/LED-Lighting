#include "memoryPool.h"

memoryPool::memoryPool( unsigned int poolSz )
{
    pPool = new long unsigned int[poolSz];
    numEle = poolSz;
}

