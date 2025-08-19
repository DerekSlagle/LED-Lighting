#include <Arduino.h>
#include <FastLED.h>

#include "LightPanel.h"
#include "DataPlayer_initFuncs.h"
//#include "aniFloatWrite.h"
#include "vec2f.h"

#include "SSD_1306Component.h"// a tiny oled panel
// a tiny oled screen
SSD1306_Display tinyOledA;
// input devices
#include "switchSPST.h"
switchSPST pushButtA;// restart game
switchSPST senseButt;// a touch sensitive pad on pin 3. toggles ballSpeed = hi/lo
#include "slidePot.h"
slidePot potA( A0, 4, 3, 3 );// paddle position

// voltage sensor on pin A1
const int maxSamples = 50;
int voltsIn[ maxSamples ];// displaying RMS deviation of ADC values
int voltsInSum = 0;// sum of ADC readings for average value

float tSampleIV = 0.4f, tElapIV = 0.0f;// read and display every tSample seconds
int numSamples = 0;// ++ each read
void updateIVsensors( float dt );

// The Light arrays
const unsigned int gridRows = 32, gridCols = 32;
const unsigned int numLEDs = gridRows*gridCols;

Light LightArr[numLEDs];// source array
CRGB leds[numLEDs];// target array for the LightPanels below

const unsigned int numPanels = 4;
LightPanel panelArr[ numPanels ];
unsigned int numPanels_inUse = 4;
void setupPanels();

// the ball
Light ballColor(220,0,120);
vec2f ballPos, ballVel;
vec2f ballVelArr[3];// limit ball direction to 0, 15, 30, 45 degrees
float ballSpeed = 16.0f;// constant
// the paddle
Light paddleColor( 0, 140, 60 );
float paddlePos;
bool updatePositions( float dt );

// targets are horizontal and have the same length
const unsigned int numTartgets = 10;
int targetCol[ numTartgets ];
int targetRow[ numTartgets ];
bool targetActive[ numTartgets ];
int targetLength = 3;
Light targetColor( 0, 140, 60 );
void setupTargets();
bool ballHitTarget();// modifies ballVel
int Score = 0;

void renderAll();

// DataPlayer for the Score
DataPlayer DPscore[2];// 2 digits
int ScoreRow = 1, ScoreCol = 1;
uint8_t imageData[100];// only 19 used
Light ColorArr[2];

// pushButtA callback
void buttCall()
{
  ballPos.setXY( 0.5f*gridCols, 1.0f );
  ballVel.setXY( 0.0f, -ballSpeed );
  for( unsigned int n = 0; n < numTartgets; ++n )
    targetActive[n] = true;// reset
  Score = 0;
  DPscore[0].stepIter = 0;
  DPscore[0].col0 = ScoreCol;
  DPscore[1].stepIter = 0;
}

// senseButt callback
void senseBackClose()
{
  if( ballSpeed < 20.0f ) ballSpeed = 30.0f;
  else ballSpeed = 16.0f;
  ballVel *= ballSpeed/ballVel.mag();
}

void setup()
{  
  // a tiny oled screen
  tinyOledA.setupDisplay();
  tinyOledA.setTextColor( COLOR_WHITE );
  // a push button
  pushButtA.init( 2, 0.1f );
  pushButtA.onClose = buttCall;// start game
  // a sense pad
  senseButt.init( 3, 0.1f );// toggle ballSpeed
  senseButt.onClose = nullptr;
  senseButt.onOpen = senseBackClose;// operation is reversed. senseButt reads as normally closed

  ballPos.x = 0.5f*gridCols;
  ballPos.y = 1.0f;// top
  ballVel.x = 0.0f;
  ballVel.y = ballSpeed;
  paddlePos = ballPos.x;// centered initially
  Score = 0;
  // upward velocities. Assign when paddle is hit
  // reverse x component on left end of paddle
  ballVelArr[0].setXY( 0.2588f, -0.8659f );// 15 degrees
  ballVelArr[1].setXY( 0.5f, -0.866f );// 30
  ballVelArr[2].setXY( 0.707f, -0.707f );// 45

  setupTargets();
  // score DPs
  ColorArr[0].setRGB(0,0,200);
  ColorArr[1].setRGB(0,140,100);
  for( unsigned int k = 0; k < 2; ++k )
  {
    init_digitsAniData( LightArr, DPscore[k], imageData, ColorArr );// same data and colors for all digits
    DPscore[k].setGridBounds( ScoreRow, ScoreCol + 4*k, gridRows, gridCols );
    DPscore[k].stepIter = 0;
  }
  
  setupPanels();
  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(64);

}

