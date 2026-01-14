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

// a 3 page menu: rect, circle, line
#include "Alevel/PageManager.h"
PageManager Menu;
MenuPage Page[5];// Home (0), Rextx2(1,4), Circle(2), Line(3)
bool gotoPage[5];
void setupMenu();

// Page[0] Home
MenuLine ML0_gotoPage1, ML0_gotoPage2, ML0_gotoPage3;

// Page[1]
#include "Shapes/RectangleShape.h"
RectangleShape rectA;
FloatLine FL_RectPosX, FL_RectPosY;
FloatLine FL_RectSzX, FL_RectSzY;
MenuLine ML1_gotoMore,  ML1_goHome;
// Page[4]
float angleXu = 0.0f, angleYu = 0.0f;
FloatLine FL_RectRotXu, FL_RectRotYu, FL_RectRotate;
bool updateXu = false, updateYu = false, rotateRect = false;
MenuLine ML4_goBack;// to Page[1]

// Page[2]
#include "Shapes/CircleShape.h"
CircleShape circleA;
FloatLine FL_CircPosX, FL_CircPosY, FL_CircRadius;
MenuLine ML2_goHome;

// Page[3]
#include "Shapes/LineShape.h"
LineShape lineA;
FloatLine FL_LinePosX, FL_LinePosY;
FloatLine FL_LineLength, FL_LineAngle;
float lineLength = 10.0f, lineAngle = 0.0f;
bool updateLinePos2 = false;// all write so new pos2 may be assigned
MenuLine ML3_goHome;

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

  MenuLine::setupInOut( theOled, &actButt, &menuButt, &rotEncButt, rotEncoderDelta );

  // a rectangle
  Shape::bindToGrid( LightArr, gridRows, gridCols );
  rectA.setup( 6, 10, Light(80,0,40), 6, 3 );
  // a circle
  circleA.setup( 4.0f, Light(0,80,40), 10, 20 );
  // a line
  lineA.setup( Light(0,40,100), 16, 16, 16, 26 );// length = 10
  lineLength = 10.0f;
  lineAngle = 0.0f;

  setupMenu();
  
  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness( LEDbrightness );  
}

