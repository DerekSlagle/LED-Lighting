#include <Arduino.h>
#include <FastLED.h>

#include "LightPanel.h"
//#include "LightPlayer2.h"
//#include "WavePlayer2.h"
#include "DataPlayer_initFuncs.h"
//#include "DataPlayer.h"
#include "aniFloat.h"

#include "SSD_1306Component.h"// a tiny oled panel
// a tiny oled screen
SSD1306_Display tinyOledA;
// input devices
#include "switchSPST.h"
switchSPST pushButtA;
#include "slidePot.h"
slidePot potA( A0, 4, 3, 3 );

const unsigned int gridRows = 16, gridCols = 32;
const unsigned int numLEDs = gridRows*gridCols;

Light LightArr[numLEDs];// source array
CRGB leds[numLEDs];// target array

const unsigned int numPanels = 10;// room for growth
LightPanel panelArr[ numPanels ];
unsigned int numPanels_inUse = 8;
void setupPanels();

//LightPlayer2 ltPlay;
//patternData PD_Arr[20];

//WavePlayer2 wvPlay;

// 10 DataPlayers
const unsigned int numDPmax = 10;
DataPlayer DP[ numDPmax ];
unsigned int numDP_inUse = 6;
// resources for the DataPlayers
const unsigned int imgDataSz = 5000;
uint8_t imageData[ imgDataSz ];
const unsigned int colorArrSz = 200;
Light colorArr[ colorArrSz ];

// another DataPlayer for floatAni
DataPlayer imgPlay_move;
aniFloat moveAniHoriz;// animate position of above
int colLt = 2, colRt = 22;// for 32 column grid
bool moveRt = true;
// swap digits and spider vertically
aniFloat moveAniVert;
bool moveDn = true;// digits down and spider up

// a callback for pushButtA
void PBonOpen()
{
  // start the animated move
  if( potA.steadyVal < 100 )// spider horizontal
  {
    if( !moveAniVert.Running() )
      moveAniHoriz.setup( colRt - colLt, 16.0f, true );
  }
  else// digits and spider trade rows
  {
    if( !moveAniHoriz.Running() )
      moveAniVert.setup( 8, 8.0f, true );
  }
}

void setup()
{  
  // a tiny oled screen
  tinyOledA.setupDisplay();
  tinyOledA.setTextColor( COLOR_WHITE );
  tinyOledA.clear();
  tinyOledA.printAt( 0, 0, "Hi there.\nBad Arduino!", 1 );// running on "bad" Arduino
  tinyOledA.show();
  // a push button
  pushButtA.init( 2, 0.1f );// trigger spider to move across the grid
  pushButtA.onOpen = PBonOpen;// on contact opening

  // someDigits
  colorArr[0].setRGB(0,0,200);// background color
  colorArr[1].setRGB(220,0,100);// foreground color

  numDP_inUse = 6;
  for( unsigned int k = 0; k < numDP_inUse; ++k )
  {
    init_digitsAniData( LightArr, DP[k], imageData, colorArr );// same data and colors for all digits
    DP[k].setGridBounds( 1, 4*( numDP_inUse - k ) - 3, gridRows, gridCols );
    DP[k].stepIter = 0;
  }

  init_spiderAniData( LightArr, imgPlay_move, imageData + 19, colorArr + 2 );
  imgPlay_move.setGridBounds( 8, colLt, gridRows, gridCols );
  
  setupPanels();

  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(64);

  
}

void setupPanels()
{
  numPanels_inUse = 2;
  for( unsigned int n = 0; n < numPanels_inUse; ++n )
  {
    //              if 8 panels                  SrcRows, SrcCols, SrcRow0, SrcCol0, TgtRows, TgtCols
  //  panelArr[n].init( LightArr, gridRows, gridCols, 16,     32,      0,       0,       8,       8 );
  //  panelArr[n].rotIdx = -1;

    //              if 2 panels                    SrcRows, SrcCols, SrcRow0, SrcCol0, TgtRows, TgtCols
    panelArr[n].init( LightArr, gridRows, gridCols,   8,      32,      8*n,       0,      32,      8 );
    panelArr[n].rotIdx = 0;

    // either
    panelArr[n].pTgt0 = leds + n*panelArr[n].tgtRows*panelArr[n].tgtCols;
    panelArr[n].type = 2;
    
  }

}

void updateAniMotion( float dt );

void loop() 
{
  const float dt = 0.02f;

  // clear LightArr
  for( unsigned int n = 0; n < numLEDs; ++n ) LightArr[n].setRGB(0,0,200);

  // floatAni
  updateAniMotion(dt);

  // draw to LightArr
  int lpCnt = potA.update();// get stable input
  // display as digits
  for( unsigned int k = 0; k < numDP_inUse; ++k )
  {
    DP[k].showImage( lpCnt%10 );
    DP[k].update();
    if( lpCnt > 0 ) lpCnt /= 10;
  }

  imgPlay_move.update();
  
  // input
  pushButtA.update(dt);

  FastLED.clear();
  // draw to leds array
  for( unsigned int n = 0; n < numPanels_inUse; ++n )
    panelArr[n].update();
  
	FastLED.show();
	delay(8.f);
}

void updateAniMotion( float dt )
{
  // floatAni
  if( moveAniHoriz.updateTime(dt) )
  {
    if( moveRt )
      imgPlay_move.col0 = colLt + (int)moveAniHoriz.currDist();
    else
      imgPlay_move.col0 = colRt - (int)moveAniHoriz.currDist();

    // last frame
    if( !moveAniHoriz.Running() )
    {
      if( moveRt ) imgPlay_move.col0 = colRt;// final position
      else imgPlay_move.col0 = colLt;
       moveRt = !moveRt;// for next time
    }
  }

  // digits and spider swap rows
  if( moveAniVert.updateTime(dt) )
  {
    if( moveDn )// the digits
    {
      imgPlay_move.row0 = 8 - (int)moveAniVert.currDist();// going up
      for( unsigned int n = 0; n < numDP_inUse; ++n )// going down      
        DP[n].row0 = 1 + (int)moveAniVert.currDist();
      
    }
    else
    {
      imgPlay_move.row0 = (int)moveAniVert.currDist();// going down
      for( unsigned int n = 0; n < numDP_inUse; ++n )// going down      
        DP[n].row0 = 9 - (int)moveAniVert.currDist();
    }

    // last frame
    if( !moveAniVert.Running() )
    {
      if( moveDn )
      {
         imgPlay_move.row0 = 0;// spider up
         for( unsigned int n = 0; n < numDP_inUse; ++n )// going down      
          DP[n].row0 = 9;// digits down
      }
      else 
      {
        imgPlay_move.row0 = 8;// spider down
        for( unsigned int n = 0; n < numDP_inUse; ++n )// going down      
          DP[n].row0 = 1;// digits up
      }

       moveDn = !moveDn;// for next time
    }
  }
  
}