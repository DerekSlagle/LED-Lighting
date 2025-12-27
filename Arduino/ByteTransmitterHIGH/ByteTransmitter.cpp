#include "ByteTransmitter.h"

void ByteTransmitter::setup( bool is_Sender, uint8_t* p_Buff, int PinBit, int PinSend, int PinConfirm )
{
    isSender = is_Sender;
    pBuff = p_Buff;
    pinBit = PinBit;
    pinSend = PinSend;
    pinConfirm = PinConfirm;

    toggleSend = toggleConfirm = HIGH;// each byte ends in this state. Pull high to start
    // EVERYTHING = 0
    bitCount = 0;// not 8. Ready to read or write
    byteCount = 0;
    byteValue = 0;
    dataSize = 0;// must be assigned before use
    //BuffSize = ??;

    if( isSender )
    {
        // 2 out
        pinMode( pinSend, OUTPUT );
        pinMode( pinBit, OUTPUT );
        // 1 in
        pinMode( pinConfirm, INPUT_PULLUP );
    }
    else// is receiver
    {
        // 2 in
        pinMode( pinSend, INPUT_PULLUP );
        pinMode( pinBit, INPUT_PULLUP );
        // 1 out
        pinMode( pinConfirm, OUTPUT );
    }
}

// for role swap NOT READY
void ByteTransmitter::setAsSender()
{
    isSender = true;
    toggleSend = toggleConfirm = HIGH;// each byte ends in this state. Pull high to start
    bitCount = 0;// not 8. Ready to read or write
    byteCount = 0;
    byteValue = 0;// ready for new value
    dataSize = 0;
    // 2 out
        pinMode( pinSend, OUTPUT );
        pinMode( pinBit, OUTPUT );
        // 1 in
        pinMode( pinConfirm, INPUT_PULLUP );
}

void ByteTransmitter::setAsReceiver()
{
    isSender = false;
    toggleSend = toggleConfirm = 0;// each byte ends in this state. Pull high to start
    bitCount = 0;// not 8. Ready to read or write
    byteCount = 0;
    byteValue = 0;// ready for new value
    dataSize = 0;// assign before use
    // 2 in
        pinMode( pinSend, INPUT_PULLUP );
        pinMode( pinBit, INPUT_PULLUP );
        // 1 out
        pinMode( pinConfirm, OUTPUT );
    
}

// receiver check once per loop for changed pinSend value then send goes all at once
bool ByteTransmitter::Monitor()// false if no toggle. true when download complete
{
    // receiver reads from pinSend and pinBit and writes to pinConfirm
    if( toggleSend == digitalRead( pinSend ) ) return false;// maybe next loop()
    toggleSend = digitalRead( pinSend );// value toggled

    // sender has written 1st bit. the download is on
    byteCount = 0;
    bitCount = 0;
    byteValue = 0;
    toggleConfirm = HIGH;// toggle before writing

    // send signal above the 1st bit
    uint8_t bitVal = digitalRead( pinBit );// read from pinBit
    byteValue = byteValue | ( bitVal << bitCount );// byteValue target bit = 0

    toggleConfirm = !toggleConfirm;
    digitalWrite( pinConfirm, toggleConfirm );
    bitCount = 1;

    // adapt from SendData()
    // go until all dataSize bytes are sent
    while( bitCount < 8 )
    {
        readBit();// Leaves bitCount = 8 when dataSize has been read
    }

    // toggleSend and toggleConfirm should = ? when done

    return true;
}

void ByteTransmitter::readBit()
{
    // get ok 1st
    while( toggleSend == digitalRead( pinSend ) );
    toggleSend = digitalRead( pinSend );

    // read next bit
    uint8_t bitVal = digitalRead( pinBit );// read from pinBit
    byteValue = byteValue | ( bitVal << bitCount );// byteValue target bit = 0

    if( ++bitCount == 8 )// process byte
    {
        pBuff[ byteCount ] = byteValue;
        byteValue = 0;
        if( ++byteCount < dataSize )
        {
            bitCount = 0;
        }
    }

    toggleConfirm = !toggleConfirm;
    digitalWrite( pinConfirm, toggleConfirm );
}

// sender just sends. Function returns when it is all sent
void ByteTransmitter::SendData()
{
    byteCount = 0;
    bitCount = 0;
    byteValue = pBuff[ byteCount ];    
    toggleSend = HIGH;// toggle before writing

    // go until all dataSize bytes are sent
    toggleConfirm = HIGH;// HIGH will be on pinConfirm until receiver writes Low after 1st bit
    while( bitCount < 8 )
        writeBit();// writes to pinBit and (toggled value to) pinSend. Leave bitCount = 8 when dataSize has been read    

    // toggleSend = ? and toggleConfirm = ? when done
}

// for sender
void ByteTransmitter::writeBit()
{
    // post next bit
    uint8_t bitVal = ( byteValue >> bitCount ) & 1;// low bit to high bit
    digitalWrite( pinBit, bitVal );
    
    // All bits sent are correct
  //  std::cout << ( ( *pinBit == 1 ) ? 1 : 0 ) << ' ';

    toggleSend = !toggleSend;
    digitalWrite( pinSend, toggleSend );    

    if( ++bitCount == 8 )
    {
        int val = byteValue;

        if( ++byteCount < dataSize )        
        {

            byteValue = pBuff[ byteCount ];// next byte
            bitCount = 0;
        }
        else byteValue = 0;
        // the last byte was written. This should not be called again while bitCount == 8
    }

    // confirm the read
    while( toggleConfirm == digitalRead( pinConfirm ) );// no signal yet
    toggleConfirm = digitalRead( pinConfirm );
}