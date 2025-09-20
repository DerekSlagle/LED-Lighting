#include "bitArray.h"

void bitArray::initClear( uint8_t& Char0, uint16_t CapBytes )// clears for fill via push()
{
    pByte = &Char0;
    capBytes = CapBytes;
    sizeBits = 0;// unsure if this will be usefull
    for( unsigned int k = 0; k < capBytes; ++k )
        *( pByte + k ) = 0;// to prepare for data via push(bool)
}

void bitArray::init( uint8_t& Char0, uint16_t CapBytes )// for prepared data
{
    pByte = &Char0;
    capBytes = CapBytes;
    sizeBits = 8*CapBytes;
}

bool bitArray::getBit( unsigned int n )const// return n th of 8*capBytes bits?
{
    const uint8_t* pTgt = pByte + n/8;// target Byte
    uint8_t Byte = *pTgt >> n%8;// target bit: 0 to 7
    return ( Byte & 1 ) > 0;// return bool value
}

uint8_t bitArray::getDblBit( unsigned int n )const//
{
    const uint8_t* pTgt = pByte + n/4;// target Byte
    uint8_t Byte = *pTgt >> 2*(n%4);// target bit: 0,2,4,6
    return ( Byte & 3 );// = 0,1,2 or 3
}

// working
uint8_t bitArray::getQuadBit( unsigned int n )const// reads from bits 4*n to 4*n + 3. returns 0 to 15
{
    uint8_t chkVal = 8*getBit(4*n) + 4*getBit(4*n+1) + 2*getBit(4*n+2) + getBit(4*n+3);
    return chkVal;// = 0 to 15
}

void bitArray::setTriBit( unsigned int n, uint8_t Val )const
{
    if( !pByte ) return;    

    setBit( 3*n, (Val >> 2 & 1) > 0 );
    setBit( 3*n + 1, (Val >> 1 & 1) > 0 );
    setBit( 3*n + 2, (Val & 1) > 0 );
}

uint8_t bitArray::getTriBit( unsigned int n )const// reads from bits 3*n, 3*n+1 and 3*n+2. returns 0 to 7
{
    uint8_t chkVal = 4*getBit(3*n) + 2*getBit(3*n+1) + getBit(3*n+2);
    return chkVal;// = 0 to 7
}

/*
uint8_t bitArray::getQuadBit( unsigned int n )const// reads from bits 4*n to 4*n + 3. returns 0 to 15
{
    const uint8_t* pTgt = pByte + n/2;// target Byte
    uint8_t Byte = *pTgt >> 4*(n%2);// target bit: 0,4    wrong?

    return ( Byte & 15 );// = 0 to 15
}
*/

void bitArray::setBit( unsigned int n, unsigned char binVal )const// changing value pointed to, not a member.
{
    uint8_t* pTgt = pByte + n/8;
    n %= 8;
    uint8_t mask = 1 << n;

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

// writes to bits 2*n and 2*n + 1
void bitArray::setDblBit( unsigned int n, uint8_t Val )const
{
    setBit( 2*n, Val/2 > 0 );// hi bit 1st
    setBit( 2*n + 1, Val%2 > 0 );// lo bit 2nd
}


// writes to bits 4*n to 4*n + 3
void bitArray::setQuadBit( unsigned int n, uint8_t Val )const
{
    setBit( 4*n, (Val >> 3 & 1) > 0 );// hi bit 1st
    setBit( 4*n + 1, (Val >> 2 & 1) > 0 );
    setBit( 4*n + 2, (Val >> 1 & 1) > 0 );
    setBit( 4*n + 3, (Val & 1) > 0 );
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

// construct a sequence of bits then deconstruct
// same types in same order
uint8_t bitArray::readBit()// returns value
{
    uint8_t* p_Byte = pByte + bitIter/8;
    uint8_t bitVal = ( *p_Byte >> bitIter%8 ) & 1;// low bit after shift
    ++bitIter;
    return bitVal;
}

void bitArray::writeBit( uint8_t bitVal )
{
    uint8_t* p_Byte = pByte + sizeBits/8;
    *p_Byte = *p_Byte | ( bitVal << sizeBits%8 );// buildup low to high
    ++sizeBits;
}

uint8_t bitArray::readByte()
{
    uint8_t* p_Byte = pByte + bitIter/8;
    uint8_t byteVal = 0;// buildup with 8 bits
    for( uint8_t b = 0; b < 8; ++b )// all 12 bits
    {
        // process bits from low to high
        uint8_t aBit = ( *p_Byte >> bitIter%8 ) & 1;// low bit after shift
        ++bitIter;
        if( bitIter%8 == 0 ) ++p_Byte;// next byte
        byteVal = byteVal | ( aBit << b );    
    }

    return byteVal;
}

void bitArray::writeByte( uint8_t byteVal )
{
    uint8_t* p_Byte = pByte + sizeBits/8;
    for( uint8_t b = 0; b < 8; ++b )
    {
        // low to high
        uint8_t aBit = ( ( byteVal >> b ) & 1 ) == 1 ? 1 : 0;
        *p_Byte = *p_Byte | ( aBit << sizeBits%8 );// buildup low to high
        ++sizeBits;
        if( sizeBits%8 == 0 )// advance to next byte        
            ++p_Byte;        
    }
}

uint16_t bitArray::readUint16()
{
    uint16_t loByte = readByte();
    uint16_t hiByte = readByte();
    return ( hiByte << 8 ) | loByte;
}

void bitArray::writeUint16( uint16_t val )
{
    writeByte( val%256 );// low byte
    writeByte( val/256 );// high byte
}

uint16_t bitArray::readADCvalue()
{
    uint8_t* p_Byte = pByte + bitIter/8;
    uint16_t aVal = 0;// buildup with 12 bits
    for( uint8_t b = 0; b < 12; ++b )// all 12 bits
    {
        // process bits from low to high
        uint16_t aBit = ( *p_Byte >> bitIter%8 ) & 1;// low bit after shift
        ++bitIter;
        if( bitIter%8 == 0 ) ++p_Byte;// next byte
        aVal = aVal | ( aBit << b );    
    }

    return aVal;
}

void bitArray::writeADCvalue( uint16_t val )
{
    uint8_t* p_Byte = pByte + sizeBits/8;
    for( uint8_t b = 0; b < 12; ++b )
    {
        // low to high
        uint8_t aBit = ( ( val >> b ) & 1 ) == 1 ? 1 : 0;
        *p_Byte = *p_Byte | ( aBit << sizeBits%8 );// buildup low to high
        ++sizeBits;
        if( sizeBits%8 == 0 )// advance to next byte        
            ++p_Byte;        
    }
}

// for the LED projects on Arduino
Light bitArray::readLight()// order: red, green, blue
{
    Light Lt;
    Lt.r = readByte();
    Lt.g = readByte();
    Lt.b = readByte();
    return Lt;
}

void bitArray::writeLight( Light Lt )
{
    writeByte( Lt.r );
    writeByte( Lt.g );
    writeByte( Lt.b );
}