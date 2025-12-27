

#include <Arduino.h>
// SENDER and RECEVER program. Set value of isSender below

#include "SSD_1306Component.h"// a tiny oled panel
// a tiny oled screen
SSD1306_Display tinyOledA;

#include "ByteTransmitter.h"
ByteTransmitter theBT;

//*** sender ***
// transmit 8 bits per button event
const int DataSize = 8;
uint8_t theData[ DataSize ];// for press and release events
// for sender


bool isSupplier = false;//************** to enable for upload

// Task: Consumer starts as sender. It sends how many bytes ( 6 ) to return
// Consumer changes to receiver and receives the 6 bytes from Supplier
// Supplier starts as receiver. Receives DataSize then changes to sender and
// sends the 6 bytes.
uint16_t numBytesToSend = 6;

void setup()
{   

  if( isSupplier )
  {
    // prepare data: 8 values
    theData[0] = 10;
    theData[1] = 20;
    theData[2] = 30;
    theData[3] = 40;
    theData[4] = 210;
    theData[5] = 220;
    theData[6] = 230;
    theData[7] = 240;

    theBT.setup( false, theData, 7, 8, 9 );// starts as receiver
    theBT.BuffSize = DataSize;
    theBT.dataSize = 2;// as setup for SendData(). Expect 2 bytes

    delay(2000);// long enough for receiver to see pinSend = HIGH before SendData() pulls it LOW
  }    
  else// is Consumer
  {
    tinyOledA.setupDisplay();
    tinyOledA.setTextColor( COLOR_WHITE );

    for( int n = 0; n < DataSize; ++n ) theData[n] = 0;
    theBT.setup( true, theData, 7, 8, 9 );// starts as sender
    theBT.BuffSize = DataSize;
    theBT.dataSize = 2;// will send 2 bytes
    // the 2 bytes
    theBT.pBuff[0] = numBytesToSend;// request 6 bytes back
    theBT.pBuff[1] = 0;
  
    String msg = "initial pin states";
    msg += "\n pinBit: Input: ";
    msg += (int)digitalRead( theBT.pinBit );
    msg += "\n pinSend: Input: ";
    msg += (int)digitalRead( theBT.pinSend );
    msg += "\n pinConfirm: Output";
    

    tinyOledA.clear();    
    tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
    tinyOledA.show();

  //  delay(3000);
  }
      
}

int numTries = 0;
void loop()
{
  if( isSupplier )
  {
    if( theBT.isSender )// ends as
    {
      if( theBT.byteCount == 0 )
      {
        theBT.SendData();// done
      }
    }
    else// starts as receiver 
    {
      if( theBT.byteCount == 0 )
      {

        if( theBT.Monitor() )// got the data
        {
          // save the size sent
          uint16_t nextDataSize = theBT.pBuff[0];// sb = 6 but must include high byte
          theBT.setAsSender();
          theBT.dataSize = nextDataSize;
        }
      }
    }
  }
  else// is Consumer
  {
    if( theBT.isSender )// starts as sender
    {
      if( theBT.byteCount == 0 )
      {      
        String msg = "Sending: 2 bytes";
        tinyOledA.clear();    
        tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
        tinyOledA.show();

        theBT.SendData();// 6, 0

        msg = "\n 2 bytes sent";
        // switch to receiver
        theBT.setAsReceiver();
        theBT.dataSize = numBytesToSend;// 6
        msg += "\nReady to send 6 bytes";
        tinyOledA.clear();    
        tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
        tinyOledA.show();

        delay(3000);
      }
    }
    else// ends as receiver 
    {
      if( theBT.byteCount == 0 )
      {
        ++numTries;
        String msg = "Monitor tries = ";
        msg += numTries;

        if( theBT.Monitor() )// got the data
        {
          // report
          String msg = "**Rcvd: ";
          msg += (int)theBT.byteCount;
          msg += "\nSend ";
          msg += (int)theBT.toggleSend;
          msg += "  Conf ";
          msg += (int)theBT.toggleConfirm;
          for( int n = 0; n < theBT.byteCount; ++n )
          {
            if( n%4 == 0 ) msg += '\n';
            msg += (int)theBT.pBuff[n];
            msg += ' ';
          }
        }

        tinyOledA.clear();    
        tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
        tinyOledA.show();

        delay(1000);
      }
    }

  }// end else is Consumer

//  delay(100);
}