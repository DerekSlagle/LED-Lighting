#include <Arduino.h>
#include <FastLED.h>
#include <SD.h>// a micro SD drive

#include "BytePool2.h"
#include "LightPool.h"
#include "FileParser.h"
#include "SSD_1306Component.h"// a tiny oled panel

#include "LightPlayer2.h"
#include "DataPlayer.h"
#include "WavePlayer2.h"
#include "RingPlayer.h"

//#include "DataPlayer_initFuncs.h"
#include "LightPanel.h"
#include "switchSPST.h"
#include "slidePot.h"
// 6 axis accel
//#include "I2Cdev.h"
//#include "MPU6050_6Axis_MotionApps20.h"
//MPU6050 mpu;
//void setupMPU();
//void loopMPU();
// variables for the MPU declared before definitions of above

// the full target rectangle
const int gridRowsFull = 32, gridColsFull = 32;// landscape
const unsigned int numLEDs = static_cast<unsigned int>( gridRowsFull*gridColsFull );

// 2 32x8 strips + 2 16x16 panels
const unsigned int numPanels = 4;// for 1,024 Lights
LightPanel panel[ numPanels ];

Light LightArr[numLEDs];// players write to this array
CRGB leds[numLEDs];// panels map above to this array

patternData PD_Arr[50];
const unsigned int numLP = 5;
unsigned int numLPinUse = 0;// from file
LightPlayer2 LP_Arr[numLP];

// RingPlayer
const unsigned int numRP = 30;
unsigned int numRPfiles = 0;// from file
int numRPplaying = 0;
RingPlayer ringPlay_Arr[numRP];
RingPlayer Radiator;
bool init_RingPlayer( RingPlayer& RP, const char* fileName );
float tStart = 0.14f;// Start() 1 per second
float tElapStart = 0.0f;
int idxStartNext = 0;
// an array of configurations to iterate through
const unsigned int numRingData = 50;
unsigned int numRingDataInUse = 0;// from file
RPdata ringData[numRingData];
bool getRingData();// from file "BulkRingData.txt"

// resourcees for the DataPlayers
const unsigned int numUsers = 10;
// byte data
const unsigned int imageDataSz = 5000;// 5kB 
uint8_t imageData[imageDataSz];
uint8_t** ppBlockBP[numUsers];
unsigned int* pBlockSzBP[numUsers];
BytePool2 bytePool( imageData, imageDataSz, ppBlockBP, pBlockSzBP, numUsers );

// LightPool
const unsigned int imgColorSz = 200;
Light imgColor[imgColorSz];// the array storage
Light** ppBlockLP[numUsers];
unsigned int* pBlockSzLP[numUsers];
LightPool colorPool( imgColor, imgColorSz, ppBlockLP, pBlockSzLP, numUsers );// cannot initialize
//colorPool.init( imgColor, imgColorSz, ppBlockLP, pBlockSzLP, numUsers );// cannot initialize

const unsigned int numDP = 5;
DataPlayer DP_Arr[numDP];// 8x8 animations
unsigned int numDPinUse = 0;// init_SD()

// WavePlayer2. Rotate which is playing
const unsigned int numWP = 20;
WavePlayer2 wvPlay[numWP];
unsigned int numWPinUse = 0;// init_SD()
// seies coefficients for each player
const unsigned int maxCoeffs = 5;// up to 4 harmonics each way
float wvC_rt[ numWP ][ maxCoeffs ];// 
float wvC_lt[ numWP ][ maxCoeffs ];// up to 4 harmonics for left going wave
unsigned int currWP = 0;// index to currently playing
// player timing
float tPeriodWP = 5.0f, tElapWP = 0.0f;// play duration and timer
float tTransWP = 1.0f;// time for transition to next player
// a buffer for the next wave player during transition
Light BufferArr[numLEDs];// wvPlay[ nextWP ] writes to this array if doBlendBuffers

// 2nd method. blending member values. Applies if !doBlendBuffers
WavePlayer2 wvPlayTrans;// player writing during transition.
float wvC_rtTrans[5], wvC_ltTrans[5];
bool doBlendBuffers = true;// use 1st method

// called within setup() and loop()
void init_1();
void loop_1();

