#include <Arduino.h>
// Send a byte each button press or release (different values sent)
// SUPPLIER and CONSUMER program. Set value of isConsumer below

#include "SSD_1306Component.h"// a tiny oled panel
// a tiny oled screen
SSD1306_Display tinyOledA;

#include "ByteTransmitter.h"
ByteTransmitter theBT;

//*** supplier of data ***
const int numVals = 8;
//const int pinButt[ numButts ] = { 2,3,4,5 };// digital in
//switchSPST Button[ numButts ];// byte values are mapped to these
uint8_t byteVal[ numVals ];// for press and release events
int sendValIdx = 0;// for sender to cycle through byteVals
const int numSizes = 5;
int sizeVal[ numSizes ];// consumer sends sizeIndex. supplier sends sizeVal[sizeIndex] as 1st 2 bytes

// *** consumer ***
#include "switchSPST.h"
// select # of bytes to upload
switchSPST sizeSelectButton;
//const int numSizes = 5;
//int sizeVal[ numSizes ];
int sizeIndex = 0;// scroll to select on oled display
// select next upload size on oled
void updateDataSize();
// trigger upload
switchSPST uploadButton;

// *** things to keep  track of ***
int byteLimit = 320;// bytes to send per button press. Assigned by menu selection
int byteCount = 0;// per transmission
int byteSum = 0;
// based on values returned by micros()
// in microseconds
uint32_t dtLoop = 0;// entire loop
uint32_t dtUpload = 0;// across upload

// the board receiving the bulk data is the consumer
// though it starts as sender to transmit byteLimit to supplier
bool isConsumer = false;//************** to enable for upload

void setup()
{  
  if( isConsumer )// the one with the oled display
  {
    theBT.setup( 7, 8, 9, true );// is sender initially
    uploadButton.init( 2, 0.04f );
    sizeSelectButton.init( 3, 0.04f, true );
    
    sizeIndex = 0;
    
    // CHEAT for value check: 8 values
    // consumer usually does not know these values
    byteVal[0] = 10;
    byteVal[1] = 20;
    byteVal[2] = 30;
    byteVal[3] = 40;
    byteVal[4] = 210;
    byteVal[5] = 220;
    byteVal[6] = 230;
    byteVal[7] = 240;

    sendValIdx = 0;
    theBT.byteValue = 123;// detect assign fail

    // a tiny oled screen for receiver program
    tinyOledA.setupDisplay();
    tinyOledA.setTextColor( COLOR_WHITE );
    // select next upload size on oled
    updateDataSize();
  //  String msg = "byteCount = 0\ndata size: ";
  //  msg += byteLimit;
  //  tinyOledA.clear();    
  //  tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
  //  tinyOledA.show();
  }    
  else// is supplier
  {
    theBT.setup( 7, 8, 9, false );// is receiver initially    

    // prepare data to send: 8 values
    byteVal[0] = 10;
    byteVal[1] = 20;
    byteVal[2] = 30;
    byteVal[3] = 40;
    byteVal[4] = 210;
    byteVal[5] = 220;
    byteVal[6] = 230;
    byteVal[7] = 240;

    // the size for each selection
    sizeVal[0] = 120;// bitSum = 15000
    sizeVal[1] = 600;// bitSum = 75000
    sizeVal[2] = 1024;// bitSum = 128000
    sizeVal[3] = 1800;// bitSum = 225000
    sizeVal[4] = 3000;// bitSum = 375000
    
    sendValIdx = 0;
    byteCount = 0;
    byteLimit = 0;// to be sent by consumer as 1st step
    theBT.byteValue = 123;// detect assign fail
  }
      
}

// for consumer to get byteLimit bytes
void getAllBytes()
{
  int badValue = -1;// error detection
  uint32_t startTime = micros();
  
//  theBT.getByte();// once to trigger upload and get 1st low byte for byteLimit
//  byteLimit = theBT.byteValue;// low byte 1st
 // getNextByte();// get high byte
//  int highVal = theBT.byteValue;
//  byteLimit += highVal << 8;
  // new
  byteLimit = theBT.getUint16( true );
  
  byteSum = 0;
  byteCount = 0;
  // straight through with the rest
  for( int n = 0; n < byteLimit; ++n )
  {
    theBT.getNextByte();
    // process uploaded value
    if( theBT.byteValue != byteVal[ byteCount%numVals ] ) badValue = theBT.byteValue;
    ++byteCount;
    byteSum += theBT.byteValue;
  }

  dtUpload = micros() - startTime;// in microseconds

  // report
  String msg = "Byte: ";
  int ByteVal = theBT.byteValue;
  msg += ByteVal;
  msg += "\nbyteCount = ";
  msg += byteCount;
  byteCount = 0;// reset for next time
  msg += "\ndtLoop = ";
  msg += dtLoop;
  msg += "\ndtUpload = ";
  msg += dtUpload;
  msg += "\nbyteSum = ";
  msg += byteSum;
  byteSum = 0;// for next time
  // each byte value is checked
  if( badValue < 0 )
  {
    msg += "\nAll values good!";
  }
  else// the last bad value read
  {
    msg += "\nbad value = ";
    msg += badValue;
  }
                  
  tinyOledA.clear();    
  tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
  tinyOledA.show();
}

