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
//#include "DataPlayer_initFuncs.h"
#include "LightPanel.h"
#include "switchSPST.h"

// the full target rectangle
const int gridRowsFull = 16, gridColsFull = 32;// landscape
const unsigned int numLEDs = static_cast<unsigned int>( gridRowsFull*gridColsFull );

// 4 panels per 32 row x 16 column LED strip
const unsigned int numPanels = 8;// for 64*8 = 512 Lights
LightPanel panel[ numPanels ];

Light LightArr[numLEDs];// players write to this array
CRGB leds[numLEDs];// panels map above to this array

patternData PD_Arr[50];
const unsigned int numLP = 5;
unsigned int numLPinUse = 0;// from file
LightPlayer2 LP_Arr[numLP];

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
const unsigned int numWP = 5;
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
Light BufferArr[numLEDs];// wvPlay[ nextWP ] writes to this array

// for a future try at blending member values
// not in use yet
WavePlayer2 wvPlayTrans;// player writing during transition.
float wvC_rtTrans[5], wvC_ltTrans[5];

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
String OLED_bytePoolReport();// image data for DataPlayers
String OLED_colorPoolReport();// color data for DataPlayers
// a pushbutton to toggle output on above
switchSPST pushButtA;
void PBclose()
{  
  tinyOledA.clear();
  tinyOledA.printAt( 0, 0, OLED_colorPoolReport().c_str(), 1 );
  tinyOledA.show();
}
void PBopen()
{  
  tinyOledA.clear();
  tinyOledA.printAt( 0, 0, OLED_bytePoolReport().c_str(), 1 );
  tinyOledA.show();
}

void setup()
{
  tinyOledA.setupDisplay();
  pushButtA.init( 2, 0.1f, true );// normally open so iClosed = false. This will fake an open event.
  pushButtA.onClose = PBclose;
  pushButtA.onOpen = PBopen;

  init_SD();
  init_1();    

  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(32);

  tinyOledA.setTextColor( COLOR_WHITE );
}

void loop() 
{
  FastLED.clear();
  loop_1();
	FastLED.show();
  // button
  pushButtA.update( 0.02f );

	delay(2.f);
}

void init_panels( const char* fileName );// from file on SD card
bool init_DataPlayer( DataPlayer& DP, const char* fileName );
bool init_LightPlayer2( LightPlayer2& LP, const char* fileName );
bool init_WavePlayer( WavePlayer2& WP, const char* fileName );// due to assoc with

void init_1()
{  
  // DataPlayers
  for( unsigned int n = 0; n < numDPinUse; ++n )
    init_DataPlayer( DP_Arr[n], SDfileNames[n].c_str() );

  // LightPlayer2
  for( unsigned int n = 0; n < numLPinUse; ++n )
    init_LightPlayer2( LP_Arr[n], SDfileNames[ n + numDPinUse ].c_str() );

  // WavePlayer2
  tElapWP = 0.0f;// player changes eeach tPeriodWP
  for( unsigned int n = 0; n < numWPinUse; ++n )
  {
    for( unsigned int j = 0; j < 5; ++j )
    {  wvC_lt[n][j] = wvC_rt[n][j] = 0.0f; }// initialize float values
    // assign storage to player
    wvPlay[n].C_Lt  = wvC_lt[n];
    wvPlay[n].C_Rt  = wvC_rt[n];
    init_WavePlayer( wvPlay[n], SDfileNames[ n + numDPinUse + numLPinUse ].c_str() );
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

  // the display panels
  init_panels("/panelsConfig.txt");
}

void updateWP( float dt );// all in 1
// 2 blending methods
void blendBuffers( float U, float dt );
void blendWP( float U, float dt );
bool doBlendBuffers = true;// use 1st method

void loop_1()
{
  const float dt = 0.02f;// 50 fps

  // clear entire array
  for( unsigned int n = 0; n < numLEDs; ++n )  
    LightArr[n] = Light(0,0,240);

  if( tElapWP >= tPeriodWP )// transition in progress
    for( unsigned int n = 0; n < numLEDs; ++n )  
      BufferArr[n] = Light(0,0,240);

  // players update to LightArr

  // the current wave player
 // wvPlay[ currWP ].update( dt );
  // next player?
//  tElapWP += dt;
//  if( tElapWP >= tPeriodWP )
//  {
    // instant transition
 //   tElapWP = 0.0f;
  //  currWP = ( 1 + currWP )%numWPinUse;// roll index;
  //  wvPlay[ currWP ].tElapLt = 0.0f;// reset
 //   wvPlay[ currWP ].tElapRt = 0.0f;// start   
 // }

  updateWP( dt );

  // other players
  for( unsigned int k = 0; k < numDPinUse; ++k )
    if( DP_Arr[k].BA.pByte ) DP_Arr[k].update();

  for( unsigned int k = 0; k < numLPinUse; ++k )
    if( LP_Arr[k].pattData ) LP_Arr[k].update();

  // LightPanels map from LightArr to leds
  for( unsigned int p = 0; p < numPanels; ++p )
    panel[p].update();
}

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
  }
  else// transition has ended
  {
    tElapWP = 0.0f;// reset cycle
    currWP = nextWP;
    wvPlay[ currWP ].pLt0 = LightArr;
    wvPlay[ currWP ].update(dt);
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
  wvPlay[ currWP ].update_tElapOnly(dt);
  wvPlay[ nextWP ].update_tElapOnly(dt);
  wvPlayTrans.tElapRt = Q*wvPlay[ currWP ].tElapRt + U*wvPlay[ nextWP ].tElapRt;
  wvPlayTrans.tElapLt = Q*wvPlay[ currWP ].tElapLt + U*wvPlay[ nextWP ].tElapLt;
  wvPlayTrans.update( 0.0f );// do not increment tElap
}