void setupPanels()
{
 for( unsigned int n = 0; n < numPanels_inUse; ++n )
  panelArr[n].init_Src( LightArr, gridRows, gridCols );// all must match across panels tiling grid

  panelArr[0].set_SrcArea( 16, 16, 16, 0 );
  panelArr[0].rotIdx = -1;
  panelArr[0].pTgt0 = leds;
  panelArr[0].type = 2;

  panelArr[1].set_SrcArea( 16, 16, 0, 0 );
  panelArr[1].rotIdx = 1;
  panelArr[1].pTgt0 = leds + 256;
  panelArr[1].type = 2;  

  Light* pLtLast = leds + 512;
  // 2 to tile the 32x8 panels on the right
  panelArr[2].set_SrcArea( 32, 8, 0, 16 );
  panelArr[2].rotIdx = 0;
  panelArr[2].pTgt0 = pLtLast;
  panelArr[2].type = 2;
  pLtLast += 8*32;
  panelArr[3].set_SrcArea( 32, 8, 0, 24 );
  panelArr[3].rotIdx = 2;
  panelArr[3].pTgt0 = pLtLast;
  panelArr[3].type = 2; 
}

void loopInput( float dt );
void loopGame( float dt );

void loop() 
{
  const float dt = 0.02f;
  loopInput(dt);
  loopGame(dt);
//	delay(8.f);
}


void loopInput( float dt )
{
  int potIn = potA.update();// get stable input
  pushButtA.update(dt);
  senseButt.update(dt);
  updateIVsensors(dt);
 
}

void loopGame( float dt )
{
  if( !updatePositions(dt) )// did not hit the paddle
  {
    if( ballHitTarget() )
    {
      ++Score;
      DPscore[0].stepIter = Score%10;
      DPscore[1].stepIter = Score/10;
      if( DPscore[1].stepIter > 0 )//
      {
        DPscore[1].col0 = ScoreCol;// tens
        DPscore[0].col0 = ScoreCol + 4;// ones
      }
      else
        DPscore[0].col0 = ScoreCol;// ones only
    }
  }

  // clear LightArr
  for( unsigned int n = 0; n < numLEDs; ++n ) LightArr[n].setRGB(0,0,200);  
  // draw to LightArr
  renderAll();

  FastLED.clear();
  // draw to leds array
  for( unsigned int n = 0; n < numPanels_inUse; ++n )
    panelArr[n].update();
  
	FastLED.show();

}

bool updatePositions( float dt )// returns true if ball hit paddle
{
  ballPos += ballVel*dt;
  // hit a wall ?
  if( ballPos.x < 1.0f )// left side
  {
    ballPos.x = 1.0f;    
    if( ballVel.x < 0.0f )
      ballVel.x *= -1.0f;
  }
  else if( ballPos.x > gridCols - 2 )// right side
  {
    ballPos.x = gridCols - 2;    
    if( ballVel.x > 0.0f )
      ballVel.x *= -1.0f;
  }
  else if( ballPos.y < 1 )// top
  {
    ballPos.y = 1;    
    if( ballVel.y < 0.0f )
      ballVel.y *= -1.0f;
  }
  else if( ballPos.y > gridRows + 8 )// bottom at gridRows + 8
  {
    ballPos.y = gridRows + 8;    
    if( ballVel.y > 0.0f )
      ballVel.y *= -1.0f;
  }

  
  const int ballCol = static_cast<int>( ballPos.x );
  const int ballRow = static_cast<int>( ballPos.y );
  // update paddle position
  paddlePos = ( potA.steadyVal*gridCols )/255.0f;
  const int padCol = static_cast<int>( paddlePos );

  // ball vs paddle
  const int dPosX = ballCol - padCol;
  if( ballVel.y > 0.0f && ( ballRow == gridRows - 3 ) )// moving down and just above paddle
  {
  //  if( ( ballRow >= gridRows - 3 ) && ( ballRow <= gridRows - 1 ) )// at or just below paddle
    if( dPosX >= -3 && dPosX <= 3 )// over the paddle
    {
    //  ballRow = gridRows - 3;// just above paddle   
    //  ballPos.y = static_cast<float>( ballRow );

      // reflection based on hit location
      switch( dPosX )
      {
        case 0 :
         ballVel.setXY( 0.0f, -ballSpeed );// straight up
         break;
         case 1 :
         ballVel = ballSpeed*ballVelArr[0];// 15 degrees
         break;
         case 2 :
         ballVel = ballSpeed*ballVelArr[1];// 30 degrees
         break;
         case 3 :
         ballVel = ballSpeed*ballVelArr[2];// 45 degrees
         break;

         case -1 :
         ballVel = ballSpeed*ballVelArr[0];// -15 degrees
         ballVel.x *= -1.0f;
         break;
         case -2 :
         ballVel = ballSpeed*ballVelArr[1];// -30 degrees
         ballVel.x *= -1.0f;
         break;
         case -3 :
         ballVel = ballSpeed*ballVelArr[2];// -45 degrees
         ballVel.x *= -1.0f;// left
         break;
         default :
         break;
      }
      return true;
    }// end if bounce
  }// end if moving down and over paddle
  
  return false;
}// end of updatePositions()


