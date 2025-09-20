#include <Arduino.h>
// SUPPLIER and CONSUMER program. Set value of isConsumer below
// supplier collects digital and analog input from connected devices
// consumer calls to download current input data

#include "SSD_1306Component.h"
// a tiny oled screen
SSD1306_Display tinyOledA;

#include "ByteTransmitter.h"
ByteTransmitter theBT;

//*** supplier of data ***
// state of 4 buttons and 3 analog inputs
// digital input
#include "switchSPST.h"
const int numButts = 4;
const int pinButt[ numButts ] = { 2,3,4,5 };// digital in
switchSPST Button[ numButts ];// digital input
// analog input
#include "slidePot.h"
const int numPots = 3;// 1 pot + a 2 axis thumbstick
const int pinPot[ numPots ] = { A0, A1, A2 };// analog in
slidePot Pot[ numPots ];// to stabilize analog input
const int pinDout= 13;// Digital Value assigned from Button[0] on close

#include "bitArray.h"
bitArray stateBA;// to manage bit storage NOT USABLE YET
// storage for the state data: 1 bit/button + 12 bits/pot
const int stateSize = 1 + ( numButts + 12*numPots )/8;// over by <= 1 byte
uint8_t stateData[ stateSize ];// maintain for quick upload
void updateInputState();

// *** consumer ***
// *** things to keep  track of ***
// based on values returned by micros()
// in microseconds
uint32_t dtLoop = 0;// entire loop
uint32_t dtUpload = 0;// across upload
uint16_t potVal[numPots];// storage for transmitted values
const int pinAout = A3;// Pot[2].steadyVal

// the board receiving the bulk data is the consumer
bool isSender = true;//************** to enable for upload

void setup()
{       
  if( isSender )
  {
    // transmitter
    theBT.setup( 7, 8, 9, true );// is sender

    // bit manager
    stateBA.initClear( stateData[0], 6 );
    stateBA.sizeBits = 4 + 3*12;
  //  for( int n = 0; n < stateSize; ++n )    
  //    stateData[n] = 0;
    
    theBT.byteValue = stateData[0];

    // the buttons
    for( int n = 0; n < numButts; ++n )
    {
      pinMode( pinButt[n], INPUT_PULLUP );
      Button[n].init( pinButt[n], 0.04f );
    }
    // the pots
    Pot[0].setup( pinPot[0], 3, 2, 2 );// range = 512
    Pot[1].setup( pinPot[1], 3, 2, 2 );// range = 512
    Pot[2].setup( pinPot[2], 4, 2, 2 );// range = 256 for analog out
    
    // digital out to mosfet
    pinMode( pinDout, OUTPUT );
  }
  else// is receiver
  {
    theBT.setup( 7, 8, 9, false );// is receiver

    // bit manager
    stateBA.initClear( stateData[0], 6 );
    stateBA.sizeBits = 4 + 3*12;
  //  for( int n = 0; n < stateSize; ++n )
  //    stateData[n] = 0;

    for( int n = 0; n < numPots; ++n )
      potVal[n] = 0;

    // a tiny oled screen for receiver program
    tinyOledA.setupDisplay();
    tinyOledA.setTextColor( COLOR_WHITE );

    tinyOledA.clear();    
    tinyOledA.printAt( 0, 0, "Start", 1 );
    tinyOledA.show();
  }
      
}

// helpers for bitwise read or write of types from an array of uint8_t
uint16_t readADCvalue( uint8_t*& pByte, uint16_t& bitCount )
{
  uint16_t aVal = 0;// buildup with 12 bits
  for( uint8_t b = 0; b < 12; ++b )// all 12 bits
  {
    // process bits from low to high
    uint16_t aBit = ( *pByte >> bitCount%8 ) & 1;// low bit after shift
    ++bitCount;
    if( bitCount%8 == 0 ) ++pByte;// next byte

    aVal = aVal | ( aBit << b );    
  }
  
  return aVal;
}

void writeADCvalue( uint16_t val, uint8_t*& pByte, uint16_t& bitCount )
{
  for( uint8_t b = 0; b < 12; ++b )
  {
    // low to high
    uint8_t aBit = ( ( val >> b ) & 1 ) == 1 ? 1 : 0;
    *pByte = *pByte | ( aBit << bitCount%8 );// buildup low to high
    ++bitCount;
    if( bitCount%8 == 0 )// advance to next byte
    {
      ++pByte;
      *pByte = 0;// prepare for buildup
    }
  }
}

uint8_t readBit( uint8_t*& pByte, uint16_t& bitCount )
{
  uint8_t aBit = ( *pByte >> bitCount%8 ) & 1;// low bit after shift
  ++bitCount;
  if( bitCount%8 == 0 ) ++pByte;// next byte
  return aBit;
}

void writeBit( uint8_t bitVal, uint8_t*& pByte, uint16_t& bitCount )
{
  *pByte = *pByte | ( bitVal << bitCount%8 );// buildup low to high
  ++bitCount;
  if( bitCount%8 == 0 )// advance to next byte
    ++pByte;
}

// bitArray versions
uint16_t readADCvalue( uint16_t& bitCount )
{
  uint8_t* pByte = stateBA.pByte + bitCount/8;
  uint16_t aVal = 0;// buildup with 12 bits
  for( uint8_t b = 0; b < 12; ++b )// all 12 bits
  {
    // process bits from low to high
    uint16_t aBit = ( *pByte >> bitCount%8 ) & 1;// low bit after shift
    ++bitCount;
    if( bitCount%8 == 0 ) ++pByte;// next byte

    aVal = aVal | ( aBit << b );    
  }
  
  return aVal;
}

