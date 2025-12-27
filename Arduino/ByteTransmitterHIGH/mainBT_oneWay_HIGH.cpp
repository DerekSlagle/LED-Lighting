

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

bool isSender = false;//************** to enable for upload

void setup()
{  
  

  if( isSender )
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

    theBT.setup( true, theData, 7, 8, 9 );
    theBT.BuffSize = DataSize;
    theBT.dataSize = DataSize;// as setup for SendData()

    delay(2000);// long enough for receiver to see pinSend = HIGH before SendData() pulls it LOW
  }    
  else// is receiver
  {
    tinyOledA.setupDisplay();
    tinyOledA.setTextColor( COLOR_WHITE );

    for( int n = 0; n < DataSize; ++n ) theData[n] = 0;
    theBT.setup( false, theData, 7, 8, 9 );// is not sender
    theBT.BuffSize = DataSize;
    theBT.dataSize = DataSize;// as setup for Monitor()
  
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
  if( isSender )
  {
    if( theBT.byteCount == 0 )
    {
        theBT.SendData();
    }
  }
  else// is receiver
  {
    if( theBT.byteCount == 0 )
    {
      ++numTries;
      String msg = "numTries = ";
      msg += numTries;
   //   msg += "\nbyteCount = ";
   //   msg += (int)theBT.byteCount;
      msg += "\n pinSend: ";
      msg += (int)digitalRead( theBT.pinSend );
      msg += "\n toggleSend: ";
      msg += (int)theBT.toggleSend;

      if( theBT.Monitor() )
      {
        int byteSum = 0;
        for( int n = 0; n < theBT.byteCount; ++n ) byteSum += theBT.pBuff[n];
          // report
          msg += "\n**Rcvd: ";
          msg += (int)theBT.byteCount;
          msg += "\nSend ";
          msg += (int)theBT.toggleSend;
          msg += "  Conf ";
          msg += (int)theBT.toggleConfirm;
     //     msg += "\n  Sum: ";
     //     msg += byteSum;
     //     msg += "  Vals:";
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
 //   else// ERROR
 //   {
 //     String msg = "FUCKED!!!";      
  //    msg += "\nbyteCount = ";
  //    msg += (int)theBT.byteCount;
  //    tinyOledA.clear();    
  //    tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
  //    tinyOledA.show();
 //     delay(3000);
 //   }

  }// end else is receiver

//  delay(100);
}