#include <Arduino.h>
// Send a byte each button press or release (different values sent)
// SENDER and RECEVER program. Set value of isSender below

#include "SSD_1306Component.h"// a tiny oled panel
// a tiny oled screen
SSD1306_Display tinyOledA;

#include "ByteTransmitter.h"
ByteTransmitter theBT;

//*** sender ***
// transmit 8 bits per button event
#include "switchSPST.h"
const int numVals = 8;
//const int pinButt[ numButts ] = { 2,3,4,5 };// digital in
//switchSPST Button[ numButts ];// byte values are mapped to these
uint8_t byteVal[ numVals ];// for press and release events
int sendValIdx = 0;// for sender to cycle through byteVals

// *** receiver ***
switchSPST uploadButton;

// *** things to keep  track of ***
int byteCount = 0;
// based on values returned by micros()
// in milliseconds
float dtLoop = 0.02f;// entire loop
float dtUpload = 0.02f;// across upload

bool isSender = true;//************** to enable for upload

void setup()
{  
  // a tiny oled screen for receiver program
  tinyOledA.setupDisplay();
  tinyOledA.setTextColor( COLOR_WHITE );

  if( isSender )
  {
    theBT.setup( 7, 8, 9, true );// is sender

  //  for( int n = 0; n < numButts; ++n )
  //  {
  //    pinMode( pinButt[n], INPUT_PULLUP );
  //    Button[n].init( pinButt[n], 0.04f );
  //  }

    // prepare data: 8 values
    byteVal[0] = 10;
    byteVal[1] = 20;
    byteVal[2] = 30;
    byteVal[3] = 40;
    byteVal[4] = 210;
    byteVal[5] = 220;
    byteVal[6] = 230;
    byteVal[7] = 240;

    sendValIdx = 0;
    theBT.byteValue = byteVal[0];// sb 10
  }    
  else// is receiver
  {
    theBT.setup( 7, 8, 9, false );// is not sender
    uploadButton.init( 2, 0.04f );
  
    byteCount = 0;
    String msg = "byteCount = 0";
    tinyOledA.clear();    
    tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
    tinyOledA.show();
  }
      
}

int Monitor_callCount = 0;// should report as 1. increments if bitCount < 8 outside of Monitor() call
void loop() 
{
  // timing for whole loop
  static unsigned long lastTime = micros();// 1st call only

  unsigned long currTime = micros();
//  dtLoop = ( currTime - lastTime )*0.001f;// in milliseconds
  dtLoop = static_cast<float>( currTime - lastTime );// in microseconds
  lastTime = currTime;// for next iteration

  if( isSender )// on processor seperate from receiver
  { 
    if( theBT.bitCount < 8 ) ++Monitor_callCount;
    // listening for an upload
    if( theBT.Monitor() )// byteValue has been sent
    {
      
      ++Monitor_callCount;
      if( Monitor_callCount > 1 )
      {
        theBT.byteValue = Monitor_callCount;// for next time        
      }
      else
      {
        sendValIdx = ( 1 + sendValIdx )%numVals;
        theBT.byteValue = byteVal[ sendValIdx ];// for next time
      }
    
      Monitor_callCount = 0;// reset the count
      dtUpload = 1.0f*( micros() - currTime );
      // theBT.byteValue = static_cast<uint8_t>( dtUpload );// not yet 
    }
    
    delay(1.f);// for sender
  }
  else// is receiver
  {
    uploadButton.update(0.1f);

    if( uploadButton.pollEvent() == 1 )// press event
    {    
      unsigned long startTime = micros();
      if( theBT.getByte() )// the upload
      {
        dtUpload = static_cast<float>( micros() - startTime );// in microseconds
     
        // report byteIn        
        ++byteCount;
        String msg = "Byte: ";
        int ByteVal = theBT.byteValue;
        msg += ByteVal;
        msg += "\nbyteCount = ";
        msg += byteCount;
        msg += "\ndtLoop = ";
        msg += dtLoop;
        msg += "\ndtUpload = ";
        msg += dtUpload;
                        
        tinyOledA.clear();    
        tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
        tinyOledA.show();   
      }
      else// miss
      {
        String msg = "getByte() returned false";                        
        tinyOledA.clear();    
        tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
        tinyOledA.show();  
      }
    }  

    delay(1.f);// for receiver
  }
  
//  delay(2.f);
}// end of loop()