// returns true if ball hit a target
bool ballHitTarget()
{
  const int ballCol = static_cast<int>( ballPos.x );
  const int ballRow = static_cast<int>( ballPos.y );
  for( unsigned int n = 0; n < numTartgets; ++n )
  {
    if( !targetActive[n] ) continue;
    // hit from above or below
    if( ( ballVel.y < 0.0f && ballRow == targetRow[n] + 1 ) // from below
        || ( ballVel.y > 0.0f && ballRow == targetRow[n] - 1 ) )// from above
    {
      int dPosC = ballCol - targetCol[n];
            
      switch( dPosC )
      {
        case -1:// grazed left end        
        case 3:// grazed right end
        ballVel.x *= -1.0f;// reflect horiz
        targetActive[n] = false;
        return true;
        case 0:
        case 1:
        case 2:
        
        ballVel.y *= -1.0f;// reflect vert
        targetActive[n] = false;
        return true;
      }// end switch
    }// end if  
  }// end for

  return false;
}

void setupTargets()
{
  // row 0
  targetRow[0] = 0; targetCol[0] = 0;
  targetRow[1] = 0; targetCol[1] = 10;
  targetRow[2] = 0; targetCol[2] = 20;
  targetRow[3] = 0; targetCol[3] = 29;

  // row 6
  targetRow[4] = 6; targetCol[4] = 5;
  targetRow[5] = 6; targetCol[5] = 25;

  // row 12
  targetRow[6] = 12; targetCol[6] = 0;
  targetRow[7] = 12; targetCol[7] = 10;
  targetRow[8] = 12; targetCol[8] = 20;
  targetRow[9] = 12; targetCol[9] = 29;

  for( unsigned int n = 0; n < numTartgets; ++n )
    targetActive[n] = true;
}

void renderAll()
{
  const int ballCol = static_cast<int>( ballPos.x );
  const int ballRow = static_cast<int>( ballPos.y );
  const int padCol = static_cast<int>( paddlePos );
  // render the paddle
  int lt = padCol - 3;
  Light* pLt = LightArr + ( gridRows - 1 )*gridCols;
  for( int c = lt; c < lt + 7; ++c  )
  {
    if( c >= 0 && c < gridCols )    
      pLt[c] = paddleColor;    
  }

  // render the 3x3 ball
  if( ballRow < gridRows + 1 )// on grid
  {
    for( int r = ballRow-1; r <= ballRow+1; ++r )
      for( int c = ballCol-1; c <= ballCol+1; ++c )
      {
        if( r < 0 ) continue;
        if( r >= gridRows ) break;
        if( c < 0 ) continue;
        if( c >= gridCols ) break;
        LightArr[ r*gridCols + c ] = ballColor;
      }
  }
  // the targets
  for( unsigned int n = 0; n < numTartgets; ++n )
  {
    if( targetActive[n] )
    {
      for( unsigned int c = 0; c < targetLength; ++c )
      {
        int i = targetRow[n]*gridCols + targetCol[n] + c;
        if( i >= 0 && i < numLEDs )// bound check
          LightArr[i] = ballColor;
      }
    }
  }

  // the Score
  DPscore[0].update();// ones digit
  DPscore[0].stepTimer = 0;
  if( DPscore[1].stepIter > 0 )// tens digit
  {
    DPscore[1].update();
    DPscore[1].stepTimer = 0;
  }

}// end of renderAll()

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
    
    //  float fVolts = (float)voltsInSum/(float)numSamples;
      voltsAvg *= 8.84f/2052.0;// sensor range ?
    //  voltsIn = static_cast<int>( voltsAvg );// floors value
      String msg("Volts: ");
      msg += voltsAvg;
      msg += "\nstdv ADC: ";
      msg += stDev;
      
      // reset more stuff
      voltsInSum = 0;
      numSamples = 0;

      // write the msg
      tinyOledA.clear();
      tinyOledA.printAt( 0, 0, msg.c_str(), 1 );
      tinyOledA.show();
    }

    
  }
}