#include <Arduino.h>
#include <FastLED.h>
#include <SD.h>// a micro SD drive
#include "LightPanel.h"
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
String OledString;
bool doUpdateOled = false;

const int numLevels = 5;
int currLevel = 0;
#include "ArduinoLevelTypes/AL_GalleryViewer.h"
String ALGV_fileName;// for setup()
#include "ArduinoLevelTypes/AL_WavePlayers.h"
#include "ArduinoLevelTypes/ArdLvlScrolling.h"
String ALS_fileName;// for setup() call above type
#include "ArduinoLevelTypes/AL_WavePlayBlending.h"
String WPB_fileName;// for setup() call above type
#include "ArduinoLevelTypes/AL_RandomRings.h"

// returns pointer to new level. Check for nullptr
ArduinoLevel* startNewLevel();// new by typeID

#include "switchSPST.h"
switchSPST actButt( 2, 0.04f, false );// regular button. Press for selected Level
switchSPST menuButt( 3, 0.04f, true );// cap touch. Select a level

// a digital display
#include "LightSourceTypes/IntegerSource.h"
IntegerSource brightDisplay;// brightness
float tBrightDisp = 3.0f, tElapBrightDisp = 0.0f;//
LightGrid Main_LG( *LightArr, gridRows, gridCols );// 

// a rotary encoder to use in levels and adjust brightness at main menu
int pinEncoderCLK = 6, pinEncoderDT = 7, pinEncoderSW = 8;
//int lastClkState = HIGH;
volatile int lastClkState = HIGH;// assigned in updateEncoder()
switchSPST pushButtEncSW( pinEncoderSW, 0.06f );// for encoder SW input
void updateEncoder();// callback for interrupt
volatile int rotEncoderValue = 64;// assigned in updateEncoder() to range 0 to 510. Use half value as brightness
int LEDbrightness = 32;// default

void selectNewLevel();// was callback for pushButtB

void setup()
{
  clearLt.setRGB( 0, 10, 20 );
  if( !init_SD() ) LightArr[0].setRGB(255,0,0);
  init_panels();

  // panel orientation test = ?
  LightArr[0].setRGB(255,255,255);
  LightArr[16].setRGB(255,255,255);
  LightArr[24].setRGB(255,255,255);
  LightArr[512].setRGB(255,255,255);

  // for encoder
  pinMode( pinEncoderCLK, INPUT_PULLUP );
  pinMode( pinEncoderDT, INPUT_PULLUP );
  pinMode( pinEncoderSW, INPUT_PULLUP );
  lastClkState = digitalRead(pinEncoderCLK);// initial value
  attachInterrupt(digitalPinToInterrupt(pinEncoderCLK), updateEncoder, CHANGE);

  // brightness display
  IntegerSource::init_static();  
  brightDisplay.setTarget( Main_LG );
  brightDisplay.row0 = 2;
  brightDisplay.col0 = 2;
  brightDisplay.bgLt.setRGB( 20, 20, 20 );
  brightDisplay.fgLt.setRGB( 200, 200, 200 );
  brightDisplay.setValue( LEDbrightness );

  currLevel = 0;
  ArduinoLevel::pCurrLvl = nullptr;
  theOled.setupDisplay();
  theOled.setTextColor( COLOR_WHITE );  
  selectNewLevel();// show level menu

  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness( LEDbrightness );  
}

void GenerateAndHandleEvents();
uint32_t lastTime = micros();
int lastRotEncoderValue = rotEncoderValue;// change event detection
int rotEncoderDelta = 0;// = rotEncoderValue - lastRotEncoderValue
// TEMP testing leds. Work around a bad lamp at leds + badIndex
Light* pTestLt = leds + gridRows*gridCols - 296;
// copy back leds by 1 from *pBadLamp to the end
// pBadLamp assigned in SDinit()
void FixAndTest();// assign -1 to just return

