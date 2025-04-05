#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H


class memoryPool
{
    public:
    long unsigned int* pPool = nullptr;
    unsigned int numEle = 100;

    memoryPool( unsigned int poolSz );
    memoryPool(){}
    ~memoryPool(){ if( pPool ) delete [] pPool; }

    memoryPool( const memoryPool& ) = delete;// copy
    memoryPool( memoryPool&& ) = delete;// move copy
    memoryPool& operator=( const memoryPool& ) = delete;// assign
    memoryPool& operator=( memoryPool&& ) = delete;// move assign

    protected:

    private:
};

#endif // MEMORYPOOL_H