// for SD card
bool SDgood = false;
bool SDfileOpened = false;
bool SDdataGot = false;
//uint8_t digitByte[19];
bool init_SD();//

// file names for SD
const unsigned int SDmaxFiles = 50;// phasing out
String SDfileNames[ SDmaxFiles ];// read to by init_SD()
unsigned int SDnumFiles = 0;// # in use

// a tiny oled screen
SSD1306_Display tinyOledA;
// written to in updateIVsensors(), updateWP(), PBopen() and updatePlayPeriods()
const int numWriters = 5;
String oledMsg[ numWriters ];// 0, 1, 2, 3 as above order
bool showOled = false;// write all Strings to display
// time passage is faked. value set in updatePlayPeriods()
float dtWave = 0.02f;// replaces local dt
float dtRing = 0.02f;// replaces local dt
// a menu for float values on tinyOledA
#include "Menu_setFloats.h"
Menu_setFloats floatMenu;
void initFloatMenu();

// a pushbutton to toggle WavePlayer blending mode
switchSPST pushButtA;
void PBopen()
{  
  if( !Radiator.onePulse );
  {
    if( Radiator.isPlaying ) Radiator.StopWave();
    else  Radiator.Start();
    oledMsg[2] = Radiator.isRadiating ? "Start" : "STOP";
    showOled = true;
  }  
}

// for the oled display. Using pot to vary 4 floats
//void initMenu();
//void updateMenu( float dt );

void setup()
{
  pushButtA.init( 2, 0.1f, false );// normally open so iClosed = false. This will fake an open event.
  pushButtA.onClose = PBopen;
  pushButtA.onOpen = nullptr;

  srand( micros() );
  init_SD();
  getRingData();
//  initMenu();// old
  initFloatMenu();// new for
  init_1();    

  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(32);

  tinyOledA.setupDisplay();
  tinyOledA.setTextColor( COLOR_WHITE );
  PBopen();// to present initial message
//  updateMenu( 0.03f );// to present initial message  
  showOled = true;  
}

void loop() 
{
  FastLED.clear();
  loop_1();
	FastLED.show();
  

//	delay(2.f);
}

void init_panels( const char* fileName );// from file on SD card
bool init_DataPlayer( DataPlayer& DP, const char* fileName );
bool init_LightPlayer2( LightPlayer2& LP, const char* fileName );
bool init_WavePlayer( WavePlayer2& WP, const char* fileName );// due to assoc with

void init_1()
{  
  unsigned int baseIdx = 0;
  // DataPlayers
  for( unsigned int n = 0; n < numDPinUse; ++n )
    init_DataPlayer( DP_Arr[n], SDfileNames[ n + baseIdx ].c_str() );

  baseIdx += numDPinUse;
  // LightPlayer2
  for( unsigned int n = 0; n < numLPinUse; ++n )
    init_LightPlayer2( LP_Arr[n], SDfileNames[ n + baseIdx ].c_str() );

  baseIdx += numLPinUse;
  // RingPlayer
  init_RingPlayer( Radiator, SDfileNames[ baseIdx ].c_str() );
  ++baseIdx;
  for( unsigned int n = 0; n < numRPfiles; ++n )
    init_RingPlayer( ringPlay_Arr[n], SDfileNames[ n + baseIdx ].c_str() );

    /*

  baseIdx += numRPfiles;
  // WavePlayer2
  tElapWP = 0.0f;// player changes eeach tPeriodWP
  for( unsigned int n = 0; n < numWPinUse; ++n )
  {
    for( unsigned int j = 0; j < 5; ++j )
    {  wvC_lt[n][j] = wvC_rt[n][j] = 0.0f; }// initialize float values
    // assign storage to player
    wvPlay[n].C_Lt  = wvC_lt[n];
    wvPlay[n].C_Rt  = wvC_rt[n];
    init_WavePlayer( wvPlay[n], SDfileNames[ n + baseIdx ].c_str() );
  }
  // the Trans player
  for( unsigned int j = 0; j < maxCoeffs; ++j )
    {  wvC_ltTrans[j] = wvC_rtTrans[j] = 0.0f; }
    wvPlayTrans.nTermsLt = maxCoeffs;
    wvPlayTrans.nTermsRt = maxCoeffs;
   wvPlayTrans.C_Lt = wvC_ltTrans;
   wvPlayTrans.C_Rt = wvC_rtTrans;
   wvPlayTrans.init( LightArr[0], gridRowsFull, gridColsFull, Light(0,0,0), Light(0,0,0) );
   wvPlayTrans.setWaveData( 1.0f, 100.0f, 100.0f, 100.0f, 100.0f );
   */

  // the display panels
  init_panels("/panelsConfig.txt");
}

