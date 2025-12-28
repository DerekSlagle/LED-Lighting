#include "AL_WavePlayers.h"

AL_WavePlayers::AL_WavePlayers()
{
    //ctor
}

AL_WavePlayers::~AL_WavePlayers()
{
    //dtor
}

bool AL_WavePlayers::setup( SSD1306_Display* p_Display )
{
    for( int n = 0; n < 4; ++n )
        Coeff_Lt[n] = Coeff_Rt[n] = 0.0f;

    // simple setup
    Light HiLt(0,200,40), LoLt(0,40,200);
    theWP.init( *Target_LG.pLt0, Target_LG.rows, Target_LG.cols, HiLt, LoLt );
    float AmpRt = 1.0f, wvLen_lt = 64.0f, wvSpd_lt = 64.0f, wvLen_rt = 256.0f, wvSpd_rt = 256.0f;
    theWP.setWaveData( AmpRt, wvLen_lt, wvSpd_lt, wvLen_rt, wvSpd_rt );
    // setSeriesCoeffs
    theWP.C_Rt = theWP.C_Lt = nullptr;
    theWP.nTermsRt = theWP.nTermsLt = 0;
    theWP.setTargetRect( Target_LG.rows, Target_LG.cols, 0, 0 );

    pDisplay = p_Display;
    menuIter = 0;
    updateDisplay();

    return true;
}

bool AL_WavePlayers::update( float dt )
{
    if( !Target_LG.pLt0 ) return false;
    theWP.update(dt);
    return true;
}

bool AL_WavePlayers::handleEvent( ArduinoEvent& rEvent )
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
            // draw to display
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
            theWP.wvLenRt += lengthScale*rEvent.value;
            theWP.periodRt = theWP.wvLenRt/theWP.wvSpdRt;
        }
        else if( menuIter == 1 )
        {
            theWP.wvSpdRt += speedScale*rEvent.value;
            theWP.periodRt = theWP.wvLenRt/theWP.wvSpdRt;
          //  std::cout << "\n wvSpdRt = " << theWP.wvSpdRt;
        }
        // left
        else if( menuIter == 2 )
        {
            theWP.wvLenLt += lengthScale*rEvent.value;
            theWP.periodLt = theWP.wvLenLt/theWP.wvSpdLt;
        }
        else if( menuIter == 3 )
        {
            theWP.wvSpdLt += speedScale*rEvent.value;
            theWP.periodLt = theWP.wvLenLt/theWP.wvSpdLt;
        }
        else if( menuIter == 4 )// AmpRt
        {
            theWP.AmpRt += ampScale*rEvent.value;
            if( theWP.AmpRt < 0.0f ) theWP.AmpRt = 0.0f;// clamp
            else if( theWP.AmpRt > 1.0f ) theWP.AmpRt = 1.0f;// clamp
            theWP.AmpLt = 1.0f - theWP.AmpRt;
        }

        updateDisplay();
    }

    return true;
}

void AL_WavePlayers::draw()const
{
    if( !Target_LG.pLt0 ) return;
    // draw is in update()
}

void AL_WavePlayers::updateDisplay()const
{
    if( !pDisplay ) return;// crash avoidance
    String msg( "WavePlayer" );
    // right
    msg += ( menuIter == 0 ) ? "\n* " : "\n  ";
    msg += "wvLenRt: ";
    msg += theWP.wvLenRt;
    msg += ( menuIter == 1 ) ? "\n* " : "\n  ";
    msg += "wvSpdRt: ";
    msg += theWP.wvSpdRt;
    // left
    msg += ( menuIter == 2 ) ? "\n* " : "\n  ";
    msg += "wvLenLt: ";
    msg += theWP.wvLenLt;
    msg += ( menuIter == 3 ) ? "\n* " : "\n  ";
    msg += "wvSpdLt: ";
    msg += theWP.wvSpdLt;
    msg += ( menuIter == 4 ) ? "\n* " : "\n  ";
    msg += "AmpRT  : ";
    msg += theWP.AmpRt;

    // Quit
    msg += ( menuIter == numOptions - 1 ) ? "\n* " : "\n  ";
    msg += "QUIT to menu";
    // write
    pDisplay->clear();
    pDisplay->printAt( 0, 0, msg.c_str(), 1 );
    pDisplay->show();
}
