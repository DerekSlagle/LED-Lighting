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
    for( int n = 0; n < 4; ++n )    
        Coeff_Lt[n] = Coeff_Rt[n] = 0.0f;
    
    theWP.C_Rt = Coeff_Rt;// capacity = 4
    theWP.nTermsRt = 3;
    theWP.C_Rt[0] = 1.0f;

    theWP.C_Lt = Coeff_Lt;
    theWP.nTermsLt = 3;
    theWP.C_Lt[0] = 1.0f;

    theWP.setTargetRect( Target_LG.rows, Target_LG.cols, 0, 0 );

    pDisplay = p_Display;
    menuIter = 0;
    currPage = 0;
    updateDisplay();

    return true;
}

bool AL_WavePlayers::update( float dt )
{
    if( !Target_LG.pLt0 ) return false;    
        
    if( graphMode )
    {
        theWP.update_tElapOnly(dt);
        theWP.drawGraph();
    }
    else
        theWP.update(dt);

    return true;
}

bool AL_WavePlayers::handleEvent( ArduinoEvent& AE )
{
    // handle Quit up front
    if( AE.type == -1 && ( AE.ID == actButtID ) &&
     ( currPage == 0 ) && (menuIter == numOptions[0] - 1) )
        return false;// Quit

    // handle return to Home page
    if( AE.type == -1 && ( AE.ID == actButtID ) &&
     ( currPage > 0 ) && (menuIter == numOptions[ currPage ] - 1) )
     {
        currPage = 0;
        menuIter = 0;
        updateDisplay();
        return true;// Home
     }

    // handle menu scroll
    if( AE.type == 1 && AE.ID == menuButtID )
    {
        menuIter = ( 1 + menuIter )%numOptions[ currPage ];
        updateDisplay();
        return true;
    }

    // return will be true from here
    if( currPage == 0 ) handleMenuEvent_0( AE );
    else if( currPage == 1 ) handleMenuEvent_1( AE );
    else if( currPage == 2 ) handleMenuEvent_2( AE );
    else if( currPage == 3 ) handleMenuEvent_3( AE );

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
    String msg( "BLANK" );
    
    if( currPage == 0 )    
        msg = updatePage_0();
    else if( currPage == 1 )    
        msg = updatePage_1();
    else if( currPage == 2 )    
        msg = updatePage_2();
    else if( currPage == 3 )    
        msg = updatePage_3();    

    // Quit or Home
    msg += ( menuIter == numOptions[ currPage ] - 1 ) ? "\n* " : "\n  ";
    msg += ( currPage == 0 ) ? "QUIT to menu" : "Home";

    // write
    pDisplay->clear();
    pDisplay->printAt( 0, 0, msg.c_str(), 1 );
    pDisplay->show();
}

void AL_WavePlayers::handleMenuEvent_0( ArduinoEvent AE  )
{
    if( currPage != 0 ) return;// wrong call
    if( AE.ID != actButtID ) return;// only action here
    if( AE.type != 1 ) return;// press only

    // scroll through page is handled at top of handleEvent()
    // Quit is also handled
    if( menuIter == 0 )// Page 1: wave length, speed and AmpRt
    {
        currPage = 1;
        menuIter = 0;
        updateDisplay();
    }
    else if( menuIter == 1 )// Page 2: colors
    {
        currPage = 2;
        menuIter = 0;
        updateDisplay();
    }
    else if( menuIter == 2 )// Page 3: coefficients
    {
        currPage = 3;
        menuIter = 0;
        updateDisplay();
    }
    else if( menuIter == 3 )// Toggle graphMode
    {
        graphMode = !graphMode;
        updateDisplay();
    }
}

String AL_WavePlayers::updatePage_0()const
{
    String msg( " WavePlayer Home" );
    msg += ( menuIter == 0 ) ? "\n* " : "\n  ";
    msg += "Adjust wave props";
    msg += ( menuIter == 1 ) ? "\n* " : "\n  ";
    msg += "Adjust colors";
    msg += ( menuIter == 2 ) ? "\n* " : "\n  ";
    msg += "Adjust coefficients";
     msg += ( menuIter == 3 ) ? "\n* " : "\n  ";
    msg += "Graph Mode: ";
    msg += graphMode ? "ON" : "OFF";

    return msg;
}