// declarations for further in
void updateWP( float dt );// all in 1
// 2 blending methods
void blendBuffers( float U, float dt );
void blendWP( float U, float dt );
// for voltage sensor on pin A1
void updateIVsensors( float dt );// writes to oledMsg[0]
// to support time readout
float dtRPup = 0.0f;
// wave properties
float fadeRratio = 1.6f;
float fadeWratio = 1.6f;
float spawnTime = 0.5f;// average rate of 2 per second
float speedFactor = 1.0f;// modulates randomly assigned value

void loop_1()
{

//  float dtReal = 0.03125f;// 1/32
  static unsigned long lastTime = micros();
  unsigned long currTime = micros();
  dtRing = ( currTime - lastTime )*0.000001f;
  lastTime = currTime;

 // tinyOledA.clear();
  updateIVsensors( dtRing );// writes to oledMsg[0]
  // button. use constant dt
  pushButtA.update( dtRing );// writes to oledMsg[2]
  floatMenu.update( dtRing );
//  updateMenu( dtRing );// writes to oledMsg[3]

  // clear entire array
  for( unsigned int n = 0; n < numLEDs; ++n )  
    LightArr[n] = Light(0,10,20);

  // players update to LightArr
  // time this update
  currTime = micros();  

  // temp suspended
//  if( tElapWP >= tPeriodWP )// transition in progress
//    for( unsigned int n = 0; n < numLEDs; ++n )  
//      BufferArr[n] = Light(0,0,240);  

//  updateWP( dtRing );// writes to oledMsg[1]

  // other players
//  for( unsigned int k = 0; k < numDPinUse; ++k )
//    if( DP_Arr[k].BA.pByte ) DP_Arr[k].update();  
  
//  for( unsigned int k = 0; k < numRPinUse; ++k )
  numRPplaying = 0;
  for( unsigned int k = 0; k < numRP; ++k )
  {
    ringPlay_Arr[k].update( dtRing );
    if( !ringPlay_Arr[k].onePulse && ringPlay_Arr[k].isRadiating )
    {
      float R = ringPlay_Arr[k].ringSpeed*ringPlay_Arr[k].tElap;
      if ( R > 3.0f*ringPlay_Arr[k].ringWidth )
        ringPlay_Arr[k].StopWave();
    }

     if( ringPlay_Arr[k].isPlaying ) ++numRPplaying;
  }  

  Radiator.update( dtRing );
  if( Radiator.isPlaying ) ++numRPplaying;
  // end timing of dtRPup
  unsigned long postTime = micros();
  dtRPup = ( postTime - currTime )*0.001f;// in ms

  // Start another
  tElapStart += dtRing;
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

//  for( unsigned int k = 0; k < numLPinUse; ++k )
//    if( LP_Arr[k].pattData )
//    {
//       LP_Arr[k].update();
//    }

  // LightPanels map from LightArr to leds
  for( unsigned int p = 0; p < numPanels; ++p )
    panel[p].update();

  // output to oled
  if( showOled || floatMenu.doShow )
  {
    showOled = false;
    floatMenu.doShow = false;

    String wholeMsg = oledMsg[0];
    for( int n = 1; n < numWriters; ++n )
      wholeMsg += '\n' + oledMsg[n];

    tinyOledA.clear();
    tinyOledA.printAt( 0, 0, wholeMsg.c_str(), 1 );
    tinyOledA.show();
  }
}

bool isBlending = false;