// select next upload size on oled
const char* label[numSizes] = { "A", "B", "C", "D", "E" };
void updateDataSize()
{
  sizeIndex = ( 1 + sizeIndex )%numSizes;
  // display new
  String msg = "select size";
  for( int n = 0; n < numSizes; ++n )
  {
    msg += ( n == sizeIndex ) ? "\n* " : "\n  ";
   // msg += sizeVal[n];
    msg += label[n];
  }

  tinyOledA.clear();    
  tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
  tinyOledA.show();
}

void loop()
{
  // timing for whole (previous) loop
  static uint32_t lastTime = micros();// 1st call only
  uint32_t currTime = micros();
  dtLoop = currTime - lastTime;// in microseconds
  lastTime = currTime;// for next iteration

  if( isConsumer )
  {
    if( theBT.isSender )// 1st step. Send value of byteLimit to receiver on button press
    {
      delay( 10 );

      sizeSelectButton.update(0.1f);
      if( sizeSelectButton.pollEvent() == -1 )// press event
      {
        updateDataSize();// select sizeIndex
        return;
      }

      uploadButton.update(0.1f);
      if( uploadButton.pollEvent() == 1 )// press event
      {
        // assign byteLimit by calling theBT.getUint16()
        byteLimit = 0;// safe off

        // send sizeIndex in 1 byte
        theBT.sendByte( sizeIndex );// new        
        // switch to receiver of byteLimit bytes
        theBT.setAsReceiver();      
      }           
    }
    else// 2nd step: receive the byteLimit bytes
    {
      getAllBytes();
      // *** Done! Switch back to be ready for next sizeIndex send ***
      theBT.setAsSender();// to send value of byteLimit
    }
  }
  else// is supplier
  {
    if( theBT.isSender )// 2nd step. Send 2 bytes for byteLimit then Send the bytes to receiver
    {          
      // sender listening for an upload signal from receiver on pinValid
      if( theBT.isCalled() )// the call has arrived. toggle handled
      {
        // now send the data
        theBT.sendUint16( byteLimit );

        // continue a big upload. No start signal on pinValid
        sendValIdx = 0;
        for( int n = 0; n < byteLimit; ++n )
        {          
          theBT.sendNextByte( byteVal[ sendValIdx ] );// was no arg. theBT.byteValue is sent
          sendValIdx = ( 1 + sendValIdx )%numVals;
        }// for each remaining byte

        // *** Done! Switch back to be ready for next data request ***
        byteLimit = 0;// what would this cause? OK for supplier. = no data on consumer = BAD
        theBT.setAsReceiver();// receive value of byteLimit
        sendValIdx = 0;    
      }
      

    }
    else// 1st step: receive value of sizeIndex
    {
      // in 1 byte
      // this works
   //   if( theBT.Monitor() )// gets 1 byte
   //   {
    //    sizeIndex = theBT.byteValue;// is this member needed ?? 
   //     byteLimit = sizeVal[ sizeIndex%numSizes ];
        // change to sender
    //    theBT.setAsSender();        
   //   }

      // this does work, but nothing else can happen
   //   sizeIndex = theBT.getNextByte();// waits until call arrives that 1st bit is ready  
   //   byteLimit = sizeVal[ sizeIndex%numSizes ];
      // change to sender
   //   theBT.setAsSender();
   
      // 3rd method: It works!
      // as receiver
      if( theBT.isCalled() )// false if no change in pinToggle value
      {
        // 1st bit value is ready to be read
     //   sizeIndex = theBT.getFirstByte();// do not wait for a call. 1st bit is ready
        sizeIndex = theBT.getNextByte();
        byteLimit = sizeVal[ sizeIndex%numSizes ];
        // change to sender
        theBT.setAsSender();
      }

    }
  }
}