void AL_WavePlayers::handleMenuEvent_1( ArduinoEvent AE  )
{
    if( currPage != 1 ) return;// wrong call
    if( AE.ID != rotEncID ) return;// only action here
    if( AE.type != 2 ) return;// is from rotary encoder

    // scroll through page is handled at top of handleEvent()
    // Quit is also handled
    if( menuIter == 0 )
    {
        float boost = lengthScale*AE.value;
        if( graphMode ) boost /= theWP.getRows();
        theWP.wvLenRt += boost;
        theWP.periodRt = theWP.wvLenRt/theWP.wvSpdRt;
    }
    else if( menuIter == 1 )
    {
        float boost = speedScale*AE.value;
        if( graphMode ) boost /= theWP.getRows();
        theWP.wvSpdRt += boost;
        theWP.periodRt = theWP.wvLenRt/theWP.wvSpdRt;
        //  std::cout << "\n wvSpdRt = " << theWP.wvSpdRt;
    }
    // left
    else if( menuIter == 2 )
    {
        float boost = lengthScale*AE.value;
        if( graphMode ) boost /= theWP.getRows();
        theWP.wvLenLt += boost;
        theWP.periodLt = theWP.wvLenLt/theWP.wvSpdLt;
    }
    else if( menuIter == 3 )
    {
        float boost = speedScale*AE.value;
        if( graphMode ) boost /= theWP.getRows();
        theWP.wvSpdLt += boost;
        theWP.periodLt = theWP.wvLenLt/theWP.wvSpdLt;
    }
    else if( menuIter == 4 )// AmpRt
    {
        theWP.AmpRt += ampScale*AE.value;
        if( theWP.AmpRt < 0.0f ) theWP.AmpRt = 0.0f;// clamp
        else if( theWP.AmpRt > 1.0f ) theWP.AmpRt = 1.0f;// clamp

        theWP.AmpLt = 1.0f - theWP.AmpRt;
    }

    updateDisplay();
}

String AL_WavePlayers::updatePage_1()const
{
    String msg( " Wave Props" );
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

    return msg;
}

// page 2 = Colors: r,g,b for left and right
void AL_WavePlayers::handleMenuEvent_2( ArduinoEvent AE  )// 6 x type 2
{
    if( currPage != 2 ) return;// wrong call
    if( AE.ID != rotEncID ) return;// only action here
    if( AE.type != 2 ) return;// is from rotary encoder

    // scroll through page is handled at top of handleEvent()
    // Quit is also handled
    if( menuIter == 0 ) theWP.frHi += AE.value;        
    else if( menuIter == 1 ) theWP.fgHi += AE.value;
    else if( menuIter == 2 ) theWP.fbHi += AE.value;
    // lo
    else if( menuIter == 3 ) theWP.frLo += AE.value;
    else if( menuIter == 4 ) theWP.fgLo += AE.value;
    else if( menuIter == 5 ) theWP.fbLo += AE.value;

    updateDisplay();
}
// Colors
String AL_WavePlayers::updatePage_2()const
{
    String msg( " Colors" );
    msg += ( menuIter == 0 ) ? "\n* " : "\n  ";
    msg += "Hi Red:   "; msg += theWP.frHi;
    msg += ( menuIter == 1 ) ? "\n* " : "\n  ";
    msg += "Hi Green: "; msg += theWP.fgHi;
    msg += ( menuIter == 2 ) ? "\n* " : "\n  ";
    msg += "Hi Blue:  "; msg += theWP.fbHi;

    msg += ( menuIter == 3 ) ? "\n* " : "\n  ";
    msg += "Lo Red:   "; msg += theWP.frLo;
    msg += ( menuIter == 4 ) ? "\n* " : "\n  ";
    msg += "Lo Green: "; msg += theWP.fgLo;
    msg += ( menuIter == 5 ) ? "\n* " : "\n  ";
    msg += "Lo Blue:  "; msg += theWP.fbLo;

    return msg;
}    

// page 3 = Coefficients: 3 each for left and right
void AL_WavePlayers::handleMenuEvent_3( ArduinoEvent AE  )// 6 x type 2
{
    if( currPage != 3 ) return;// wrong call
 //   if( rEvent.ID != rotEncID ) return;// only action here
 //   if( rEvent.type != 2 ) return;// is from rotary encoder

    // scroll through page is handled at top of handleEvent()
    // Quit is also handled
    float* Coeff = ( menuIter < 3 ) ? theWP.C_Lt + menuIter : theWP.C_Rt + menuIter - 3;

    if( AE.type == 2 && AE.ID == rotEncID )
    {
        const float Scale = 0.1f;
        
        *Coeff += Scale*AE.value;
        if( *Coeff < 0.0f && *Coeff > -0.02 ) *Coeff = 0.0f;// avoid value -0
    }

    // press actButt to re Zero or = 1
    if( AE.type == 1 && AE.ID == actButtID )
    {
        if( menuIter == 0 ) *Coeff = 1.0f;
        else *Coeff = 0.0f;
    }

    updateDisplay();
}

String AL_WavePlayers::updatePage_3()const
{
    String msg( " Coefficients" );
    msg += ( menuIter == 0 ) ? "\n* " : "\n  ";
    msg += "Left 0: "; msg += theWP.C_Lt[0];
    msg += ( menuIter == 1 ) ? "\n* " : "\n  ";
    msg += "Left 1: "; msg += theWP.C_Lt[1];
    msg += ( menuIter == 2 ) ? "\n* " : "\n  ";
    msg += "Left 2: "; msg += theWP.C_Lt[2];

    msg += ( menuIter == 3 ) ? "\n* " : "\n  ";
    msg += "Right 0: "; msg += theWP.C_Rt[0];
    msg += ( menuIter == 4 ) ? "\n* " : "\n  ";
    msg += "Right 1: "; msg += theWP.C_Rt[1];
    msg += ( menuIter == 5 ) ? "\n* " : "\n  ";
    msg += "Right 2: "; msg += theWP.C_Rt[2];

    return msg;
}
