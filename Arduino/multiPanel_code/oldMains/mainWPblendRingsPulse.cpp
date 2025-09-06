#include <Arduino.h>
#include <FastLED.h>
#include <SD.h>// a micro SD drive

#include "FileParser.h"
#include "SSD_1306Component.h"// a tiny oled panel

#include "RingPlayer.h"
#include "WavePlayer2.h"

#include "LightPanel.h"
#include "switchSPST.h"
#include "slidePot.h"

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
RingPlayer Radiator;
String RadiatorFileName;
bool init_RingPlayer( RingPlayer& RP, const char* fileName );
float tStart = 0.14f;// Start() 1 per second
float tElapStart = 0.0f;
int idxStartNext = 0;
//float dtRing = 0.02f;// replaces local dt for loop time
void updateRingPlayers( float dt );
bool doShowRings = true;

WavePlayer2 transWP;
const unsigned int maxTerms = 3;// up to 3 terms each
float* coeffsWP = nullptr;// left + right for numWP players
unsigned int coeffsWPsize = 0;// until allocated
//String* fileNameWP = nullptr;
WavePlayer2* wvPlayArr = nullptr;// use new init_SD()
unsigned int numWP = 0;// until allocated
unsigned int currWPidx = 0;
// player timing
float tPeriodWP = 5.0f, tElapWP = 0.0f;// play duration and timer
float tTransWP = 1.0f;// time for transition to next player
// a buffer for the next wave player during transition
Light BufferArr[numLEDs];// wvPlay[ nextWP ] writes to this array if doBlendBuffers

bool init_WavePlayer( WavePlayer2& WP, const char* fileName );
void updateWP( float dt );// calls functions below
// 2 blending methods
bool doBlendBuffers = true;// use 1st method
void blendBuffers( float U, float dt );// nextWP writes to BufferArr
void blendWP( float U, float dt );
// active or not
bool doShowWP = false;

// new
#include "LinePulser.h"
LinePulser LnPulse;// experimental
bool doShowLnPulser = true;

// called within setup() and loop()
void init_1();
void loop_1();

// for SD card
bool SDgood = false;
bool SDfileOpened = false;
bool SDdataGot = false;
bool init_SD();

// a tiny oled screen
SSD1306_Display tinyOledA;
const int numWriters = 2;
String oledMsg[ numWriters ];// 0, 1, 2, 3 as above order
bool showOled = false;// write all Strings to display
int numOledPages = 4, oledPage = 0;// 1st page
// a menu for float values on tinyOledA
#include "Menu_setFloats.h"
Menu_setFloats floatMenuRing, floatMenuWP;// oledPages 1 and 2
Menu_setFloats boolMenu;// abusing to fake bool values
// for   doShowRings        doShowWP          doShowLnPulse
float fDoShowRings = 1.0f, fDoShowWP = 1.0f, fDoShowLnPulse = 1.0f;
void initFloatMenus();

// a pushbutton to toggle WavePlayer blending mode
switchSPST pushButtA;
void PBopen()
{  
    oledPage = ( 1 + oledPage )%numOledPages;
    showOled = true;    
//  if( !Radiator.onePulse );
//  {
//    if( Radiator.isPlaying ) Radiator.StopWave();
//    else  Radiator.Start();
//    oledMsg[2] = Radiator.isRadiating ? "Start" : "STOP";
//    showOled = true;
//  }  
}