void updateWP( float dt )
{
  unsigned int nextWP = ( 1 + currWP )%numWPinUse;// roll index
  tElapWP += dt;

  if( tElapWP < tPeriodWP )
  {
    wvPlay[ currWP ].update(dt);
  }
  else if( tElapWP < tPeriodWP + tTransWP )// blend
  {
    float U = ( tElapWP - tPeriodWP )/tTransWP;
    if( doBlendBuffers ) blendBuffers( U, dt );// both will update
    else blendWP( U, dt );

    // output to oled
    if( !isBlending )// transition begins
    {
      isBlending = true;
      oledMsg[1] = " trans begin";
      showOled = true;
    }
  }
  else// transition has ended
  {
    tElapWP = 0.0f;// reset cycle
    currWP = nextWP;
    wvPlay[ currWP ].pLt0 = LightArr;
    wvPlay[ currWP ].update(dt);

    if( !doBlendBuffers )
    {
      wvPlay[currWP].tElapLt = wvPlay[currWP].tElapRt = tTransWP;
      wvPlayTrans.tElapLt = wvPlayTrans.tElapRt = 0.0;
    }

    // oled report
    oledMsg[1] = " trans over";
    isBlending = false;
    showOled = true;
  }
}

// current try at blending from 2 buffers during transition
void blendBuffers( float U, float dt )
{
  unsigned int nextWP = ( 1 + currWP )%numWPinUse;
  
  // update both
  wvPlay[ currWP ].update(dt);// writing to LightArr
  wvPlay[ nextWP ].pLt0 = BufferArr;
  wvPlay[ nextWP ].update(dt);// writing to BufferArr

  // blend buffers
  float Q = 1.0f - U;
  for( unsigned int n = 0; n < numLEDs; ++n )
  {
    //            current                    next
    float rd = Q*(float)LightArr[n].r + U*(float)BufferArr[n].r;
    float gn = Q*(float)LightArr[n].g + U*(float)BufferArr[n].g;
    float bu = Q*(float)LightArr[n].b + U*(float)BufferArr[n].b;
    LightArr[n].r = rd;
    LightArr[n].g = gn;
    LightArr[n].b = bu;
    // switch back?
   // wvPlay[ nextWP ].pLt0 = LightArr;
  }
}

// for a future try at blending the players member values
void blendWP( float U, float dt )
{
  unsigned int nextWP = ( 1 + currWP )%numWPinUse;
  float Q = 1.0f - U;
  wvPlayTrans.AmpRt = Q*wvPlay[ currWP ].AmpRt + U*wvPlay[ nextWP ].AmpRt;
  wvPlayTrans.AmpLt = Q*wvPlay[ currWP ].AmpLt + U*wvPlay[ nextWP ].AmpLt;
  // wvLen
  wvPlayTrans.wvLenRt = Q*wvPlay[ currWP ].wvLenRt + U*wvPlay[ nextWP ].wvLenRt;
  wvPlayTrans.wvLenLt = Q*wvPlay[ currWP ].wvLenLt + U*wvPlay[ nextWP ].wvLenLt;
  // wvSpd
  wvPlayTrans.wvSpdRt = Q*wvPlay[ currWP ].wvSpdRt + U*wvPlay[ nextWP ].wvSpdRt;
  wvPlayTrans.wvSpdLt = Q*wvPlay[ currWP ].wvSpdLt + U*wvPlay[ nextWP ].wvSpdLt;
  // period ? tElap?
   wvPlayTrans.periodRt =  wvPlayTrans.wvLenRt/wvPlayTrans.wvSpdRt;
   wvPlayTrans.periodLt =  wvPlayTrans.wvLenLt/wvPlayTrans.wvSpdLt;
   // the 2 colors
   // Lo
   wvPlayTrans.frLo = Q*wvPlay[ currWP ].frLo + U*wvPlay[ nextWP ].frLo;
   wvPlayTrans.fgLo = Q*wvPlay[ currWP ].fgLo + U*wvPlay[ nextWP ].fgLo;
   wvPlayTrans.fbLo = Q*wvPlay[ currWP ].fbLo + U*wvPlay[ nextWP ].fbLo;
   // Hi
   wvPlayTrans.frHi = Q*wvPlay[ currWP ].frHi + U*wvPlay[ nextWP ].frHi;
   wvPlayTrans.fgHi = Q*wvPlay[ currWP ].fgHi + U*wvPlay[ nextWP ].fgHi;
   wvPlayTrans.fbHi = Q*wvPlay[ currWP ].fbHi + U*wvPlay[ nextWP ].fbHi;

  // coeffs
  // zero all
  for( unsigned int n = 0; n < maxCoeffs; ++n )
    wvPlayTrans.C_Rt[n] = wvPlayTrans.C_Lt[n] = 0.0f;

  // build up
  // right
  if( wvPlay[ currWP ].C_Rt )
  {
    for( unsigned int n = 0; n < maxCoeffs; ++n )
      wvPlayTrans.C_Rt[n] = Q*wvPlay[ currWP ].C_Rt[n];
  }
  if( wvPlay[ nextWP ].C_Rt )
  {
    for( unsigned int n = 0; n < maxCoeffs; ++n )
      wvPlayTrans.C_Rt[n] += U*wvPlay[ nextWP ].C_Rt[n];
  }
  // left
  if( wvPlay[ currWP ].C_Lt )
  {
    for( unsigned int n = 0; n < maxCoeffs; ++n )
      wvPlayTrans.C_Lt[n] = Q*wvPlay[ currWP ].C_Lt[n];
  }
  if( wvPlay[ nextWP ].C_Lt )
  {
    for( unsigned int n = 0; n < maxCoeffs; ++n )
      wvPlayTrans.C_Lt[n] += U*wvPlay[ nextWP ].C_Lt[n];
  }

  // ???
  // tElap
  // wvPlay[ currWP ].update_tElapOnly(dt);
  // wvPlay[ nextWP ].update_tElapOnly(dt);
  wvPlayTrans.tElapRt = Q*wvPlay[ currWP ].tElapRt + U*wvPlay[ nextWP ].tElapRt;
  wvPlayTrans.tElapLt = Q*wvPlay[ currWP ].tElapLt + U*wvPlay[ nextWP ].tElapLt;
  wvPlayTrans.update( dt );// do not increment tElap
}