void loop()
{
    uint32_t currTime = micros();
    float dtLoop = ( currTime - lastTime )*0.000001f;
    lastTime = currTime;

    for( int n = 0; n < gridRows*gridCols; ++n )
        LightArr[n] = clearLt;

    // both buttons are used by levels
    // no use of callbacks. Events only
    actButt.update( dtLoop );//  act on menu selection
    menuButt.update( dtLoop );// scroll through a menu here and in level
    // update slidePots    
    // reset brightness?
    pushButtEncSW.update( dtLoop );// no event yet
    if( pushButtEncSW.pollEvent() == 1 )// press event
    {
      rotEncoderValue = 0;// double the brightness value
    }

    // update rotary encoder and save delta for event.value
    rotEncoderDelta = 0;
    if( lastRotEncoderValue != rotEncoderValue )// encoder changed
    {
      // save delta!
      rotEncoderDelta = rotEncoderValue - lastRotEncoderValue;
      lastRotEncoderValue = rotEncoderValue;// reset
    }

    GenerateAndHandleEvents();
    
    // update the level
    if( ArduinoLevel::pCurrLvl )
    {
      ArduinoLevel::update_stat(dtLoop);
      ArduinoLevel::draw_stat();
    }
    else// draw an "at menu" image
    {
        for( int n = 0; n < gridRows*gridCols; ++n )
          LightArr[n] =  Light(0,40,0);// Menu color
    }

    // the bright display
    if( tElapBrightDisp < tBrightDisp )
    {
      tElapBrightDisp += dtLoop;
      brightDisplay.draw();
    }

    // write to leds
    FastLED.clear();
    for( unsigned int p = 0; p < numPanels; ++p )
      panel[p].update();

    FixAndTest();

    FastLED.show();

    // update oled
    if( doUpdateOled )
    {
      doUpdateOled = false;
      theOled.clear();
      theOled.printAt( 0, 0, OledString.c_str(), 1 );
      theOled.show();
    }
}