void setup()
{
  pushButtA.init( 2, 0.1f, false );// normally open so iClosed = false. This will fake an open event.
  pushButtA.onClose = PBopen;
  pushButtA.onOpen = nullptr;

  srand( micros() );
  init_SD();
  initFloatMenus();// new for

  gridColor.setRGB( 0, 10, 20 );
  init_1();    

  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(32);

  tinyOledA.setupDisplay();
  tinyOledA.setTextColor( COLOR_WHITE );
//  PBopen();// to present initial message
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

void init_1()
{  
  // RingPlayer
  init_RingPlayer( Radiator, RadiatorFileName.c_str() );
  // all members assigned procedurally. Not from file
  for( unsigned int n = 0; n < numRP; ++n )
    ringPlay_Arr[n].initToGrid( &( LightArr[0] ), gridRowsFull, gridColsFull );

  
  // storage to player assigned in initSD()
  currWPidx = 0;
  // trans player during fade period
  transWP.nTermsLt = maxTerms;
  transWP.nTermsRt = maxTerms;// C_Lt, C_Rt assigned in initSD()
  transWP.init( LightArr[0], gridRowsFull, gridColsFull, Light(0,0,0), Light(0,0,0) );
  transWP.setWaveData( 1.0f, 100.0f, 100.0f, 100.0f, 100.0f );
  // match drawing region
  if( wvPlayArr )
  {
    const WavePlayer2& wp0 = wvPlayArr[0];
    transWP.setTargetRect( wp0.getRows(), wp0.getCols(), wp0.getRow0(), wp0.getCol0() );
    // new
//    int rowA = wp0.getRow0(), colA = wp0.getCol0();
//    LnPulse.setLineEnds( rowA, colA, rowA, colA + wp0.getCols() );// top edge of wave player area
//    LnPulse.hiLt.setRGB( 200, 0, 100 );
  }
//  else// set LnPulse elsewhere
//    LnPulse.setLineEnds( 8, 4, 8, 27 );// top edge of wave player area

  // new
 //  LnPulse.initToGrid( LightArr, gridRowsFull, gridColsFull );
//   LnPulse.setPulseProps( 6.0f, 4.0f, 2.0f, 4.0f );
   

  // the display panels
  init_panels("/panelsConfig.txt");

  // clear entire Buffer array
  for( unsigned int n = 0; n < numLEDs; ++n )  
    BufferArr[n] = gridColor;
}

// declarations for further in
// for voltage sensor on pin A1
void updateIVsensors( float dt );// writes to oledMsg[0]
// to support time readout
//float dtRPup = 0.0f;
// wave properties
float fadeRratio = 1.6f;
float fadeWratio = 1.6f;
float spawnTime = 0.5f;// average rate of 2 per second
float speedFactor = 1.0f;// modulates randomly assigned value
// turn LnPulse on or off. Declared with LnPulse
//bool doShowLnPulse = true;// update or not
//float fDoShowLnPulse = 1.0f;// doShowLnPulse = ( fDoShowLnPulse > 0.5f )

void loop_1()
{
  static unsigned long lastTime = micros();
  unsigned long currTime = micros();
  dtLoop = ( currTime - lastTime )*0.000001f;
  lastTime = currTime;
  
  // button. use constant dt
  pushButtA.update( dtLoop );// changes oledPage
  if( oledPage == 0 ) updateIVsensors( dtLoop );// writes to oledMsg[0]
  else if( oledPage == 1 ) floatMenuRing.update( dtLoop );
  else if( oledPage == 2 )
  {
     floatMenuWP.update( dtLoop );
  }
  else if( oledPage == 3 )
  {
     boolMenu.update( dtLoop );
     if( boolMenu.editMode > 0 )
     {
        doShowRings = ( fDoShowRings > 0.5f );
        doShowWP = ( fDoShowWP > 0.5f );
        doShowLnPulser = ( fDoShowLnPulse > 0.5f );        
     }
  }

  // clear entire array
  for( unsigned int n = 0; n < numLEDs; ++n )  
    LightArr[n] = gridColor;

  // time across the WavePlayer updates
  unsigned long startTime = micros();// start
  if( doShowWP ) updateWP( dtLoop );
  unsigned long endTime = micros();// end
  dtWaves = 0.001f*( endTime - startTime );

  // time across the RingPlayer updates
  startTime = endTime;
  if( doShowRings ) updateRingPlayers( dtLoop );
  endTime = micros();// end
  dtRings = 0.001f*( endTime - startTime );

  // new LinePulser over it all
  if( doShowLnPulser )
  {
    LnPulse.update( dtLoop );
    if( !LnPulse.isPlaying ) LnPulse.Start();// re start
  }

  // LightPanels map from LightArr to leds
  for( unsigned int p = 0; p < numPanels; ++p )
    panel[p].update();

  // output to oled
  if( showOled )
  {
    tinyOledA.clear();

    showOled = false;
    // must reset here. How to avoid?
    floatMenuRing.doShow = false;
    floatMenuWP.doShow = false;

    if( oledPage == 0 )// home
    {
      String wholeMsg = oledMsg[0];
      for( int n = 1; n < numWriters; ++n )
        wholeMsg += '\n' + oledMsg[n];      
      tinyOledA.printAt( 0, 0, wholeMsg.c_str(), 1 );
    }
    else if( oledPage == 1 )    
      tinyOledA.printAt( 0, 0, floatMenuRing.StrOut.c_str(), 1 );    
    else if( oledPage == 2 )    
      tinyOledA.printAt( 0, 0, floatMenuWP.StrOut.c_str(), 1 );    
    else if( oledPage == 3 )    
      tinyOledA.printAt( 0, 0, boolMenu.StrOut.c_str(), 1 );    

    tinyOledA.show();
  }
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

bool init_LinePulser( LinePulser& LnPu, const char* fileName )
{
    FileParser FP( fileName );

    int rd, gn, bu;
    FP >> rd >> gn >> bu;
    LnPu.hiLt.setRGB(rd,gn,bu);
    FP >> rd >> gn >> bu;
    LnPu.loLt.setRGB(rd,gn,bu);
    LnPu.initToGrid( LightArr, gridRowsFull, gridColsFull );
    float Speed = 0.0f, PulseWidth = 0.0f, Rdist = 0.0f, FadeDist = 0.0f;
    FP >> Speed >> PulseWidth >> Rdist >> FadeDist;
    LnPu.setPulseProps( Speed, PulseWidth, Rdist, FadeDist );
    FP >> LnPu.Amp;
    // the end points
    int rowA = 0, colA = 0, rowB = 0, colB = 2;
    FP >> rowA >> colA >> rowB >> colB;
    LnPu.setLineEnds( rowA, colA, rowB, colB );
    
    FP >> rd;    
    LnPu.onePulse = ( rd == 1 );

    // read in Ta and Tb ?
    FP >> rd;
    if( rd == 1 )// do it
    {
      FP >> LnPu.Ta.x >> LnPu.Ta.y;
      LnPu.Ta /= LnPu.Ta.mag();// normalize
      LnPu.Sa = 1.0f/( LnPu.Ta.dot( LnPu.Tw ) );

      FP >> LnPu.Tb.x >> LnPu.Tb.y;
      LnPu.Tb /= LnPu.Tb.mag();// normalize
      LnPu.Sb = 1.0f/( LnPu.Tb.dot( LnPu.Tw ) );
    }
  //  RP.Start();

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

  // Radiator
  FP >> RadiatorFileName;

  // WavePlayer files
  int N = 0;
  FP >> N;// number of file names to read
  numWP = N;
  if( N < 2 ) return false;

  // new
  wvPlayArr = nullptr;
  wvPlayArr = new WavePlayer2[ numWP ];
  if( wvPlayArr == nullptr  ) return false;// alloc fail
  // for coeffs storage
  coeffsWPsize = 2*maxTerms*( numWP + 1);// + 1 for transWP;
  coeffsWP = new float[ coeffsWPsize ];
  if( coeffsWP == nullptr  ) return false;// alloc fail
  // initialize
  for( unsigned int j = 0; j < 6*maxTerms; ++j )
  {  coeffsWP[j] = 0.0f; }// initialize float values

  transWP.C_Lt = coeffsWP;
  transWP.C_Rt = coeffsWP + maxTerms;

  String fileNameIn;
  for( unsigned int n = 0; n < numWP; ++n )
  {
    FP >> fileNameIn;
    wvPlayArr[n].C_Lt = coeffsWP + 2*maxTerms*(n+1);
    wvPlayArr[n].C_Rt = wvPlayArr[n].C_Lt + maxTerms;
    init_WavePlayer( wvPlayArr[n], fileNameIn.c_str() );
  }
    

  // player change period
  FP >> tPeriodWP;
  FP >> tTransWP;
  // show wave players?
  FP >> N;
  doShowWP = ( N == 1 );
  // which blend method
  FP >> N;
  doBlendBuffers = ( N == 1 );

  // new
  FP >> fileNameIn;
  init_LinePulser( LnPulse, fileNameIn.c_str() );

  return true;
}

// for a voltage sensor on A1. Display to oledMsg[0]
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
      oledMsg[0] += "\ndtLoop: ";
      oledMsg[0] += ( 1000.0f*dtLoop);
      oledMsg[0] += "\ndtWaves: ";
      oledMsg[0] += ( dtWaves );
      oledMsg[0] += "\ndtRings: ";
      oledMsg[0] += ( dtRings );
      oledMsg[0] += "\nnum rings: ";
      oledMsg[0] += ( numRPplaying );

      showOled = true;
      
      // reset more stuff
      voltsInSum = 0;
      numSamples = 0;
    }    
  }
}

