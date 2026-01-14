#include <Arduino.h>
#include <FastLED.h>
#include <SD.h>// a micro SD drive
#include "LightTypes/LightPanel.h"
#include "FileParser.h"

// the full target rectangle
const int gridRows = 32, gridCols = 32;
const unsigned int numLEDs = static_cast<unsigned int>( gridRows*gridCols );

// now dynamic allocation
unsigned int numPanels = 4;
LightPanel* panel = nullptr;
// Light arrays
Light LightArr[numLEDs];// players write to this static array
CRGB leds[numLEDs];// panels map to this array from LightArr
Light clearLt;

void init_panels();// per coded configuration
bool init_SD();

#include "SSD_1306Component.h"// a tiny oled panel
SSD1306_Display theOled;

// input
#include "switchSPST.h"
switchSPST actButt, menuButt, rotEncButt;

// a rotary encoder to use in levels and adjust brightness at main menu
int pinEncoderCLK = 6, pinEncoderDT = 7, pinEncoderSW = 8;
//int lastClkState = HIGH;
volatile int lastClkState = HIGH;// assigned in updateEncoder()
//switchSPST pushButtEncSW( pinEncoderSW, 0.06f );// for encoder SW input
void updateEncoder();// callback for interrupt
volatile int rotEncoderValue = 64;// assigned in updateEncoder() to range 0 to 510. Use half value as brightness
int lastRotEncoderValue = rotEncoderValue;// change event detection
int rotEncoderDelta = 0;// = rotEncoderValue - lastRotEncoderValue

int LEDbrightness = 32;// default
#include "LightSourceTypes/LightGrid.h"
LightGrid MainLG( *LightArr, gridRows, gridCols );

#include "Alevel/LevelPerspective.h"
LevelPerspective lvlPerspect;

void setup()
{
    clearLt.setRGB( 0, 10, 20 );
  if( !init_SD() ) LightArr[0].setRGB(255,0,0);
  init_panels();

  // panel orientation test = ?
  //LightArr[0].setRGB(255,255,255);
 // LightArr[16].setRGB(255,255,255);
 // LightArr[24].setRGB(255,255,255);
 // LightArr[512].setRGB(255,255,255);

  // for encoder
  pinMode( pinEncoderCLK, INPUT_PULLUP );
  pinMode( pinEncoderDT, INPUT_PULLUP );
  pinMode( pinEncoderSW, INPUT_PULLUP );
  lastClkState = digitalRead(pinEncoderCLK);// initial value
  attachInterrupt(digitalPinToInterrupt(pinEncoderCLK), updateEncoder, CHANGE);
  
  theOled.setupDisplay();
  theOled.setTextColor( COLOR_WHITE );
  actButt.init( 2,  0.04f, false );
  menuButt.init( 3,  0.04f, true );
  menuButt.flipState = true;
  rotEncButt.init( 8,  0.04f, false );

//  MenuLine::setupInOut( theOled, &actButt, &menuButt, &rotEncButt, rotEncoderDelta );
  // all Shapes
//  Shape::bindToGrid( LightArr, gridRows, gridCols );
  // levels
  BaseLevel::setupBase( theOled, &actButt, &menuButt, &rotEncButt, rotEncoderDelta );
  lvlPerspect.bindToGrid( LightArr, gridRows, gridCols );
  lvlPerspect.setup();
  
  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness( LEDbrightness );  
}

void FixBadLamp();// assign -1 to just return
uint32_t lastTime = micros();
void loop()
{
    uint32_t currTime = micros();
    float dtLoop = ( currTime - lastTime )*0.000001f;
    lastTime = currTime;

    // set rotEnc delta
    rotEncoderDelta = 0;
    if( lastRotEncoderValue != rotEncoderValue )// encoder changed
    {
      // save delta!
      rotEncoderDelta = rotEncoderValue - lastRotEncoderValue;
      lastRotEncoderValue = rotEncoderValue;// reset
    }

    actButt.update(dtLoop);
    menuButt.update(dtLoop);
    rotEncButt.update(dtLoop);

    lvlPerspect.update( dtLoop );
    lvlPerspect.draw();
   
    // write to leds
    FastLED.clear();
    for( unsigned int p = 0; p < numPanels; ++p )
      panel[p].update();

    FixBadLamp();
    FastLED.show();

    // level writes to oled display
}

// copy back leds by 1 from badIndex to the end
Light* pBadLamp = leds + 754;
void FixBadLamp()
{
    // the fix?
    if( pBadLamp )
    {
      int numLts = gridRows*gridCols;
      // bound check
      if( pBadLamp < leds || pBadLamp >= leds + numLts )
        return;
      // fix it
      Light* copyIter = pBadLamp;
      while( copyIter + 1 < leds + numLts )
      {
        *copyIter = copyIter[1];
        ++copyIter;
      }
    }
}

void updateEncoder()// callback for interrupt
{
  int currentClkState = digitalRead(pinEncoderCLK);
  int currentDtState = digitalRead(pinEncoderDT);

  if (currentClkState != lastClkState) { // Check for a change in CLK
    if (currentDtState != currentClkState) { // Determine direction
      rotEncoderValue++; // Clockwise
    } else {
      rotEncoderValue--; // Counter-clockwise
    }    
  }

  lastClkState = currentClkState;
}

void init_panels()
{  
  Light* pLt0 = leds; 
  String fName = "/IVsensors/panels_init.txt";
  FileParser FP( fName.c_str() );
  int NP = 0;
  FP >> NP;// no support for unsigned int
  numPanels = NP;
  panel = new LightPanel[ numPanels];
  for( unsigned int p = 0; p < numPanels; ++p )// all same
  {
    panel[p]. init_Src( LightArr, gridRows, gridCols );
    int Rows, Cols, Row0, Col0;
    FP >> Rows >> Cols >> Row0 >> Col0;
    panel[p].set_SrcArea( Rows, Cols, Row0, Col0 );
    FP >> panel[p].rotIdx;
    FP >> panel[p].type;
    panel[p].pTgt0 = pLt0;
    pLt0 += Rows*Cols;
    // for special 32x8 case
    if( Cols > Rows ) 
      panel[p].swapTgtRCs = true;
    else
      panel[p].swapTgtRCs = false;
  } 
}

bool init_SD()
{
  File myFile;
  if (!SD.begin( 10 ))  
    return false;// FAIL!!
    
  return true;// Good

  // open main file
  String mainFileName = "/IVsensors/SD_initLevels.txt";
  FileParser FP( mainFileName.c_str() );
  
  // bad lamp fix
  int badIndex = 0;
  FP >> badIndex;
  if( badIndex >=0 && badIndex < gridRows*gridCols )
    pBadLamp = leds + badIndex;
  else
    pBadLamp = nullptr;

  return true;
}