// display a full color image

#include <Arduino.h>
#include <FastLED.h>
#include <SD.h>// a micro SD drive

#include "FileParser.h"
//#include "SSD_1306Component.h"// a tiny oled panel

#include "LightPanel.h"

// the full target rectangle
const int gridRows = 32, gridCols = 32;
const unsigned int numLEDs = static_cast<unsigned int>( gridRows*gridCols );

const unsigned int numPanels = 4;
LightPanel panel[ numPanels ];


CRGB leds[numLEDs];// panels map to this array
Light clearLt;

#include "BufferSource.h"// includes LightGrid and LightSource
Light LightArr[numLEDs];// players write to this array
LightGrid LightArr_LG;// points to LightArr


// 2 buffer arrays
Light BuffA_Arr[numLEDs];
LightGrid BuffA_LG;
BufferSource BuffA_Src;// draw from BuffA_Arr to LightArr

Light BuffB_Arr[numLEDs];
LightGrid BuffB_LG;
BufferSource BuffB_Src;// draw from BuffB_Arr to LightArr

// transfer from B to A then load B
BufferSource BtoA_Src;// draw from BuffB_LG to BuffA_LG


// time delta
float dtLoop = 0.02f;// entire loop

void init_panels();// per coded configuration
bool init_SD();
//String imageName;
int numFiles = 0;// dynamically allocated to below
String* pFileNames = nullptr;
int fileIndex = 0;// cycle through
int* shuffleIndex = nullptr;// 1 per file
void shuffleElements();
//int imageHeight = gridRowsFull, imageWidth = gridColsFull;
void setImage( LightGrid& LG, const char* fName );
// shift new image in and old out
char shiftDirection = 'L';// cycle: L. U, R, D
void cycleShift();
void updateShift();

#include "switchSPST.h"
switchSPST pushButtA( 2, 0.4f, false );// regular button. Press for next image
// below called every 10 seconds or push button
float tElapImage = 0.0f, tDisplayImage = 8.0f;
bool updateLEDS = false;
void LoadNextImage()// callback for pushButtA and for timed loading
{
  fileIndex = ( 1 + fileIndex )%numFiles;
  if( fileIndex == 0 ) shuffleElements();
  BtoA_Src.draw();// copy image to BuffA_Arr
  setImage( BuffB_LG, pFileNames[ shuffleIndex[fileIndex] ].c_str() );
  cycleShift();
  BuffA_Src.draw();
  BuffB_Src.draw();  
  tElapImage = 0.0f;
  updateLEDS = true;
}

// a rotary encoder to adjust brightness
int pinEncoderCLK = 6, pinEncoderDT = 7, pinEncoderSW = 8;
int lastClkState = HIGH;
switchSPST pushButtEncSW( pinEncoderSW, 0.06f );// for encoder SW input
void updateEncoder();// callback for interrupt
volatile int LEDbrightness = 64;// assigned in updateEncoder() to range 0 to 510. Use half value as brightness

void setup()
{
  pushButtA.onClose = LoadNextImage;

  clearLt.setRGB( 0, 10, 20 );
  if( !init_SD() ) LightArr[520].setRGB(255,0,0);
  
  for( unsigned int n = 0; n < numLEDs; ++n )  
    LightArr[n] = clearLt;
  // panel orientation test = pass!
  LightArr[0].setRGB(255,255,255);
  LightArr[16].setRGB(255,255,255);
  LightArr[24].setRGB(255,255,255);
  LightArr[512].setRGB(255,255,255);

  // initialize LightGrids
  LightArr_LG.init( LightArr[0], gridRows, gridCols );// main grid
  BuffA_LG.init( BuffA_Arr[0], gridRows, gridCols );// buffer A  
  BuffB_LG.init( BuffB_Arr[0], gridRows, gridCols );// buffer B
  BuffB_LG.Clear( clearLt );// will copy to A initial image to A
  // initialize BufferSources
  BuffA_Src.setTarget( LightArr_LG );
  BuffA_Src.setSource( BuffA_LG );
  BuffA_Src.setPosition( 0, 0 );

  BuffB_Src.setTarget( LightArr_LG );
  BuffB_Src.setSource( BuffB_LG );
  BuffB_Src.setPosition( 0, BuffB_LG.cols - 1 );

  BtoA_Src.setTarget( BuffA_LG );
  BtoA_Src.setSource( BuffB_LG );
  BtoA_Src.setPosition( 0, 0 );

  fileIndex = -1;// will be incremented before use
  shiftDirection = 'D';// will be rotated to 'L'
  LoadNextImage();
  
  // set initial brightness  
  LEDbrightness = 64; 

  // for encoder
  pinMode( pinEncoderCLK, INPUT_PULLUP );
  pinMode( pinEncoderDT, INPUT_PULLUP );
  pinMode( pinEncoderSW, INPUT_PULLUP );
  lastClkState = digitalRead(pinEncoderCLK);// initial value  
  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(pinEncoderCLK), updateEncoder, CHANGE);

  init_panels();
  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness( LEDbrightness/2 );

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
    FastLED.setBrightness( LEDbrightness/2 );
    currBrightness = LEDbrightness;// reset
    updateLEDS = true;
  }
  
  tElapImage += dtLoop;
  if( tElapImage >= tDisplayImage )// next image
  {
    LoadNextImage();    
  }
  else// brightness change?
  {
    if( currBrightness != LEDbrightness )// encoder changed LEDbrightness
    {
      currBrightness = LEDbrightness;// reset
      FastLED.setBrightness( LEDbrightness/2 );
      updateLEDS = true;      
    }    
  }

  updateShift();
  
  if( updateLEDS )
  {
    FastLED.clear();

    // map LightArr to leds
    for( unsigned int p = 0; p < numPanels; ++p )
      panel[p].update();
    
    FastLED.show();
    updateLEDS = false;
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
    panel[p]. init_Src( LightArr, gridRows, gridCols );
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

  FP >> numFiles;// images to cycle through
  pFileNames = new String[numFiles];
  for( int n = 0; n < numFiles; ++n )
  { FP >> pFileNames[n]; }
  // array for shuffled index map
  shuffleIndex = new int[numFiles];
  for( int n = 0; n < numFiles; ++n )
    shuffleIndex[n] = n;// initial order is straight

  return true;
}