void initFloatMenus()
{
  floatMenuRing.Alloc( 4, "Ring Props" );
  floatMenuRing.thePot.setup(  A0, 4, 3, 3 );  
  floatMenuRing.editModeButt.init( 3, 0.01f, false );// initially closed
  floatMenuRing.ButtType = 'C';// normally closed
  floatMenuRing.pDoShow = &showOled;
  // each value
  floatMenuRing.initVal( 0, "fadeR", fadeRratio, 0.4f, 5.0f, 3.0f/256.0f );  
  floatMenuRing.initVal( 1, "fadeW", fadeWratio, 0.4f, 5.0f, 3.0f/256.0f );
  floatMenuRing.initVal( 2, "spawnTime", spawnTime, 0.1f, 5.0f, 0.5f/256.0f );
  floatMenuRing.initVal( 3, "speedFactor", speedFactor, 0.2f, 5.0f, 2.0f/256.0f );
  floatMenuRing.makeMsg();// to prepare initial message

  floatMenuWP.Alloc( 2, "Wave Props" );
  floatMenuWP.thePot.setup(  A0, 4, 3, 3 );  
  floatMenuWP.editModeButt.init( 3, 0.01f, false );// initially closed
  floatMenuWP.ButtType = 'C';// normally closed
  floatMenuWP.pDoShow = &showOled;
  // each value
  floatMenuWP.initVal( 0, "tPeriodWP", tPeriodWP, 1.0f, 30.0f, 10.0f/256.0f );  
  floatMenuWP.initVal( 1, "tTransWP", tTransWP, 1.0f, 5.0f, 3.0f/256.0f );
  floatMenuWP.makeMsg();// to prepare initial message

  // toggle players on or off
  boolMenu.Alloc( 3, "Show Players" );
  boolMenu.thePot.setup(  A0, 4, 3, 3 );  
  boolMenu.editModeButt.init( 3, 0.01f, false );// initially closed
  boolMenu.ButtType = 'C';// normally closed
  boolMenu.pDoShow = &showOled;
  // each value
  boolMenu.initVal( 0, "Waves", fDoShowWP, 0.0f, 1.0f, 3.0f/256.0f );  
  boolMenu.initVal( 1, "Rings", fDoShowRings, 0.0f, 1.0f, 3.0f/256.0f );
  boolMenu.initVal( 2, "LnPulse", fDoShowLnPulse, 0.0f, 1.0f, 3.0f/256.0f );
  boolMenu.makeMsg();
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

  Radiator.update( dt );
  if( Radiator.isPlaying ) ++numRPplaying;
  

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

// update functions for the wave players
bool isBlending = false;

void updateWP( float dt )
{
  unsigned int nextWP = ( 1 + currWPidx )%numWP;// roll index
  tElapWP += dt;

  if( tElapWP < tPeriodWP )
  {
    wvPlayArr[ currWPidx ].update(dt);
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
    currWPidx = nextWP;
    wvPlayArr[ currWPidx ].pLt0 = LightArr;
    wvPlayArr[ currWPidx ].update(dt);

    if( !doBlendBuffers )
    {
      wvPlayArr[ currWPidx ].tElapLt = wvPlayArr[ currWPidx ].tElapRt = tTransWP;
      transWP.tElapLt = transWP.tElapRt = 0.0;
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
  unsigned int nextWP = ( 1 + currWPidx )%numWP;
  
  // update both
  wvPlayArr[ currWPidx ].update(dt);// writing to LightArr
  wvPlayArr[ nextWP ].pLt0 = BufferArr;
  wvPlayArr[ nextWP ].update(dt);// writing to BufferArr

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
  unsigned int nextWP = ( 1 + currWPidx )%numWP;
  float Q = 1.0f - U;
  transWP.AmpRt = Q*wvPlayArr[ currWPidx ].AmpRt + U*wvPlayArr[ nextWP ].AmpRt;
  transWP.AmpLt = Q*wvPlayArr[ currWPidx ].AmpLt + U*wvPlayArr[ nextWP ].AmpLt;
  // wvLen
  transWP.wvLenRt = Q*wvPlayArr[ currWPidx ].wvLenRt + U*wvPlayArr[ nextWP ].wvLenRt;
  transWP.wvLenLt = Q*wvPlayArr[ currWPidx ].wvLenLt + U*wvPlayArr[ nextWP ].wvLenLt;
  // wvSpd
  transWP.wvSpdRt = Q*wvPlayArr[ currWPidx ].wvSpdRt + U*wvPlayArr[ nextWP ].wvSpdRt;
  transWP.wvSpdLt = Q*wvPlayArr[ currWPidx ].wvSpdLt + U*wvPlayArr[ nextWP ].wvSpdLt;
  // period ? tElap?
   transWP.periodRt =  transWP.wvLenRt/transWP.wvSpdRt;
   transWP.periodLt =  transWP.wvLenLt/transWP.wvSpdLt;
   // the 2 colors
   // Lo
   transWP.frLo = Q*wvPlayArr[ currWPidx ].frLo + U*wvPlayArr[ nextWP ].frLo;
   transWP.fgLo = Q*wvPlayArr[ currWPidx ].fgLo + U*wvPlayArr[ nextWP ].fgLo;
   transWP.fbLo = Q*wvPlayArr[ currWPidx ].fbLo + U*wvPlayArr[ nextWP ].fbLo;
   // Hi
   transWP.frHi = Q*wvPlayArr[ currWPidx ].frHi + U*wvPlayArr[ nextWP ].frHi;
   transWP.fgHi = Q*wvPlayArr[ currWPidx ].fgHi + U*wvPlayArr[ nextWP ].fgHi;
   transWP.fbHi = Q*wvPlayArr[ currWPidx ].fbHi + U*wvPlayArr[ nextWP ].fbHi;

  // coeffs
  // zero all
  for( unsigned int n = 0; n < maxTerms; ++n )
    transWP.C_Rt[n] = transWP.C_Lt[n] = 0.0f;

  // build up
  // right
  if( wvPlayArr[ currWPidx ].C_Rt )
  {
    for( unsigned int n = 0; n < maxTerms; ++n )
     transWP.C_Rt[n] = Q*wvPlayArr[ currWPidx ].C_Rt[n];
  }
  if( wvPlayArr[ nextWP ].C_Rt )
  {
    for( unsigned int n = 0; n < maxTerms; ++n )
     transWP.C_Rt[n] += U*wvPlayArr[ nextWP ].C_Rt[n];
  }
  // left
  if( wvPlayArr[ currWPidx ].C_Lt )
  {
    for( unsigned int n = 0; n < maxTerms; ++n )
      transWP.C_Lt[n] = Q*wvPlayArr[ currWPidx ].C_Lt[n];
  }
  if( wvPlayArr[ nextWP ].C_Lt )
  {
    for( unsigned int n = 0; n < maxTerms; ++n )
      transWP.C_Lt[n] += U*wvPlayArr[ nextWP ].C_Lt[n];
  }

  // ???
  // tElap
  // wvPlay[ currWP ].update_tElapOnly(dt);
  // wvPlay[ nextWP ].update_tElapOnly(dt);
  transWP.tElapRt = Q*wvPlayArr[ currWPidx ].tElapRt + U*wvPlayArr[ nextWP ].tElapRt;
  transWP.tElapLt = Q*wvPlayArr[ currWPidx ].tElapLt + U*wvPlayArr[ nextWP ].tElapLt;
  transWP.update( dt );// do not increment tElap
}