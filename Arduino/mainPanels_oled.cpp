#include <Arduino.h>
#include <FastLED.h>

#include "LightPanel.h"
#include "LightPlayer2.h"
#include "WavePlayer2.h"
#include "DataPlayer_initFuncs.h"

#include "SSD_1306Component.h"// a tiny oled panel
// a tiny oled screen
SSD1306_Display tinyOledA;

const unsigned int gridRows = 16, gridCols = 32;
const unsigned int numLEDs = gridRows*gridCols;

Light LightArr[numLEDs];// source array
CRGB leds[numLEDs];// target array

const unsigned int numPanels = 10;// room for growth
LightPanel panelArr[ numPanels ];

LightPlayer2 ltPlay;
patternData PD_Arr[20];

WavePlayer2 wvPlay;

DataPlayer DP;
const unsigned int imgDataSz = 5000;
uint8_t imageData[ imgDataSz ];
const unsigned int colorArrSz = 200;
Light colorArr[ colorArrSz ];

void setup()
{  
  // a tiny oled screen
  tinyOledA.setupDisplay();
  tinyOledA.setTextColor( COLOR_WHITE );

  PD_Arr[0].funcIndex = 1;// scroll right
  PD_Arr[0].stepPause = 3;
  PD_Arr[0].param = 3;
  PD_Arr[1].funcIndex = 10;// col to right
  PD_Arr[1].stepPause = 10;
  PD_Arr[1].param = 1;
  PD_Arr[2].funcIndex = 12;// row to bottom
  PD_Arr[2].stepPause = 4;
  PD_Arr[2].param = 1;
  ltPlay.offLt.setRGB( 0, 0, 240 );
  ltPlay.onLt.setRGB( 220, 0, 0 );
  ltPlay.init( LightArr[0], gridRows, gridCols, PD_Arr[0], 3 );
  ltPlay.setTargetRect( 12, 14, 2, 18 );  
  ltPlay.drawOffLt = false;

  wvPlay.init( LightArr[0], gridRows, gridCols, Light(0,200,0), Light(0,0,200) );
  wvPlay.setWaveData( 1.0f, 60.0f, 90.0f, 60.0f, 90.0f );
//  wvPlay.setTargetRect( 16, 8, 16, 0 );

  init_spiderAniData( LightArr, DP, imageData, colorArr );
  DP.setGridBounds( 2, 10, gridRows, gridCols );

  for( unsigned int n = 0; n < 8; ++n )
  {
    panelArr[n].init_Src( LightArr, gridRows, gridCols );
    panelArr[n].set_SrcArea( 8, 8, 8*(1 - n/4), 8*(n%4) );
    panelArr[n].pTgt0 = leds + 64*n;
    panelArr[n].type = 2;
    panelArr[n].rotIdx = 1;
  }

  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(64);
}

bool firstCall = true;

void loop() 
{
  // draw to oled display
  if( firstCall )
  {
    firstCall = false;    
    String msg = "fIdx = ";
    msg += ltPlay.pattData[ ltPlay.patternIter ].funcIndex;
    msg += "\nparam = ";
    msg += ltPlay.pattData[ ltPlay.patternIter ].param;
    msg += "\npatIter = ";
    msg += ltPlay.patternIter;
    msg += "\nPL0 = ";
    msg += ltPlay.getPattLength();
    
    tinyOledA.clear();
    tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
    tinyOledA.show();
  }

  // draw to LightArr
  wvPlay.update( 0.02f );  
  DP.update();
  ltPlay.update();
  

  FastLED.clear();
  // draw to leds array
  for( unsigned int n = 0; n < 8; ++n )
    panelArr[n].update();
  
	FastLED.show();
	delay(8.f);
}