void setImage( LightGrid& LG, const char* fName )
{
  // open the file for the image data
  FileParser FP( fName );
  int Cols = 1, Rows = 6;
  FP >> Rows >> Cols;
  if( Cols > LG.cols ) Cols = LG.cols;// check
  if( Rows > LG.rows ) Rows =  LG.rows;// check  

  int rd, gn, bu, al;// each Light
  for( int X = 0; X < Cols; ++X )
  {
      for( int Y = 0; Y < Rows; ++Y )
      {
        FP >> rd >> gn >> bu >> al;
        if( al == 0 )
        {
      //    LightBuff.pLt0[ Cols*Y + X ] = gridColor;
          LG.pLt0[ Cols*Y + X ] = clearLt;
        }
        else LG.pLt0[ Cols*Y + X ].setRGB( rd, gn, bu );
      }
  }
}

void shuffleElements()
{
  int lastVal = shuffleIndex[numFiles - 1];// to compare with 1st after shuffle

  // swap last element with a random element
  for( int n = numFiles - 1; n > 0; --n )
  {
    int rand_n = rand()%(n+1);
    if( n == rand_n ) continue;;// leave it in place
    int temp = shuffleIndex[n];
    shuffleIndex[n] = shuffleIndex[rand_n];
    shuffleIndex[rand_n] = temp;
  }

  if( shuffleIndex[0] == lastVal )// oops. 1st same as last was
  {
    // swap 1st with middle element
    int temp = shuffleIndex[numFiles/2];
    shuffleIndex[numFiles/2] = shuffleIndex[0];
    shuffleIndex[0] = temp;
  }
}

void cycleShift()
{
  BuffA_Src.row0 = BuffA_Src.col0 = 0;// re center

    if( shiftDirection == 'L' )
    {
        shiftDirection = 'U';
        BuffB_Src.row0 = LightArr_LG.rows;
    }
    else if( shiftDirection == 'U' )
    {
        shiftDirection = 'R';
        BuffB_Src.col0 = -LightArr_LG.cols;
    }
    else if( shiftDirection == 'R' )
    {
        shiftDirection = 'D';
        BuffB_Src.row0 = -LightArr_LG.rows;
    }
    else
    {
        shiftDirection = 'L';
        BuffB_Src.col0 = LightArr_LG.cols;
    }
}

void updateShift()
{
  if( BuffB_Src.col0 == 0 && BuffB_Src.row0 == 0 ) return;// already home

  if( BuffB_Src.col0 > 0 )
  {
      --BuffB_Src.col0;
      BuffA_Src.col0 = BuffB_Src.col0 - BuffB_Src.cols;// A is left of B
  }
  else if( BuffB_Src.col0 < 0 )
  {
      ++BuffB_Src.col0;
      BuffA_Src.col0 = BuffB_Src.cols + BuffB_Src.col0;// A is right of B
  }

  if( BuffB_Src.row0 > 0 )
  {
      --BuffB_Src.row0;
      BuffA_Src.row0 = BuffB_Src.row0 - BuffB_Src.rows;// A is above B
  }
  else if( BuffB_Src.row0 < 0 )
  {
      ++BuffB_Src.row0;
      BuffA_Src.row0 = BuffB_Src.rows + BuffB_Src.row0;// A is below B
  }

  BuffA_Src.draw();
  BuffB_Src.draw();
  updateLEDS = true;
}