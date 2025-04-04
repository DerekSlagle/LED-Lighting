#include "bitArray.h"

void bitArray::init( unsigned char& Char0, unsigned int CapBytes )// manager
{
    isOwner = false;
    pByte = &Char0;
    capBytes = CapBytes;
    sizeBits = 0;// unsure if this will be usefull
    for( unsigned int k = 0; k < capBytes; ++k )
        *( pByte + k ) = 0;
}

void bitArray::init( unsigned int CapBytes )// owner
{
    isOwner = true;
    pByte = new unsigned char[ CapBytes ];
    capBytes = CapBytes;
    sizeBits = 0;// unsure if this will be usefull
    for( unsigned int k = 0; k < capBytes; ++k )
        *( pByte + k ) = 0;
}

bool bitArray::loadBitsFromStream( std::istream& is )// as 8*capBytes characters = '0' or '1'
{
    unsigned char inBit = 0, inByte = 0;
    unsigned char twoPow = 1;
    unsigned char* pIter = pByte;

    for( unsigned int k = 0; k < 8*capBytes; ++k )
    {
        is >> inBit;// '0' or '1'

        inByte += twoPow*inBit;

        if( twoPow < 128 )
        {
            twoPow *= 2;
        }
        else// write and advance
        {
            *pIter = inByte;
            ++pIter;
            twoPow = 1;
        }
    }

    return true;
}

void bitArray::loadBitsFromCharArray( const unsigned char* pSrc )// reads capBytes Bytes
{
    for( unsigned int n = 0; n < capBytes; ++n )
        *( pByte + n ) = *( pSrc + n );

    sizeBits = 8*capBytes - 1;// full up. All elements assigned
}

bool bitArray::getBit( unsigned int n )const// return n th of 8*capBytes bits?
{
    const unsigned char* pTgt = pByte + n/8;
    n %= 8;
    unsigned char mask = 1;
 //   if( n > 0 ) mask << n;// works here but not anywhere else. will not use if I do not understand
    while( n > 0 )// instead
    {
        mask *= 2;// look it up
        --n;
    }

    // bitwise and target byte with mask
    unsigned char result = *pTgt & mask;
    return result > 0;
}

void bitArray::setBit( unsigned int n, unsigned char binVal )const// changing value pointed to, not a member.
{
    unsigned char* pTgt = pByte + n/8;
    n %= 8;
    unsigned char mask = 1;
 //   if( n > 0 ) mask << n;// tried. this does not work
    while( n > 0 )
    {
        mask *= 2;
        --n;
    }

    if( binVal == 1 )// write 1
    {
        // bitwise or with eg. 00000100 for n = 2
        *pTgt = *pTgt | mask;
    }
    else// write 0 to n th bit
    {
        // bitwise and with eg 11111011 for n = 2
        mask = 255 - mask;
        *pTgt = *pTgt & mask;
    }

    return;
}

// copy from or write to entire block of capBytes Bytes
void bitArray::copyFrom( const unsigned char* pSrc )
{
    for( unsigned int k = 0; k < capBytes; ++k )
        *( pByte + k ) = *( pSrc + k );
}

void bitArray::copyTo( unsigned char* pTgt )const
{
    for( unsigned int k = 0; k < capBytes; ++k )
        *( pTgt + k ) = *( pByte + k );
}

void bitArray::view( unsigned int bitsPerRow )const
{
    const unsigned char* pIter = pByte;
//    pIter += pByte;

    unsigned int bitCount = 0;
    unsigned char mask = 1;

    unsigned int numBits = sizeBits;
    while( numBits > 0 )
    {
        // write a bit to console
        unsigned char result = *pIter & mask;
        if( result > 0 ) std::cout << '1';
        else std::cout << '0';

        // cycle mask
        if( mask < 128 ) mask *= 2;
    //    if( mask < 128 ) mask << 1;// does not work to replace *= 2
        else
        {
            mask = 1;
            ++pIter;
        }

        // new line
        if( ( ++bitCount )%bitsPerRow == 0 )
            std::cout << '\n';

        --numBits;
    }
}
