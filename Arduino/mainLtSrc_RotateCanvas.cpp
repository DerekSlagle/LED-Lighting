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
switchSPST pushButtA( 2, 0.04f, false );// regular button. Press for next image
switchSPST pushButtB( 3, 0.04f, true );// cap touch button. Press to rotate image
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

void cyclePanelOrientation();
void rotateCanvasCW();
void rotateCanvasCCW();

void RotateCanvas()// callback for pushButtB
{
//  cyclePanelOrientation();
//  setPanelOrientation();
  rotateCanvasCCW();
  updateLEDS = true;
}

// a rotary encoder to adjust brightness
int pinEncoderCLK = 6, pinEncoderDT = 7, pinEncoderSW = 8;
int lastClkState = HIGH;
switchSPST pushButtEncSW( pinEncoderSW, 0.06f );// for encoder SW input
void updateEncoder();// callback for interrupt
volatile int LEDbrightness = 64;// assigned in updateEncoder() to range 0 to 510. Use half value as brightness
// a numeric display for brightness
#include "IntegerSource.h"
IntegerSource brightDisplay;

void setup()
{
  pushButtA.onClose = LoadNextImage;
  pushButtB.onOpen = RotateCanvas;

  clearLt.setRGB( 0, 10, 20 );
  if( !init_SD() ) LightArr[520].setRGB(255,0,0);

  IntegerSource::init_static();
  // a numeric display
  brightDisplay.setTarget( LightArr_LG );
  // set initial brightness  
  LEDbrightness = 64;
  brightDisplay.setValue( LEDbrightness/2 );
  brightDisplay.bgLt.setRGB( 20,20,20 );
  brightDisplay.fgLt.setRGB( 200,200,200 );
  brightDisplay.setPosition( 2, 2 );
  
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
//  BuffB_Src.setPosition( 0, BuffB_LG.cols - 1 );
  BuffB_Src.setPosition( 0, 0 );
  BuffB_Src.cShift = BuffB_LG.cols - 1;

  BtoA_Src.setTarget( BuffA_LG );
  BtoA_Src.setSource( BuffB_LG );
  BtoA_Src.setPosition( 0, 0 );

  fileIndex = -1;// will be incremented before use
  shiftDirection = 'D';// will be rotated to 'L'
  LoadNextImage();
  
   

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
  pushButtA.update( dtLoop );
  pushButtB.update( dtLoop );
  // reset brightness?
  pushButtEncSW.update( dtLoop );  
  if( pushButtEncSW.pollEvent() == 1 )// press event
  {
    LEDbrightness = 64;// double the brightness value    
    FastLED.setBrightness( LEDbrightness/2 );
    currBrightness = LEDbrightness;// reset
    
    // re draw base image if number of digits reduced
    if( brightDisplay.setValue( LEDbrightness/2 ) ) BuffB_Src.draw();
    brightDisplay.draw();
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
      
      // re draw base image if number of digits reduced
      if( brightDisplay.setValue( LEDbrightness/2 ) ) BuffB_Src.draw();
      brightDisplay.draw();

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

char panelSide = 'U';// image is up
void cyclePanelOrientation()
{
  if( panelSide == 'U' ) panelSide = 'R';
  else if( panelSide == 'R' ) panelSide = 'D';
  else if( panelSide == 'D' ) panelSide = 'L';
  else panelSide = 'U';
}

void rotateCanvasCW()
{
  
  // new method for CW canvas rotation not working
  for( unsigned int n = 0; n < numPanels; ++n )
  {
    // increment each rotIdx
    
    // rotate each row0, col0 back
  //  int newRow0 = gridRows - 1 - panel[n].col0 - panel[n].cols;
    int newRow0 = gridRows - panel[n].col0 - panel[n].cols;
    int newCol0 = panel[n].row0;
    // swap rows, cols
    panel[n].set_SrcArea( panel[n].cols, panel[n].rows, newRow0, newCol0 );
    // assign swapTgtRCs = true if 8x24 is horizontal
    if( panel[n].cols != panel[n].rows )// 8x32
    {
      panel[n].swapTgtRCs = panel[n].cols > panel[n].rows;
      if( panel[n].swapTgtRCs )// went horizontal
      {
        if( panel[n].rotIdx == 0 ) panel[n].rotIdx = 2;
        else panel[n].rotIdx = 0;
      }
    }
    else// square panel
    {
      ++panel[n].rotIdx;
      if( panel[n].rotIdx > 2 ) panel[n].rotIdx = -1;
    }
  }

  return;
  

  switch( panelSide )
  {
    case 'U' :// done
    panel[0].set_SrcArea( 16, 16, 16, 0 );
    panel[0].rotIdx = -1;
    panel[1].set_SrcArea( 16, 16, 0, 0 );
    panel[1].rotIdx = 1;
    panel[2].set_SrcArea( 32, 8, 0, 16 );
    panel[2].rotIdx = 0;
    panel[2].swapTgtRCs = false;
    panel[3].set_SrcArea( 32, 8, 0, 24 );
    panel[3].rotIdx = 2;
    panel[3].swapTgtRCs = false;
    break;

    case 'R' :// done
    panel[0].set_SrcArea( 16, 16, 16, 16 );
    panel[0].rotIdx = 0;
    panel[1].set_SrcArea( 16, 16, 16, 0 );
    panel[1].rotIdx = 2;
    panel[2].set_SrcArea( 8, 32, 8, 0 );
    panel[2].rotIdx = 2;
    panel[2].swapTgtRCs = true;
    panel[3].set_SrcArea( 8, 32, 0, 0 );
    panel[3].rotIdx = 0;
    panel[3].swapTgtRCs = true;
    break;

    case 'D' :// done
    panel[0].set_SrcArea( 16, 16, 0, 16 );
    panel[0].rotIdx = 1;
    panel[1].set_SrcArea( 16, 16, 16, 16 );
    panel[1].rotIdx = -1;
    panel[2].set_SrcArea( 32, 8, 0, 8 );
    panel[2].rotIdx = 2;
    panel[2].swapTgtRCs = false;
    panel[3].set_SrcArea( 32, 8, 0, 0 );
    panel[3].rotIdx = 0;
    panel[3].swapTgtRCs = false;
    break;

    case 'L' :// done
    panel[0].set_SrcArea( 16, 16, 0, 0 );
    panel[0].rotIdx = 2;
    panel[1].set_SrcArea( 16, 16, 0, 16 );
    panel[1].rotIdx = 0;
    panel[2].set_SrcArea( 8, 32, 16, 0 );
    panel[2].rotIdx = 0;
    panel[2].swapTgtRCs = true;
    panel[3].set_SrcArea( 8, 32, 24, 0 );
    panel[3].rotIdx = 2;
    panel[3].swapTgtRCs = true;
    break;

    default :// same as 'U'
    panel[0].set_SrcArea( 16, 16, 16, 0 );
    panel[0].rotIdx = -1;
    panel[1].set_SrcArea( 16, 16, 0, 0 );
    panel[1].rotIdx = 1;
    panel[2].set_SrcArea( 32, 8, 0, 16 );
    panel[2].rotIdx = 0;
    panel[2].swapTgtRCs = false;
    panel[3].set_SrcArea( 32, 8, 0, 24 );
    panel[3].rotIdx = 2;
    panel[3].swapTgtRCs = false;
    break;
  }
}

void rotateCanvasCCW()
{
  for( unsigned int n = 0; n < numPanels; ++n )
  {
    // increment each rotIdx
    
    // rotate each row0, col0 forward
  //  int newRow0 = gridRows - 1 - panel[n].col0 - panel[n].cols;
    int newRow0 = panel[n].col0;
    int newCol0 = gridRows - panel[n].row0 - panel[n].rows;
    // swap rows, cols
    panel[n].set_SrcArea( panel[n].cols, panel[n].rows, newRow0, newCol0 );
    // assign swapTgtRCs = true if 8x24 is horizontal
    if( panel[n].cols != panel[n].rows )// 8x32
    {
      panel[n].swapTgtRCs = panel[n].cols > panel[n].rows;
      if( !panel[n].swapTgtRCs )// went vertical
      {
        if( panel[n].rotIdx == 0 ) panel[n].rotIdx = 2;
        else panel[n].rotIdx = 0;
      }
    }
    else// square panel
    {
      --panel[n].rotIdx;
      if( panel[n].rotIdx < -1 ) panel[n].rotIdx = 2;
    }
  }
}

void init_panels()
{
  // each panel. Common member values
  Light* pLt0 = leds;
  for( unsigned int p = 0; p < numPanels; ++p )// all same
  {
    panel[p]. init_Src( LightArr, gridRows, gridCols );
    panel[p].type = 2;
    panel[p].pTgt0 = pLt0;
    pLt0 += 256;
  }

  // initial orientation
  panel[0].set_SrcArea( 16, 16, 16, 0 );
  panel[0].rotIdx = -1;
  panel[1].set_SrcArea( 16, 16, 0, 0 );
  panel[1].rotIdx = 1;
  panel[2].set_SrcArea( 32, 8, 0, 16 );
  panel[2].rotIdx = 0;
  panel[2].swapTgtRCs = false;
  panel[3].set_SrcArea( 32, 8, 0, 24 );
  panel[3].rotIdx = 2;
  panel[3].swapTgtRCs = false;

//  setPanelOrientation();  
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
  // BuffA_Src.row0 = BuffA_Src.col0 = 0;// re center
  //   BuffA_Src.rShift = BuffA_Src.cShift = 0;// now in updateShift()

    if( shiftDirection == 'L' )
    {
        shiftDirection = 'U';
        BuffB_Src.rShift = BuffB_LG.rows;
    }
    else if( shiftDirection == 'U' )
    {
        shiftDirection = 'R';
      //  BuffB_Src.col0 = -LightArr_LG.cols;
        BuffB_Src.cShift = -BuffB_LG.cols;
    }
    else if( shiftDirection == 'R' )
    {
        shiftDirection = 'D';
      //  BuffB_Src.row0 = -LightArr_LG.rows;
        BuffB_Src.rShift = -BuffB_LG.rows;
    }
    else
    {
        shiftDirection = 'L';
      //  BuffB_Src.col0 = LightArr_LG.cols;
        BuffB_Src.cShift = BuffB_LG.cols;
    }
}

void updateShift()
{
//  if( BuffB_Src.col0 == 0 && BuffB_Src.row0 == 0 ) return;// already home
  if( BuffB_Src.cShift == 0 && BuffB_Src.rShift == 0 ) return;// already home

  if( BuffB_Src.cShift > 0 )
  {
      --BuffB_Src.cShift;
      BuffA_Src.cShift = BuffB_Src.cShift - BuffB_Src.getCols();// A is left of B
  }
  else if( BuffB_Src.cShift < 0 )
  {
      ++BuffB_Src.cShift;
      BuffA_Src.cShift = BuffB_Src.getCols() + BuffB_Src.cShift;// A is right of B
  }

  if( BuffB_Src.rShift > 0 )
  {
      --BuffB_Src.rShift;
      BuffA_Src.rShift = BuffB_Src.rShift - BuffB_Src.getRows();// A is above B
  }
  else if( BuffB_Src.rShift < 0 )
  {
      ++BuffB_Src.rShift;
      BuffA_Src.rShift = BuffB_Src.getRows() + BuffB_Src.rShift;// A is below B
  }

  BuffA_Src.draw();
  BuffB_Src.draw();

  // last iteration
  if( BuffB_Src.cShift == 0 && BuffB_Src.rShift == 0 )
  {
   //   brightDisplay.draw();
      BuffA_Src.rShift = BuffA_Src.cShift = 0;// reset
  }

  updateLEDS = true;
}