// from file on SD card
void init_panels( const char* fileName )
{
  // each panel. Common member values
  for( unsigned int p = 0; p < numPanels; ++p )// all same
    panel[p]. init_Src( LightArr, gridRowsFull, gridColsFull );

  FileParser FP( fileName );
  // assign each panel in address order = order of connection in field
  Light* pTgt = leds;// iterator
  int Rows = 0, Cols = 0, Row0 = 0, Col0 = 0;// used in for loop
  int numInstalled = 0;
  FP >> numInstalled;
  for( unsigned int p = 0; p < numInstalled; ++p )
  {
    panel[p].pTgt0 = pTgt;    
    FP >> Rows >> Cols >> Row0 >> Col0;
    panel[p].set_SrcArea( Rows, Cols, Row0, Col0 );
    FP >> panel[p].rotIdx;
    FP >> panel[p].type;
    pTgt += panel[p].rows*panel[p].cols;// next panel start address
  }
}

bool init_DataPlayer( DataPlayer& DP, const char* fileName )
{
//  uint8_t* pData = DP.BA.pByte;

  FileParser FP( fileName );
  int Rows = 0, Cols = 0, Row0 = 0, Col0 = 0;
  FP >> Rows >> Cols >> Row0 >> Col0;
  int StepPause = 10, DrawOff = 1;
  FP >> StepPause >> DrawOff;
  int bitSize = 0;
  FP >> bitSize;

  int NumColors = 2;
  FP >> NumColors;
  if( !colorPool.Alloc( DP.pLight, DP.numColors, NumColors ) ) return false;
  for( int k = 0; k < NumColors; ++k )
  {
    int rd, gn, bu;
    FP >> rd >> gn >> bu;
    DP.pLight[k].setRGB( rd, gn, bu );
  }

  int NumBytes = 0;
  FP >> NumBytes;
  if( !bytePool.Alloc( DP.BA.pByte, DP.BA.capBytes, NumBytes ) ) return false;

  uint8_t* pData = DP.BA.pByte;

  for( unsigned int k = 0; k < NumBytes; ++k )
  {
    int inVal;
    FP >> inVal;
    pData[k] = inVal;
  }

  DP.init( LightArr[0], Rows, Cols, pData[0], NumBytes, NumColors );
  DP.setGridBounds( Row0, Col0, gridRowsFull, gridColsFull );
  DP.BA.sizeBits = bitSize;
  DP.drawOff = ( DrawOff == 1 );
  DP.stepPause = StepPause;
  return true;
}

