// display a full color image

#include <Arduino.h>
#include <FastLED.h>
#include <SD.h>// a micro SD drive

#include "FileParser.h"
//#include "SSD_1306Component.h"// a tiny oled panel

#include "LightPanel.h"

// the full target rectangle
const int gridRowsFull = 32, gridColsFull = 32;
const unsigned int numLEDs = static_cast<unsigned int>( gridRowsFull*gridColsFull );

const unsigned int numPanels = 4;
LightPanel panel[ numPanels ];

Light LightArr[numLEDs];// players write to this array
CRGB leds[numLEDs];// panels map above to this array
Light gridColor;

// time delta
float dtLoop = 0.02f;// entire loop

void init_panels();// per coded configuration
bool init_SD();
//String imageName;
int numFiles = 0;// dynamically allocated to below
String* pFileNames = nullptr;
int fileIndex = 0;// cycle through
int imageHeight = gridRowsFull, imageWidth = gridColsFull;
void setImage( const char* fName );

#include "switchSPST.h"
switchSPST pushButtA( 2, 0.4f, false );// regular button. Press for next image
// below called every 10 seconds or push button
float tElapImage = 0.0f, tDisplayImage = 10.0f;
void LoadNextImage()// for above
{
  for( unsigned int n = 0; n < numLEDs; ++n )  
    LightArr[n] = gridColor;
  fileIndex = ( 1 + fileIndex )%numFiles;
  setImage( pFileNames[fileIndex].c_str() );
  tElapImage = 0.0f;
}

// a digital display
#include "intDisplay.h"
intDisplay intDisp_A;// brightness
// a rotary encoder to adjust brightness
int pinEncoderCLK = 6, pinEncoderDT = 7, pinEncoderSW = 8;
int lastClkState = HIGH;
switchSPST pushButtEncSW( pinEncoderSW, 0.06f );// for encoder SW input
void updateEncoder();// callback for interrupt
volatile int LEDbrightness = 64;// assigned in updateEncoder() to range 0 to 510. Use half value as brightness

void setup()
{
  pushButtA.onClose = LoadNextImage;

  gridColor.setRGB( 0, 10, 20 );
  if( !init_SD() ) LightArr[520].setRGB(255,0,0);
  
  // panel orientation test = pass!
  LightArr[0].setRGB(255,255,255);
  LightArr[16].setRGB(255,255,255);
  LightArr[24].setRGB(255,255,255);
  LightArr[512].setRGB(255,255,255);

  // initial image loads to imageBuffA
  fileIndex = 0;
  setImage( pFileNames[fileIndex].c_str() );
  // display initial brightness
  intDisp_A.init( LightArr[0], gridRowsFull, gridColsFull, 1, 3, gridColor, Light(120,120,120) );
  LEDbrightness = 64;
  intDisp_A.value = LEDbrightness/2;// brightness
  intDisp_A.update();// writes to LightArr

  // for encoder
  pinMode( pinEncoderCLK, INPUT_PULLUP );
  pinMode( pinEncoderDT, INPUT_PULLUP );
  pinMode( pinEncoderSW, INPUT_PULLUP );
  lastClkState = digitalRead(pinEncoderCLK);// initial value
  
  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(pinEncoderCLK), updateEncoder, CHANGE);

  init_panels();
  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness( intDisp_A.value%255 );

  // write to leds
  FastLED.clear();
  // LightPanels map from LightArr to leds
  for( unsigned int p = 0; p < numPanels; ++p )
    panel[p].update(); 
	FastLED.show();
}


int currBrightness = LEDbrightness;// change event detection
void loop() 
{
  static unsigned long lastTime = micros();
  unsigned long currTime = micros();
  dtLoop = ( currTime - lastTime )*0.000001f;
  lastTime = currTime; 

  // LightArr is written to once per image load
  // load a new image?
  pushButtA.update( 0.2f );
  // reset brightness?
  pushButtEncSW.update( dtLoop );
  if( pushButtEncSW.pollEvent() == 1 )// press event
  {
    LEDbrightness = 64;// double the brightness value
  }

  bool updateLEDS = false;
  tElapImage += dtLoop;
  if( tElapImage >= tDisplayImage )// next image
  {
    LoadNextImage();
    updateLEDS = true;
  }
  else// brightness change?
  {
    if( currBrightness != LEDbrightness )// encoder changed
    {
      intDisp_A.value = LEDbrightness/2;// reset
      currBrightness = LEDbrightness;// reset
      FastLED.setBrightness( intDisp_A.value%255 );
      intDisp_A.update();
      updateLEDS = true;      
    }    
  }
  
  if( updateLEDS )
  {
    FastLED.clear();
    for( unsigned int p = 0; p < numPanels; ++p )
      panel[p].update(); 
    FastLED.show();
  }
  else
    delay(10); 

}// end of loop()

void updateEncoder()// callback for interrupt
{
  int currentClkState = digitalRead(pinEncoderCLK);
  int currentDtState = digitalRead(pinEncoderDT);

  if (currentClkState != lastClkState) { // Check for a change in CLK
    if (currentDtState != currentClkState) { // Determine direction
      if( LEDbrightness < 510 ) LEDbrightness++; // Clockwise
    } else {
      if( LEDbrightness > 0 ) LEDbrightness--; // Counter-clockwise
    }    
  }

  lastClkState = currentClkState;
}

void init_panels()
{
  // each panel. Common member values
  for( unsigned int p = 0; p < numPanels; ++p )// all same
  {
    panel[p]. init_Src( LightArr, gridRowsFull, gridColsFull );
    panel[p].type = 2;
  }

  // assign each panel in address order = order of connection in field
  panel[0].set_SrcArea( 16, 16, 16, 0 );
  panel[0].pTgt0 = leds;
  panel[0].rotIdx = -1;

  panel[1].set_SrcArea( 16, 16, 0, 0 );
  panel[1].pTgt0 = leds + 256;
  panel[1].rotIdx = 1;

  panel[2].set_SrcArea( 32, 8, 0, 16 );
  panel[2].pTgt0 = leds + 512;
  panel[2].rotIdx = 0;

  panel[3].set_SrcArea( 32, 8, 0, 24 );
  panel[3].pTgt0 = leds + 768;
  panel[3].rotIdx = 2;
}

bool init_SD()
{
  File myFile;
  if (!SD.begin( 10 ))
  {
    return false;
  }

  // get the file names for initializing players
  FileParser FP( "/fullColorFileNames.txt" );
  
  FP >> numFiles;
  pFileNames = new String[numFiles];
  for( int n = 0; n < numFiles; ++n )
  {
    FP >> pFileNames[n];
  }

  return true;
}

void setImage( const char* fName )
{
  // open the file for the image data
  FileParser FP( fName );
  imageWidth = imageHeight = 6;
  FP >> imageWidth >> imageHeight;
  if( imageWidth > gridColsFull ) imageWidth = gridColsFull;// check
  if( imageHeight > gridRowsFull ) imageHeight = gridRowsFull;// check

  int rd, gn, bu, al;// each Light
  for( int X = 0; X < imageWidth; ++X )
  {
    for( int Y = 0; Y < imageHeight; ++Y )
    {
      FP >> rd >> gn >> bu >> al;
      if( al == 0 )
      {
        LightArr[ gridColsFull*Y + X ] = gridColor;
      }
      else LightArr[ gridColsFull*Y + X ].setRGB( rd, gn, bu );
    }
  }
}