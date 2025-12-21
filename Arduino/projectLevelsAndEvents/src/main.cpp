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

#include "ArduinoLevelTypes/AL_BasicA.h"
#include "ArduinoLevelTypes/AL_BasicB.h"
#include "ArduinoLevelTypes/ArdLvlScrolling.h"
String ALS_fileName;// for setup() call above type

char typeID = 'A';// just 'A' and 'B' for now
// returns pointer to new level. Check for nullptr
ArduinoLevel* startNewLevel();// new by typeID

#include "switchSPST.h"
switchSPST actButt( 2, 0.04f, false );// regular button. Press for selected Level
switchSPST menuButt( 3, 0.04f, true );// cap touch. Select a level

// a digital display
//#include "intDisplay.h"
//intDisplay intDisp_A;// brightness
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

  typeID = 'C';
  ArduinoLevel::pCurrLvl = nullptr;
  selectNewLevel();
//  ArduinoLevel::pCurrLvl = startNewLevel();
//  if( !ArduinoLevel::pCurrLvl )// Fail!!
  theOled.setupDisplay();
  theOled.setTextColor( COLOR_WHITE );
  theOled.clear();
  theOled.printAt( 0, 0, "Begin...", 1 );
  theOled.show();
  selectNewLevel();

  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness( LEDbrightness );
  
}

void GenerateAndHandleEvents();
uint32_t lastTime = micros();
int lastRotEncoderValue = rotEncoderValue;// change event detection
int rotEncoderDelta = 0;// = rotEncoderValue - lastRotEncoderValue
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
    // update rotary encoder and save delta for event.value
    // reset brightness?
    pushButtEncSW.update( dtLoop );// no event yet
    if( pushButtEncSW.pollEvent() == 1 )// press event
    {
      rotEncoderValue = 0;// double the brightness value
    }

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
          LightArr[n] =  Light(60,0,100);// Menu color
    }

    // write to leds
    FastLED.clear();
    for( unsigned int p = 0; p < numPanels; ++p )
      panel[p].update();

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
        // to the level
        if( ArduinoLevel::pCurrLvl )
        {
            if( !ArduinoLevel::pCurrLvl->handleEvent( AE ) )
            {
                delete ArduinoLevel::pCurrLvl;
                ArduinoLevel::pCurrLvl = nullptr;
                EventCount = 0;
                // back to the main menu
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
                    if( typeID == 'A' )// it is A
                        typeID = 'B';// make it B
                    else if( typeID == 'B' )// it is B
                        typeID = 'C';// make it C
                    else// it is C
                        typeID = 'A';// make it A

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
              LEDbrightness += rotEncoderDelta;
              if( LEDbrightness < 0 ) LEDbrightness = 0;
              else if( LEDbrightness > 255 ) LEDbrightness = 255;
              FastLED.setBrightness( LEDbrightness );
            }
        }
    }
    // all parsed out
    EventCount = 0;
}

void selectNewLevel()// callback for pushButtB
{
  if( ArduinoLevel::pCurrLvl ) return;// in use by Level

   // alternate
//  if( typeID == 'A' )// it is A
//      typeID = 'B';// make it B
//  else if( typeID == 'B' )// it is B
//      typeID = 'C';// make it C
 // else// it is C
 //     typeID = 'A';// make it A

  String str("Select Level");
  str += typeID == 'A' ? "\n* " : "\n  ";
  str += "Level A";
  str += typeID == 'B' ? "\n* " : "\n  ";
  str += "Level B";
  str += typeID == 'C' ? "\n* " : "\n  ";
  str += "Scroll Frames";

  theOled.clear();
  theOled.printAt( 0, 0, str.c_str(), 1 );
  theOled.show();
}

ArduinoLevel* startNewLevel()
{
    if( ArduinoLevel::pCurrLvl ) delete ArduinoLevel::pCurrLvl;
    ArduinoLevel::pCurrLvl = nullptr;

//  theOled.clear();
 // OledString = "new level type: ";
 // OledString += typeID;
 // theOled.printAt( 0, 0, OledString.c_str(), 1 );
 // theOled.show();

    switch( typeID )
    {
        case 'A' :
        {
            AL_BasicA* pALB_A = new AL_BasicA;
            // do type specific setup here
            pALB_A->bindToGrid( LightArr, gridRows, gridCols );
            pALB_A->setup( &theOled );
            // assign static master pointer
            ArduinoLevel::pCurrLvl = pALB_A;
        }
        break;

        case 'B' :
        {
            AL_BasicB* pALB_B = new AL_BasicB;
            // do type specific setup here
            pALB_B->bindToGrid( LightArr, gridRows, gridCols );
            pALB_B->setup( &theOled );
            // assign static master pointer
            ArduinoLevel::pCurrLvl = pALB_B;
        }
        break;

        case 'C' :
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

  return true;
}