bool init_RingPlayer( RingPlayer& RP, const char* fileName )
{
    FileParser FP( fileName );

    int rd, gn, bu;
    FP >> rd >> gn >> bu;
    RP.hiLt.setRGB(rd,gn,bu);
    FP >> rd >> gn >> bu;
    RP.loLt.setRGB(rd,gn,bu);
    RP.initToGrid( &( LightArr[0] ), gridRowsFull, gridColsFull );
    float Speed = 0.0f, RingWidth = 0.0f, FadeRadius = 0.0f, FadeWidth = 0.0f;
    FP >> Speed >> RingWidth >> FadeRadius >> FadeWidth;
    RP.setRingProps( Speed, RingWidth, FadeRadius, FadeWidth );
    float rowC = 0.0f, colC = 0.0f;
    FP >> rowC >> colC >> RP.Amp;
    RP.setRingCenter( rowC, colC );
    FP >> rd;
    
    RP.onePulse = ( rd == 1 );
  //  RP.Start();

    return true;
}

int PD_iter = 0;// offset to 1st element in PD_Arr
bool init_LightPlayer2( LightPlayer2& LP, const char* fileName )
{
  FileParser FP( fileName );
  int Rows = 0, Cols = 0, Row0 = 0, Col0 = 0;
  FP >> Rows >> Cols >> Row0 >> Col0;
  int DrawOff = 1;
  FP >> DrawOff;
  // the 2 colors
  int rd = 0, gn = 0, bu = 0;
  FP >> rd >> gn >> bu;
  LP.onLt.setRGB( rd, gn, bu );
  FP >> rd >> gn >> bu;
  LP.offLt.setRGB( rd, gn, bu );
  // the patterns
  int numPatterns = 0;
  FP >> numPatterns;
  for( int k = 0; k < numPatterns; ++k )
  {
    int fIdx = 0, sp = 0, Param = 0;
    FP >> fIdx >> sp >> Param;
    PD_Arr[ k + PD_iter ].init( fIdx, sp, Param );
    
  }
  
  LP.init( LightArr[0], gridRowsFull, gridColsFull, PD_Arr[ PD_iter ], numPatterns );
  PD_iter += numPatterns;// for next iteration
  LP.setTargetRect( Rows, Cols, Row0, Col0 );
  
  LP.drawOffLt = ( DrawOff == 1 );
  return true;
}

bool init_WavePlayer( WavePlayer2& WP, const char* fileName )
{
  FileParser FP( fileName );
  int Rows = 0, Cols = 0, Row0 = 0, Col0 = 0;
  FP >> Rows >> Cols >> Row0 >> Col0;
  // the 2 colors
  Light HiLt( 0,160,80 ), LoLt( 0,20,240 );
  int rd = 0, gn = 0, bu = 0;
  FP >> rd >> gn >> bu;
  HiLt.setRGB( rd, gn, bu );
  FP >> rd >> gn >> bu;
  LoLt.setRGB( rd, gn, bu );
  
  WP.init( LightArr[0], gridRowsFull, gridColsFull, HiLt, LoLt );
  // wave data
   float AmpLt, AmpRt, wvLen_lt, wvSpd_lt, wvLen_rt, wvSpd_rt;
   FP >> AmpLt >> wvLen_lt >> wvSpd_lt >> AmpRt >> wvLen_rt >> wvSpd_rt;
  WP.setWaveData( AmpRt, wvLen_lt, wvSpd_lt, wvLen_rt, wvSpd_rt );// all right
  WP.AmpLt = AmpLt;
  WP.setTargetRect( Rows, Cols, Row0, Col0 );
  // series coefficients
  int numCoeff_rt = 0;
  // right
  FP >> numCoeff_rt;
  if( numCoeff_rt > 0 && WP.C_Rt )// storage already assigned
  {
    WP.nTermsRt = numCoeff_rt;
    for( int c = 0; c < numCoeff_rt; ++c )  
      FP >> WP.C_Rt[c];
  }
  else
  {
      WP.nTermsRt = 0;
      WP.C_Rt = nullptr;// issue if pool allocated?
  }
  // left
  int numCoeff_lt = 0;
  FP >> numCoeff_lt;
  if( numCoeff_lt > 0 && WP.C_Lt )// storage already assigned
  {
    WP.nTermsLt = numCoeff_lt;
    for( int c = 0; c < numCoeff_lt; ++c )  
      FP >> WP.C_Lt[c];
  }
  else
  {
      WP.nTermsLt = 0;
      WP.C_Lt = nullptr;// issue if pool allocated?
  }
  
  return true;
}

