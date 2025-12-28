#include "AL_WavePlayBlending.h"


bool AL_WavePlayBlending::setup( const char* setupFilename, SSD1306_Display* p_Display )
{
    // validate the target grid
    if( !Target_LG.pLt0 ) return false;
    if( Target_LG.rows <= 0 || Target_LG.cols <= 0 ) return false;

    // parse data from file or other
    FileParser fin( setupFilename );
  //  if( !fin ) return false;

    fin >> numWP >> maxCoeffs;
    // allocate waveplayers
    wvPlay = new WavePlayer2[ numWP ];

    int ofstMid = numWP*maxCoeffs;// left 1st
    int arrSz = 2*ofstMid;
    // allocate float storage
    wv_Coeff = new float[ arrSz ];

    // the buffer grid
    BufferGrid.init( Target_LG.rows, Target_LG.cols );

    // a filename for each
    for( int n = 0; n < numWP; ++n )
    {
      String fName;
      fin >> fName;
      // assign storage to player
      wvPlay[n].C_Lt  = wv_Coeff + n*maxCoeffs;
      wvPlay[n].C_Rt  = wv_Coeff + ofstMid + n*maxCoeffs;
      init_WavePlayer( wvPlay[n], fName.c_str() );
    }

    // other members
    tPeriodWP = 8.0f;
    tTransWP = 2.0f;
    tElapWP = 0.0f;
    currWP = 0;

    pDisplay = p_Display;
    menuIter = 0;
    updateDisplay();

    return true;
}

bool AL_WavePlayBlending::init_WavePlayer( WavePlayer2& WP, const char* fileName )
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

  WP.init( Target_LG.pLt0[0], Target_LG.rows, Target_LG.cols, HiLt, LoLt );
  // wave data
  float AmpLt, AmpRt, wvLen_lt, wvSpd_lt, wvLen_rt, wvSpd_rt;
  FP >> AmpLt >> wvLen_lt >> wvSpd_lt >> AmpRt >> wvLen_rt >> wvSpd_rt;
  WP.setWaveData( AmpRt, wvLen_lt, wvSpd_lt, wvLen_rt, wvSpd_rt );// all right
  WP.setTargetRect( Rows, Cols, Row0, Col0 );

//   WP.C_Rt =  WP.C_Lt = nullptr;
//  return true;

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

void AL_WavePlayBlending::blendBuffers( float U, float dt )
{
    if( !Target_LG.pLt0 ) return;
    unsigned int nextWP = ( 1 + currWP )%numWP;

    // update both
    wvPlay[ currWP ].update(dt);// writing to LightArr
    wvPlay[ nextWP ].pLt0 = BufferGrid.pLt0;
    wvPlay[ nextWP ].update(dt);// writing to BufferArr

    // blend buffers
    float Q = 1.0f - U;
    int numLEDs = Target_LG.rows*Target_LG.cols;
    for( int n = 0; n < numLEDs; ++n )
    {
        //            current                    next
        float rd = Q*(float)Target_LG.pLt0[n].r + U*(float)BufferGrid.pLt0[n].r;
        float gn = Q*(float)Target_LG.pLt0[n].g + U*(float)BufferGrid.pLt0[n].g;
        float bu = Q*(float)Target_LG.pLt0[n].b + U*(float)BufferGrid.pLt0[n].b;
        Target_LG.pLt0[n].r = rd;
        Target_LG.pLt0[n].g = gn;
        Target_LG.pLt0[n].b = bu;
    }
}

bool AL_WavePlayBlending::update( float dt )
{
    if( !Target_LG.pLt0 ) return false;    

    int numLEDs = Target_LG.rows*Target_LG.cols;
    if( tElapWP >= tPeriodWP )// transition in progress
      for( int n = 0; n < numLEDs; ++n )
        BufferGrid.pLt0[n] = clearLt;

    // updateWP()
    int nextWP = ( 1 + currWP )%numWP;// roll index
    tElapWP += dt;
    dt *= timeScale;// for the player updates

    if( tElapWP < tPeriodWP )
    {
      wvPlay[ currWP ].update(dt);
    }
    else if( tElapWP < tPeriodWP + tTransWP )// blend
    {
      float U = ( tElapWP - tPeriodWP )/tTransWP;
      blendBuffers( U, dt );// both will update
    }
    else// transition has ended
    {
      tElapWP = 0.0f;// reset cycle
      currWP = nextWP;
      wvPlay[ currWP ].pLt0 = Target_LG.pLt0;
      wvPlay[ currWP ].update(dt);
    }

    return true;
}

bool AL_WavePlayBlending::handleEvent( ArduinoEvent& rEvent )
{
    // handle Quit up front
    if( rEvent.type == -1 && (rEvent.ID == actButtID) && (menuIter == numOptions - 1) )
        return false;// Quit

    // return will be true from here
    if( rEvent.type == 1 )// button press
    {
        if( rEvent.ID == actButtID )
        {
            if( menuIter == 0 )
            {
           //     tFrame = 0.2f;
            }
        }
        else if( rEvent.ID == menuButtID )// menu scroll button
        {
            menuIter = ( 1 + menuIter )%numOptions;
            updateDisplay();
        }
    }
    else if( rEvent.type == -1 )// button release
    {
        if( rEvent.ID == actButtID )// act
        {
            if( menuIter == 0 )
            {
          //      tFrame = 1.0f;
            }
        }
    }
    else if( rEvent.type == 2 && rEvent.ID == rotEncID )// rotary encoder
    {
        if( menuIter == 0 )
        {
          tPeriodWP  += rotEncScale*rEvent.value;
          if( tPeriodWP < 1.0f ) tPeriodWP = 1.0f;
        }
        else if( menuIter == 1 )
        {
            tTransWP  += rotEncScale*rEvent.value;
            if( tTransWP < 1.0f ) tTransWP = 1.0f;
        }
        else if( menuIter == 2 )
        {
            timeScale  += 0.05f*rEvent.value;
            if( timeScale < 0.1f ) timeScale = 0.1f;
        }

        updateDisplay();
    }

    return true;
}

void AL_WavePlayBlending::draw()const
{
    if( !Target_LG.pLt0 ) return;
    // draw is in update()
}

void AL_WavePlayBlending::updateDisplay()const
{
    if( !pDisplay ) return;// crash avoidance
    String msg( "WavePlayer" );
    // times
    msg += ( menuIter == 0 ) ? "\n* " : "\n  ";
    msg += "tPeriodWP: ";
    msg += tPeriodWP;
    msg += ( menuIter == 1 ) ? "\n* " : "\n  ";
    msg += "tTransWP: ";
    msg += tTransWP;
    msg += ( menuIter == 2 ) ? "\n* " : "\n  ";
    msg += "timeScale: ";
    msg += timeScale;
    // Quit
    msg += ( menuIter == numOptions - 1 ) ? "\n* " : "\n  ";
    msg += "QUIT to menu";
    // write
    pDisplay->clear();
    pDisplay->printAt( 0, 0, msg.c_str(), 1 );
    pDisplay->show();
}
