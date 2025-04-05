#ifndef BITARRAY_H
#define BITARRAY_H

#include<iostream>// to write state to  console for viewing

// use an array of type unsigned char
class bitArray
{
    private:
    unsigned char* pByte = nullptr;
    bool isOwner = false;
    unsigned int capBytes = 0;
    unsigned int sizeBits = 0;// role ??

    public:
    void init( unsigned char& Char0, unsigned int CapBytes );// manager
    bitArray( unsigned char& Char0, unsigned int CapBytes ) { init( Char0, CapBytes ); }

    void init( unsigned int CapBytes );// owner
    bitArray( unsigned int CapBytes ) { init( CapBytes ); }

    bitArray( const bitArray& ) = delete;// no copy
    bitArray& operator = ( const bitArray& ) = delete;// no assignment

    unsigned int ByteCapacity()const { return capBytes; }
    unsigned int bitCapacity()const { return 8*capBytes; }
    unsigned int bitSize()const { return sizeBits; }

    bool loadBitsFromStream( std::istream& is );// as 8*capBytes characters = '0' or '1'
    void loadBitsFromCharArray( const unsigned char* pSrc );// reads capBytes Bytes

    // copy from or write to entire block of capBytes Bytes
    void copyFrom( const unsigned char* pSrc );
    void copyTo( unsigned char* pTgt )const;

    bool getBit( unsigned int n )const;// return n th of 8*capBytes bits?
    void setBit( unsigned int n, unsigned char binVal )const;// changing value pointed to, not a member.
    void Clear(){ sizeBits = 0; }
    void reSize( unsigned int newSz ){ sizeBits = newSz; }
    void pop(){ if( sizeBits > 0 ) --sizeBits; }
    void push( bool bitVal )// increments sizeBits
    {
        setBit( sizeBits, bitVal > 0 );
        ++sizeBits;// no re allocation when capacity = 8*capBytes is reached
    }

    void view( unsigned int bitsPerRow = 8 )const;

    bitArray(){}
    ~bitArray()
    {
        if( isOwner && pByte )
        {
            if( capBytes > 1 )
                delete [] pByte;
            else
                delete pByte;
        }
    }

    protected:


};

#endif // BITARRAY_H