bool init_SD()
{
  File myFile;
  if (!SD.begin( 10 ))
  {
    SDgood = false;
    return false;
  }

  SDgood = true;
  // get the file names for initializing players
  FileParser FP( "/initFileNames.txt" );
  // DataPlayers
  SDnumFiles = 0;
  int N = 0;// to read into
  FP >> N;
  numDPinUse = N;// write to int   
  for( unsigned int n = 0; n < numDPinUse; ++n )  
    FP >> SDfileNames[n];// write to String
  SDnumFiles += numDPinUse;
  // ltPlay[]
  FP >> N;
  numLPinUse = N;
  for( unsigned int n = 0; n < numLPinUse; ++n )  
    FP >> SDfileNames[n + SDnumFiles];
  SDnumFiles += numLPinUse;

  // Radiator
  FP >> SDfileNames[ SDnumFiles++ ];
  // ringPlay_Arr[]
  FP >> N;
  numRPfiles = N;
  for( unsigned int n = 0; n < numRPfiles; ++n )  
    FP >> SDfileNames[n + SDnumFiles];
  SDnumFiles += numRPfiles;

  // wvPlay[]
  FP >> N;
  numWPinUse = N;
  for( unsigned int n = 0; n < numWPinUse; ++n )  
    FP >> SDfileNames[n + SDnumFiles];
  SDnumFiles += numWPinUse;

  // player change period
  FP >> tPeriodWP;
  FP >> tTransWP;
  // which blend method
  FP >> N;
  doBlendBuffers = ( N == 1 );

  return true;
}

bool getRingData()
{
  FileParser FP( "/BulkRingData.txt" );
  int N = 0;// to read into
  FP >> N;
  numRingDataInUse = N;// write to int  
  for( unsigned int k = 0; k < N; ++k )
  {
    ringData[k].init( FP );
  }

  return true;
}

// for a voltage sensor on A1. Display to oled
const int maxSamples = 50;
int voltsIn[ maxSamples ];// displaying RMS deviation of ADC values
int voltsInSum = 0;// sum of ADC readings for average value
float tSampleIV = 0.4f, tElapIV = 0.0f;// read and display every tSample seconds
int numSamples = 0;// ++ each read

// 1st oled writer
void updateIVsensors( float dt )
{
  // every loop()
  
  tElapIV += dt;

  if( numSamples < maxSamples )
  {
    voltsIn[ numSamples ] = analogRead( A1 );
    voltsInSum += voltsIn[ numSamples ];
    ++numSamples;
  }

  if( tElapIV >= tSampleIV )
  {
    tElapIV = 0.0f;// reset always
    
    if( numSamples > 10 )// else keep adding samples
    {
      // analyze sample data
      float voltsAvg = (float)voltsInSum/(float)numSamples;
      float sumSqDev = 0.0f;
      for( int n = 0; n < numSamples; ++n )
      {
        float dev = voltsIn[n] - voltsAvg;
        sumSqDev += dev*dev;
      }
      // average of dev*dev
      sumSqDev /= numSamples;
      float stDev = sqrtf( sumSqDev );
    
      voltsAvg *= 8.84f/2052.0;// sensor range ?
      oledMsg[0] = "Volts: ";
      oledMsg[0] += voltsAvg;
    //  oledMsg[0] += "\nstdv ADC: ";
    //  oledMsg[0] += stDev;
      oledMsg[0] += "\ndtRing: ";
      oledMsg[0] += ( 1000.0f*dtRing);
      oledMsg[1] = "dtRPupdate: ";
      oledMsg[1] += ( dtRPup );
      oledMsg[2] = "numPlaying: ";
      oledMsg[2] += ( numRPplaying );

      showOled = true;
      
      // reset more stuff
      voltsInSum = 0;
      numSamples = 0;
    }    
  }
}