void writeADCvalue( uint16_t val, uint16_t& bitCount )
{
  uint8_t* pByte = stateBA.pByte + bitCount/8;
  for( uint8_t b = 0; b < 12; ++b )
  {
    // low to high
    uint8_t aBit = ( ( val >> b ) & 1 ) == 1 ? 1 : 0;
    *pByte = *pByte | ( aBit << bitCount%8 );// buildup low to high
    ++bitCount;
    if( bitCount%8 == 0 )// advance to next byte
    {
      ++pByte;
      *pByte = 0;// prepare for buildup
    }
  }
}

uint8_t readBit( uint16_t& bitCount )
{
  uint8_t* pByte = stateBA.pByte + bitCount/8;
  uint8_t aBit = ( *pByte >> bitCount%8 ) & 1;// low bit after shift
  ++bitCount;
  return aBit;
}

void writeBit( uint8_t bitVal, uint16_t& bitCount )
{
  uint8_t* pByte = stateBA.pByte + bitCount/8;
  *pByte = *pByte | ( bitVal << bitCount%8 );// buildup low to high
  ++bitCount;
}

void loop()
{
  // timing for whole (previous) loop
  static uint32_t lastTime = micros();// 1st call only
  uint32_t currTime = micros();
  dtLoop = currTime - lastTime;// in microseconds
  lastTime = currTime;// for next iteration

  
  if( theBT.isSender )
  {
    // keep the stateData fresh
    updateInputState();
    delay( 4 );// temp

    // 1st value ready for upload
    theBT.byteValue = stateData[0];
    for( int n = 0; n < 40; ++n )// listen up to 40 times
    {
      if( theBT.Monitor() )// 1st byte of stateData sent
      {
        // the rest
          for( int k = 1; k < stateSize; ++k )
          {
            theBT.byteValue = stateData[k];
            theBT.sendNextByte();
          }

          break;
      }

    }// end for 40 times

  }
  else// is receiver
  {
    // get the stateData    

    theBT.getByte();
    stateData[0] = theBT.byteValue;
    for( int n = 1; n < stateSize; ++n )
    {
      theBT.getNextByte();
      stateData[n] = theBT.byteValue;
    }

    // parse it to oled
    uint16_t bitCount = 0;
    stateBA.bitIter = 0;
    String msg = "button states";
    
//    uint8_t* pByte = stateData;// strip it for button states
    
    for( int n = 0; n < numButts; ++n )
    {
      if( n%6 == 0 ) msg += '\n';// 6 per line      
    //  uint8_t aBit = readBit( bitCount );
      uint8_t aBit = stateBA.readBit();
      msg += ( aBit == 1 ) ? " 1" : " 0";
    }// end for each Button

    msg += "\nAnalog";
    // the Pots
    for( int n = 0; n < numPots; ++n )
    {
    //  potVal[n] = readADCvalue( bitCount );
      potVal[n] = stateBA.readADCvalue();
      msg += "\nPot: ";
      msg += potVal[n];
    }
      

    tinyOledA.clear();    
    tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
    tinyOledA.show();

    // ** use values **
    // write to pinAout when potVal[2] changes
    static uint16_t svLast = potVal[2];// 1st time
    if(  potVal[2] != svLast )
    {
      svLast =  potVal[2];
      analogWrite( pinAout,  potVal[2] );
    }

    delay( 10 );// oled = 25ms already
  }// end else is receiver

}// end loop()

void updateInputState()
{
  // zero it
  for( int n = 0; n < stateSize; ++n )
    stateData[n] = 0;

  // digital in
//  uint8_t* pByte = stateData; 
  uint16_t bitCount = 0;// this guides all
  stateBA.sizeBits = 0;// reset for write of all
  for( int n = 0; n < numButts; ++n )
  {
    Button[n].update( 0.1f );
    // write button state to the next bit
  //  writeBit( Button[n].isClosed, bitCount );
    stateBA.writeBit( Button[n].isClosed ); 
  }


  int lastSteadyVal = Pot[2].steadyVal;// detect change to limit analogWrite frequency
  // analog in
  Pot[0].update();// 1 real
//  writeADCvalue( Pot[0].steadyVal, bitCount );
  stateBA.writeADCvalue( Pot[0].steadyVal );
  for( int n = 1; n < numPots; ++n )// 2 fake
  {
  //  Pot[n].update();// now connected
  //  writeADCvalue( Pot[n].steadyVal, pByte, bitCount );
  //  writeADCvalue( 1000*( n + 1 ), bitCount );// 1000, 2000, 3000
    stateBA.writeADCvalue( 1000*( n + 1 ) );// 1000, 2000, 3000
  }

  // temp: skip output below
  return;// early

  // for digital output on pinDout
  if( Button[0].pollEvent() != 0 )
  {
    if( Button[0].isClosed )
      digitalWrite( pinDout, HIGH );
    else
      digitalWrite( pinDout, LOW );
  }
  
  // analog out on pinAout when Pot[2].steadyVal changes
  if( Pot[2].steadyVal != lastSteadyVal )
  {
    analogWrite( pinAout, Pot[2].steadyVal );
  }

}