int BPcalls = 0;
String OLED_bytePoolReport()// image data for DataPlayers
{
  String str( "BP: " );
  unsigned int numHeld = 0;
  str += bytePool.BytesHeld();
  str += " of ";
  str += bytePool.poolSz;
  // for each block
  uint8_t* pLastEnd = bytePool.pByte;
  for( unsigned int n = 0; n < bytePool.numUsers; ++n )
  {
    if( !( bytePool.ppBlock[n] && bytePool.pBlockSz[n] ) ) break;// Done!
    // free block?
    int gapSz = *bytePool.ppBlock[n] - pLastEnd;
    if( gapSz > 0 )
    {
      str += "\nFREE ";
      str += gapSz;
      str += " from ";
      str += (int)( pLastEnd - bytePool.pByte );
    }

    // the held block
    str += "\nheld ";
    str += *bytePool.pBlockSz[n];
    str += " from ";
    str += (int)( *bytePool.ppBlock[n] - bytePool.pByte );
    // for next iteration
    pLastEnd = *bytePool.ppBlock[n] + *bytePool.pBlockSz[n];   
  }

  // the big free block at the end
  str += "\nFREE ";
  str += (int)( bytePool.pByte + bytePool.poolSz - pLastEnd );
  str += " from ";
  str += (int)( pLastEnd - bytePool.pByte );
   // call count
  ++BPcalls;
  str += "\ncalls = ";
  str += BPcalls;

  return str;
}

int CPcalls = 0;
String OLED_colorPoolReport()// color data for DataPlayers
{
  String str( "LtPool: " );
  unsigned int numHeld = 0;
  str += colorPool.BytesHeld();
  str += " of ";
  str += colorPool.poolSz;
  // for each block
  Light* pLastEnd = colorPool.pLt0;
  for( unsigned int n = 0; n < colorPool.numUsers; ++n )
  {
    if( !( colorPool.ppBlock[n] && colorPool.pBlockSz[n] ) ) break;// Done!
    // free block?
    int gapSz = *colorPool.ppBlock[n] - pLastEnd;
    if( gapSz > 0 )
    {
      str += "\nFREE ";
      str += gapSz;
      str += " from ";
      str += (int)( pLastEnd - colorPool.pLt0 );
    }

    // the held block
    str += "\nheld ";
    str += *colorPool.pBlockSz[n];
    str += " from ";
    str += (int)( *colorPool.ppBlock[n] - colorPool.pLt0 );
    // for next iteration
    pLastEnd = *colorPool.ppBlock[n] + *colorPool.pBlockSz[n];
  }

  // the big free block at the end
  str += "\nFREE ";
  str += (int)( colorPool.pLt0 + colorPool.poolSz - pLastEnd );
  str += " from ";
  str += (int)( pLastEnd - colorPool.pLt0 );
   // call count
  ++CPcalls;
  str += "\ncalls = ";
  str += CPcalls;

  return str;
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
    PD_Arr[k].init( fIdx, sp, Param );
  }
  
  LP.init( LightArr[0], gridRowsFull, gridColsFull, PD_Arr[0], numPatterns );
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
   float AmpRt, wvLen_lt, wvSpd_lt, wvLen_rt, wvSpd_rt;
   FP >> AmpRt >> wvLen_lt >> wvSpd_lt >> wvLen_rt >> wvSpd_rt;
  WP.setWaveData( AmpRt, wvLen_lt, wvSpd_lt, wvLen_rt, wvSpd_rt );// all right
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