// copy back leds by 1 from badIndex to the end
Light* pBadLamp = nullptr;
void FixAndTest()
{
    // the test
    if( !ArduinoLevel::pCurrLvl )// at home only
    {
      for( int n = 0; n < 16; ++n )// 2 x 8 columns
      {
        Light* pLt = pTestLt + n;// global pTestLt is varied using rotary encoder
        if( pLt < leds ) 
          pLt = leds;// 1st element
        else if( pLt >= leds + gridRows*gridCols ) 
          pLt = leds + gridRows*gridCols - 1;// last element
        // write
        pLt->setRGB( 16*n, 0, 255 - 16*n );
      }
    }

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

// TEMP testing leds
bool menuButtPressed = false;
// an event system
const int maxEvents = 6;
ArduinoEvent ArdEvent[ maxEvents ];
int EventCount = 0;
void GenerateAndHandleEvents()
{
    // standardize for below
    const int actButtID = 1, menuButtID = 2;
    const int rotEncID = 1;// and type = 2
    ArduinoEvent AE;
    AE.type = 0;//buttEvent;// pressed: 1, released: -1
    AE.ID = 0;// no ID yet
    AE.value = 0;// not used for buttons
    // find which button    
    if( actButt.pollEvent() )
    {
        AE.type = actButt.pollEvent();// type: pressed: 1, released: -1
        AE.ID = actButtID;// act on selection
    }
    else if( menuButt.pollEvent() )
    {
        AE.type = menuButt.pollEvent();// type: pressed: 1, released: -1
        AE.type *= -1;// cap touch button. operation reversed
        AE.ID = menuButtID;// select next menu option
    }
    // add for rotary encoder (for value delta): add event type = 2
    else if( rotEncoderDelta != 0 )
    {
        AE.type = 2;
        AE.ID = 1;
        AE.value = rotEncoderDelta;// deltas are too high = 4
    }

    // add for slidePot: update + add event type = 3

    if( AE.ID != 0 )// add it
    {
        ArdEvent[ EventCount ] = AE;
        ++EventCount;// only one hit here
    } 

    // Parse them
    for( int n = 0; n < EventCount; ++n )
    {
        ArduinoEvent& AE = ArdEvent[n];
        // special case: toggle menuButtPressed
        if( AE.ID == menuButtID )
        {
          if( AE.type == 1 ) menuButtPressed = true;
          else if( AE.type == -1 ) menuButtPressed = false;
        }

        // "consume" an event meant just to adjust brightness
        if( menuButtPressed && AE.type == 2 )// rotary encoder
          continue;// do not pass on to level

        // to the level
        if( ArduinoLevel::pCurrLvl )
        {
            if( !ArduinoLevel::pCurrLvl->handleEvent( AE ) )
            {
                delete ArduinoLevel::pCurrLvl;
                ArduinoLevel::pCurrLvl = nullptr;
                EventCount = 0;
                // back to the main menu
                currLevel = 0;
                selectNewLevel();
                break;
            }
        }
        else// use theOled as level select
        {
            // handle event here
            if( AE.type == 1 )// button press
            {
                if( AE.ID == menuButtID )// cycle options
                {
                    currLevel = ( 1 + currLevel )%numLevels;
                    selectNewLevel();
                }
            }// load on  release so release does not happen in the level (option 1 is acted on)
            else if( AE.type == -1 )// button release
            {
                if( AE.ID == actButtID )// load the typeID level
                {
                    ArduinoLevel::pCurrLvl = startNewLevel();
                    for( int n = 0; n < gridRows*gridCols; ++n )
                      LightArr[n] = clearLt;
                }
            }
            else if( AE.type == 2 )// rotary encoder event
            {
              if( AE.ID == 1 )
              {
                // TEMP
                if( !menuButtPressed )
                {
                  pTestLt += rotEncoderDelta;
                }
              }
            }
        }
    }
    // all parsed out
    EventCount = 0;

    // adjust brightness if menuButt presses and rotEncoderDelta != 0
    if( menuButtPressed && rotEncoderDelta != 0 )
    {
      LEDbrightness += rotEncoderDelta;
      if( LEDbrightness < 0 ) LEDbrightness = 0;
      else if( LEDbrightness > 255 ) LEDbrightness = 255;
      FastLED.setBrightness( LEDbrightness );
      // brightness display
      tElapBrightDisp = 0.0f;
      brightDisplay.setValue( LEDbrightness );
    }
}

void selectNewLevel()// callback for pushButtB
{
  if( ArduinoLevel::pCurrLvl ) return;// in use by Level

  String str("** Select Level **");
  str += currLevel == 0 ? "\n* " : "\n  ";
  str += "Gallery Viewer";
  str += currLevel == 1 ? "\n* " : "\n  ";
  str += "Wave Player";
  str += currLevel == 2 ? "\n* " : "\n  ";
  str += "Scroll Frames";
  str += currLevel == 3 ? "\n* " : "\n  ";
  str += "WavePlay Blend";
  str += currLevel == 4 ? "\n* " : "\n  ";
  str += "Random Rings";

  theOled.clear();
  theOled.printAt( 0, 0, str.c_str(), 1 );
  theOled.show();
}

ArduinoLevel* startNewLevel()
{
    if( ArduinoLevel::pCurrLvl ) delete ArduinoLevel::pCurrLvl;
    ArduinoLevel::pCurrLvl = nullptr;

    switch( currLevel )
    {
        case 0 :
        {
            AL_GalleryViewer* pAL_GV = new AL_GalleryViewer;
            // do type specific setup here
            pAL_GV->bindToGrid( LightArr, gridRows, gridCols );
            if( pAL_GV->setup( ALGV_fileName.c_str(), &theOled ) )            
              ArduinoLevel::pCurrLvl = pAL_GV;
            else
              ArduinoLevel::pCurrLvl = nullptr;// Fail!
        }
        break;

        case 1 :
        {
            AL_WavePlayers* pAL_WP = new AL_WavePlayers;
            // do type specific setup here
            pAL_WP->bindToGrid( LightArr, gridRows, gridCols );
            if( pAL_WP->setup( &theOled ) )
              ArduinoLevel::pCurrLvl = pAL_WP;
            else
              ArduinoLevel::pCurrLvl = nullptr;// Fail!
        }
        break;

        case 2 :
        {
            ArdLvlScrolling* pALS = new ArdLvlScrolling;
            // do type specific setup here
            pALS->bindToGrid( LightArr, gridRows, gridCols );
            pALS->clearLt = clearLt;
            pALS->setup( ALS_fileName.c_str(), &theOled );
            // the images are loading sideways. Force correction here
            for( int n = 0; n < pALS->AniBuff_Src.numFrames; ++n )
              pALS->AniBuff_Src.pSrcBuff[n].RotateCW();
            // assign static master pointer
            ArduinoLevel::pCurrLvl = pALS;
        }
        break;

        case 3 :
        {
            AL_WavePlayBlending* pWPB = new AL_WavePlayBlending;
            // do type specific setup here
            pWPB->clearLt = clearLt;
            pWPB->bindToGrid( LightArr, gridRows, gridCols );
            if( pWPB->setup( WPB_fileName.c_str(), &theOled ) )           
              ArduinoLevel::pCurrLvl = pWPB;
            else
              ArduinoLevel::pCurrLvl = nullptr;// Fail!
        }
        break;

        case 4 :
        {
            AL_RandomRings* pRR = new AL_RandomRings;
            // do type specific setup here
            pRR->clearLt.setRGB(0,0,0);// black
            pRR->bindToGrid( LightArr, gridRows, gridCols );
            if( pRR->setup( &theOled ) )
              ArduinoLevel::pCurrLvl = pRR;
            else
              ArduinoLevel::pCurrLvl = nullptr;// Fail!
        }
        break;

        default:
        break;
    }

    return ArduinoLevel::pCurrLvl;
}

void init_panels()
{  
  Light* pLt0 = leds; 
  String fName = "/SourcePlayers/panels_init.txt";
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
  // open main file
  String mainFileName = "/SourcePlayers/SD_initLevels.txt";
  FileParser FP( mainFileName.c_str() );
  FP >> ALS_fileName;
  FP >> ALGV_fileName;
  FP >> WPB_fileName;
  // bad lamp fix
  int badIndex = 0;
  FP >> badIndex;
  if( badIndex >=0 && badIndex < gridRows*gridCols )
    pBadLamp = leds + badIndex;
  else
    pBadLamp = nullptr;

  return true;
}