void setupMenu()
{
  // Page[0] Home
  ML0_gotoPage1.setupBase( "Rectangle", gotoPage + 1 );
  ML0_gotoPage1.pNextLine = &ML0_gotoPage2;
  ML0_gotoPage2.setupBase( "Circle", gotoPage + 2 );
  ML0_gotoPage2.pNextLine = &ML0_gotoPage3;
  ML0_gotoPage3.setupBase( "Line", gotoPage + 3 );
  ML0_gotoPage3.pNextLine = nullptr;
  Page[0].setup( " -- Shapes Home --", ML0_gotoPage1 );

  // Page[1] Rectangle
  FL_RectPosX.setupBase( "col0: " );// head
  FL_RectPosX.setupFloat( rectA.pos.x, -20.0f, 50.0f );
  FL_RectPosX.inScale = 0.5f;
  FL_RectPosX.pNextLine = &FL_RectPosY;
  FL_RectPosY.setupBase( "row0: " );
  FL_RectPosY.setupFloat( rectA.pos.y, -20.0f, 50.0f );
  FL_RectPosY.inScale = 0.5f;
  FL_RectPosY.pNextLine = &FL_RectSzX;
  FL_RectSzX.setupBase( "Width: " );
  FL_RectSzX.setupFloat( rectA.Sz.x, 1.0f, 32.0f );
  FL_RectSzX.inScale = 0.5f;
  FL_RectSzX.pNextLine = &FL_RectSzY;
  FL_RectSzY.setupBase( "Height: " );
  FL_RectSzY.setupFloat( rectA.Sz.y, 1.0f, 32.0f );
  FL_RectSzY.inScale = 0.5f;
  FL_RectSzY.pNextLine = &ML1_gotoMore;
  ML1_gotoMore.setupBase( "More...", gotoPage + 4 );
  ML1_gotoMore.pNextLine = &ML1_goHome;
  ML1_goHome.setupBase( "Home", gotoPage );
  ML1_goHome.pNextLine = nullptr;
  Page[1].setup( " -- Rectangle --", FL_RectPosX );
  // Page[4] rectA More...
  FL_RectRotXu.setupBase( "Xu: ", &updateXu );
  FL_RectRotXu.doNotify = true;
  FL_RectRotXu.setupFloat( angleXu, -400.0f, 400.0f );
  FL_RectRotXu.inScale = 0.5f;
  FL_RectRotXu.pNextLine = &FL_RectRotYu;
  FL_RectRotYu.setupBase( "Yu: ", &updateYu );
  FL_RectRotYu.doNotify = true;
  FL_RectRotYu.setupFloat( angleYu, -400.0f, 400.0f );
  FL_RectRotYu.inScale = 0.5f;
  FL_RectRotYu.pNextLine = &FL_RectRotate;
  FL_RectRotate.setupBase( "Rotate: ", &rotateRect );
  FL_RectRotate.doNotify = true;
  FL_RectRotate.setupFloat( angleXu, -400.0f, 400.0f );// also writes to angleXu
  FL_RectRotate.inScale = 0.5f;
  FL_RectRotate.pNextLine = &ML4_goBack;
  ML4_goBack.setupBase( "Back...", gotoPage + 1 );
  ML4_goBack.pNextLine = nullptr;
  Page[4].setup( " -- More Rect --", FL_RectRotXu );

  // Page[2] Circle
  FL_CircPosX.setupBase( "col0: " );// head2
  FL_CircPosX.setupFloat( circleA.pos.x, -10.0f, 40.0f );
  FL_CircPosX.inScale = 0.5f;
  FL_CircPosX.pNextLine = &FL_CircPosY;
  FL_CircPosY.setupBase( "row0: " );
  FL_CircPosY.setupFloat( circleA.pos.y, -10.0f, 40.0f );
  FL_CircPosY.inScale = 0.5f;
  FL_CircPosY.pNextLine = &FL_CircRadius;
  FL_CircRadius.setupBase( "Radius: " );
  FL_CircRadius.setupFloat( circleA.Radius, 1.0f, 20.0f );
  FL_CircRadius.inScale = 0.5f;
  FL_CircRadius.pNextLine = &ML2_goHome;
  ML2_goHome.setupBase( "Home", gotoPage );
  ML2_goHome.pNextLine = nullptr;
  Page[2].setup( " -- Circle --", FL_CircPosX );

  // Page[3] Line
  FL_LinePosX.setupBase( "col0: ", &updateLinePos2 );
  FL_LinePosX.doNotify = true;
  FL_LinePosX.setupFloat( lineA.pos.x, -40.0f, 40.0f );
  FL_LinePosX.inScale = 0.5f;
  FL_LinePosX.pNextLine = &FL_LinePosY;
  FL_LinePosY.setupBase( "row0: ", &updateLinePos2 );
  FL_LinePosY.doNotify = true;
  FL_LinePosY.setupFloat( lineA.pos.y, -40.0f, 40.0f );
  FL_LinePosY.inScale = 0.5f;
  FL_LinePosY.pNextLine = &FL_LineLength;
  FL_LineLength.setupBase( "Length: ", &updateLinePos2 );
  FL_LineLength.doNotify = true;
  FL_LineLength.setupFloat( lineLength, 1.0f, 40.0f );
  FL_LineLength.inScale = 0.5f;
  FL_LineLength.pNextLine = &FL_LineAngle;
  FL_LineAngle.setupBase( "Angle: ", &updateLinePos2 );
  FL_LineAngle.doNotify = true;
  FL_LineAngle.setupFloat( lineAngle, -180.0f, 180.0f );
  FL_LineAngle.inScale = 0.5f;
  FL_LineAngle.pNextLine = &ML3_goHome;
  ML3_goHome.setupBase( "Home", gotoPage );
  ML3_goHome.pNextLine = nullptr;
  Page[3].setup( " -- Line --", FL_LinePosX );

  // PageManager
  Menu.setup( Page, gotoPage, 5 );
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

    // write to variables
    Menu.update();
    
    if( updateLinePos2 )
    {
      updateLinePos2 = false;
      lineA.pos2.x = lineA.pos.x + lineLength*cosf( lineAngle*0.017453f );
      lineA.pos2.y = lineA.pos.y + lineLength*sinf( lineAngle*0.017453f );
    }

    if( updateXu )
    {
        updateXu = false;
        rectA.Xu.x = 1.0f; rectA.Xu.y = 0.0f;
        rectA.Xu = rectA.Xu.Rotate( angleXu*0.01745f );
    }

    if( updateYu )
    {
        updateYu = false;
        rectA.Yu.x = 0.0f; rectA.Yu.y = 1.0f;
        rectA.Yu = rectA.Yu.Rotate( angleYu*0.01745f );
    }

    if( rotateRect )
    {
        rotateRect = false;
        rectA.Xu.x = 1.0f; rectA.Xu.y = 0.0f;
        rectA.Xu = rectA.Xu.Rotate( angleXu*0.01745f );
        rectA.Yu.x = -rectA.Xu.y;// right hand normal
        rectA.Yu.y = rectA.Xu.x;
        angleYu = angleXu;
        Menu.updateDisplay();
    }
    

    // write to LightArr
    MainLG.Clear( Light(10,20,20) );

    rectA.draw();
    circleA.draw();
    lineA.draw();
   
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