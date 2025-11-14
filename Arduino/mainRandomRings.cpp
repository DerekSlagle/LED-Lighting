#include <Arduino.h>
#include <FastLED.h>
//#include <SD.h>// a micro SD drive

//#include "FileParser.h"
//#include "SSD_1306Component.h"// a tiny oled panel

#include "RingPlayer.h"
//#include "WavePlayer2.h"

#include "LightPanel.h"
//#include "switchSPST.h"
//#include "slidePot.h"

// the full target rectangle
const int gridRowsFull = 32, gridColsFull = 32;
const unsigned int numLEDs = static_cast<unsigned int>( gridRowsFull*gridColsFull );

const unsigned int numPanels = 4;
LightPanel panel[ numPanels ];

Light LightArr[numLEDs];// players write to this array
CRGB leds[numLEDs];// panels map above to this array
Light gridColor;

// time deltas displayed on oledPage 0
float dtLoop = 0.02f;// entire loop
float dtRings = 0.02f;// across updates
float dtWaves = 0.02f;// across updates

// RingPlayer
const unsigned int numRP = 30;
int numRPplaying = 0;
RingPlayer ringPlay_Arr[numRP];
//RingPlayer Radiator;
//String RadiatorFileName;
bool init_RingPlayer( RingPlayer& RP, const char* fileName );
float tStart = 0.14f;// Start() 1 per second
float tElapStart = 0.0f;
int idxStartNext = 0;
//float dtRing = 0.02f;// replaces local dt for loop time
void updateRingPlayers( float dt );
//bool doShowRings = true;

// called within setup() and loop()
void init_1();
void loop_1();

void setup()
{
  srand( micros() );

  gridColor.setRGB( 0, 10, 20 );
  init_1();    

  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(32);
}

void loop() 
{
  FastLED.clear();
  loop_1();
	FastLED.show();
  

//	delay(2.f);
}

void init_panels();// from file on SD card

void init_1()
{  
  // RingPlayer
//  init_RingPlayer( Radiator, RadiatorFileName.c_str() );
  // all members assigned procedurally. Not from file
  for( unsigned int n = 0; n < numRP; ++n )
    ringPlay_Arr[n].initToGrid( &( LightArr[0] ), gridRowsFull, gridColsFull );

  // the display panels
  init_panels();
}

float fadeRratio = 1.6f;
float fadeWratio = 1.6f;
float spawnTime = 0.5f;// average rate of 2 per second
float speedFactor = 1.0f;// modulates randomly assigned value

void loop_1()
{
  static unsigned long lastTime = micros();
  unsigned long currTime = micros();
  dtLoop = ( currTime - lastTime )*0.000001f;
  lastTime = currTime;
  

  // clear entire array
  for( unsigned int n = 0; n < numLEDs; ++n )  
    LightArr[n] = gridColor;

  // time across the RingPlayer updates
  updateRingPlayers( dtLoop );
  unsigned long endTime = micros();// end
  dtRings = 0.001f*( endTime - currTime );

  // panel orientation test = pass!
 // LightArr[0].setRGB(255,255,255);
 // LightArr[16].setRGB(255,255,255);
 // LightArr[24].setRGB(255,255,255);
 // LightArr[512].setRGB(255,255,255);

  // LightPanels map from LightArr to leds
  for( unsigned int p = 0; p < numPanels; ++p )
    panel[p].update();  
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

void updateRingPlayers( float dt )
{
  numRPplaying = 0;
  for( unsigned int k = 0; k < numRP; ++k )
  {
    ringPlay_Arr[k].update( dt );
    if( !ringPlay_Arr[k].onePulse && ringPlay_Arr[k].isRadiating )
    {
      float R = ringPlay_Arr[k].ringSpeed*ringPlay_Arr[k].tElap;
      if ( R > 3.0f*ringPlay_Arr[k].ringWidth )
        ringPlay_Arr[k].StopWave();
    }

     if( ringPlay_Arr[k].isPlaying ) ++numRPplaying;
  }  

  // Start another
  tElapStart += dt;
  if( tElapStart >= tStart )
  {
    tElapStart = 0.0f;
    RingPlayer& RP = ringPlay_Arr[ idxStartNext ];
  //  int idxRD = rand()%numRingDataInUse;
  //  RP.setup( ringData[idxRD] );
    int rC = rand()%48 - 8, cC = rand()%48 - 8;
    RP.setRingCenter( rC, cC );
    int range = 160;// base = 80;
    RP.hiLt.r = 80 + rand()%range;
    RP.hiLt.g = 80 + rand()%range;
    RP.hiLt.b = 80 + rand()%range;
    RP.loLt.r = 16 + rand()%range;
    RP.loLt.g = 16 + rand()%range;
    RP.loLt.b = 16 + rand()%range;
    // ring props
    if( (rand()%30)/10 == 1 ) RP.onePulse = false;// 1 in 3 chance to radiate
    else RP.onePulse = true;

    float ringWidth = 2.0f + 0.06f*(float)( rand()%101 );// 1 to 8
    if( !RP.onePulse ) ringWidth *= 0.6f;
    float fadeR = fadeRratio*ringWidth;
    float fadeW = fadeWratio*ringWidth;
    float time = 0.5f + 0.02f*(float)( rand()%101 );// 0.5 to 2.0
    float Speed = speedFactor*( fadeR + fadeW )/time;
    RP.setRingProps( Speed, ringWidth, fadeR, fadeW );
    RP.Amp = 0.3 + 0.007f*(float)( rand()%101 );
    
    RP.Start();
    idxStartNext = ( 1 + idxStartNext )%numRP;
    tStart = 2.0f*spawnTime*(float)( rand()%1000 );
    tStart /= 1000.0f;
  }
}