void initFloatMenu()
{
  const unsigned int numVals = 4;
  floatMenu.Alloc( numVals, oledMsg[3] );
  floatMenu.thePot.setup(  A0, 4, 3, 3 );  
  floatMenu.editModeButt.init( 3, 0.01f, false );// initially closed
  floatMenu.ButtType = 'C';// normally closed
  // each value
  floatMenu.initVal( 0, "fadeR", fadeRratio, 0.4f, 5.0f, 3.0f/256.0f );  
  floatMenu.initVal( 1, "fadeW", fadeWratio, 0.4f, 5.0f, 3.0f/256.0f );
  floatMenu.initVal( 2, "spawnTime", spawnTime, 0.1f, 5.0f, 0.5f/256.0f );
  floatMenu.initVal( 3, "speedFactor", speedFactor, 0.2f, 5.0f, 2.0f/256.0f );

  floatMenu.makeMsg();// to prepare initial message
}

/*
//** writes to oledMsg[3] **
slidePot playPot( A0, 4, 3, 3 );// value range: 0 to 256
int editMode = 0;// 0 = display values only. 1, 2, 3, 4 = set float value
// a button to set edit mode on the oled display
switchSPST pushButtPP( 3, 0.01f );// sensor type open/close state reversed ( normally closed )

const int numVals = 4;// float values to adjust
String valLabel[ numVals ];// "label: "
float* pVal[ numVals ];// a pointer for writing for each
float minVal[ numVals ];
float maxVal[ numVals ];
float potFactor[ numVals ];// pot sensitivity

void initMenu()
{
  valLabel[0] = "fadeR";// insert "\n* " or "\n  " before
  pVal[0] = &fadeRratio;// RP.fadeRadius = fadeRatio*ringWidth
  minVal[0] = 0.4f;
  maxVal[0] = 5.0f;
  potFactor[0] =  3.0f/256.0f;

  valLabel[1] = "fadeW";// insert "\n* " or "\n  " before
  pVal[1] = &fadeWratio;// RP.fadeWidth = fadeRatio*ringWidth
  minVal[1] = 0.4f;
  maxVal[1] = 5.0f;
  potFactor[1] =  3.0f/256.0f;

  valLabel[2] = "spawnTime";// insert "\n* " or "\n  " before
  pVal[2] = &spawnTime;
  minVal[2] = 0.1f;
  maxVal[2] = 5.0f;
  potFactor[2] =  0.5f/256.0f;

  valLabel[3] = "speedFactor";// insert "\n* " or "\n  " before
  pVal[3] = &speedFactor;// modulates range of RP.ringSpeed assignments
  minVal[3] = 0.2f;
  maxVal[3] = 5.0f;
  potFactor[3] =  1.0f/256.0f;
}

void updateMenu( float dt )
{
  // using pushButtPP to set editMode
  pushButtPP.update(dt);
  // cycle editMode
  if( pushButtPP.pollEvent() == -1 )// press event
  {
    ++editMode;
    editMode %= 1 + numVals;// range: 0 to numVals
    showOled = true;// move asterisk to next line
  }

  if( editMode > 0 )// && pushButtPP.isClosed )// 1, 2, 3 or 4
  {
    int potDiff = playPot.updateChange();// may re center pot while button is pressed
    if( potDiff != 0 && pushButtPP.isClosed )// button has been released
    {   
      showOled = true;
      unsigned int i = editMode - 1;
      float& val = *pVal[i];// for convenience
      val += (float)potDiff*potFactor[i];
      if( val < minVal[i] ) val = minVal[i];
      else if( val > maxVal[i] ) val = maxVal[i];
    }
  }

  if( showOled )
  {
    // 1st line
    if( editMode == 1 ) oledMsg[3] = "* " + valLabel[0] + " :";
    else oledMsg[3] = "  " + valLabel[0] + " :";
    oledMsg[3] += *pVal[0];

    // editMode == 2, 3 or 4
    for( unsigned int n = 1; n < numVals; ++n )
    {
      if( editMode == n + 1 ) oledMsg[3] += "\n* " + valLabel[n] + " :";
      else oledMsg[3] += "\n  " + valLabel[n] + " :";
      oledMsg[3] += *pVal[n];
    }    
  }

}
*/