#ifndef BITARRAY_H
#define BITARRAY_H

//#include<iostream>// to write state to  console for viewing
#include<stdint.h>// to write state to  console for viewing

// special types to read or write on array
#include "Light.h"

// use an array of type unsigned char
class bitArray
{
    public:
    uint8_t* pByte = nullptr;
    uint16_t capBytes = 0;
    uint16_t sizeBits = 0;// role ??

    void initClear( uint8_t& Char0, uint16_t CapBytes );
    void init( uint8_t& Char0, uint16_t CapBytes );
    bitArray( uint8_t& Char0, uint16_t CapBytes ) { init( Char0, CapBytes ); }


 //   bitArray( const bitArray& ) = delete;// no copy
 //   bitArray& operator = ( const bitArray& ) = delete;// no assignment

    uint16_t ByteCapacity()const { return capBytes; }
    uint16_t bitCapacity()const { return 8*capBytes; }
    uint16_t bitSize()const { return sizeBits; }

    // copy from or write to entire block of capBytes Bytes
    void copyFrom( const unsigned char* pSrc );
    void copyTo( unsigned char* pTgt )const;

    // use when array represents all same sized groups: 1, 2, 3 or 4 bits each
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

    // hi bit 1st
    void pushDbl( uint8_t Val )// increments sizeBits by 2
    {
        setBit( sizeBits, Val/2 > 0 );// hi bit
        ++sizeBits;
        setBit( sizeBits, Val%2 > 0 );// lo bit
        ++sizeBits;
    }

    // writes to bits 2*n and 2*n + 1
    void setDblBit( unsigned int n, uint8_t Val )const;
    uint8_t getDblBit( unsigned int n )const;// reads from bits 2*n and 2*n + 1. returns 0 to 3

    void setTriBit( unsigned int n, uint8_t Val )const;
    uint8_t getTriBit( unsigned int n )const;// reads from bits 3*n, 3*n+1 and 3*n+2. returns 0 to 7
    void pushTri( uint8_t Val )// increments sizeBits by 2
    {
        setBit( sizeBits, Val/4 > 0 );// hi bit
        ++sizeBits;
        setBit( sizeBits, (Val%4)/2 > 0 );// mid bit
        ++sizeBits;
        setBit( sizeBits, Val%2 > 0 );// lo bit
        ++sizeBits;
    }

    void setQuadBit( unsigned int n, uint8_t Val )const;
    uint8_t getQuadBit( unsigned int n )const;// reads from bits 4*n to 4*n + 3. returns 0 to 15
    void pushQuad( uint8_t Val )// increments sizeBits by 2
    {
        setQuadBit( sizeBits/4, Val );
     //   setQuadBit( sizeBits, Val );
        sizeBits += 4;
    }
    
    // another use: 
    // Write a sequence of types: start at sizeBits = 0.
    // sizeBits is incremented to buildup array ( push value )
    // clear array 1st. Assign sizeBits = 0. write = push
    // construct a sequence of bits then deconstruct
    // as the same types in the same order
    // writeValue() increments sizeBits
    // Read a sequence of types: start at readBit = 0
    uint16_t bitIter = 0;// bit iterator for a read through the array
    uint8_t readBit();
    void writeBit( uint8_t bitVal );

    uint8_t readByte();
    void writeByte( uint8_t byteVal );

    uint16_t readUint16();
    void writeUint16( uint16_t val );

    uint16_t readADCvalue();// 12 bit ADC for analog in/out on esp32 microcontroller
    void writeADCvalue( uint16_t val );
    
    // for the LED projects on Arduino
    Light readLight();// order: red, green, blue
    void writeLight( Light Lt );

    bitArray(){}
    ~bitArray(){}

    protected:


};

#endif // BITARRAY_H
