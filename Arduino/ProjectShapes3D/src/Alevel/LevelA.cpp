#include "LevelA.h"

void LevelA::setup()
{
    // simple setup
    Light HiLt(0,200,40), LoLt(0,40,200);
    theWP.init( *Target_LG.pLt0, Target_LG.rows, Target_LG.cols, HiLt, LoLt );
    float AmpRt = 1.0f, wvLen_lt = 64.0f, wvSpd_lt = 64.0f, wvLen_rt = 256.0f, wvSpd_rt = 256.0f;
    theWP.setWaveData( AmpRt, wvLen_lt, wvSpd_lt, wvLen_rt, wvSpd_rt );
    // setSeriesCoeffs
    for( int n = 0; n < 4; ++n ) Coeff_Lt[n] = Coeff_Rt[n] = 0.0f;    
    theWP.C_Rt = Coeff_Rt;// capacity = 4
    theWP.nTermsRt = 3;
    theWP.C_Rt[0] = 1.0f;
    theWP.C_Lt = Coeff_Lt;
    theWP.nTermsLt = 3;
    theWP.C_Lt[0] = 1.0f;
    theWP.setTargetRect( Target_LG.rows/2, Target_LG.cols/2, 4, 8 );

    // setup thePage
    FL_wvLenLt.setupBase( "wvLenLt: ", nullptr );// head
    FL_wvLenLt.setupFloat( theWP.wvLenLt, 10.0f, 1000.0f );
    FL_wvLenLt.inScale = 8.0f;
    FL_wvLenLt.pNextLine = &FL_wvSpdLt;

    FL_wvSpdLt.setupBase( "wvSpdLt: ", nullptr );
    FL_wvSpdLt.setupFloat( theWP.wvSpdLt, 10.0f, 1000.0f );
    FL_wvSpdLt.inScale = 8.0f;
    FL_wvSpdLt.pNextLine = &FL_wvLenRt;

    FL_wvLenRt.setupBase( "wvLenRt: ", nullptr );
    FL_wvLenRt.setupFloat( theWP.wvLenRt, 10.0f, 1000.0f );
    FL_wvLenRt.inScale = 4.0f;
    FL_wvLenRt.pNextLine = &FL_wvSpdRt;

    FL_wvSpdRt.setupBase( "wvSpdRt: ", nullptr );
    FL_wvSpdRt.setupFloat( theWP.wvSpdRt, 10.0f, 1000.0f );
    FL_wvSpdRt.inScale = 4.0f;
    FL_wvSpdRt.pNextLine = &FL_AmpRt;

    FL_AmpRt.setupBase( "AmpRt: ", nullptr );
    FL_AmpRt.setupFloat( theWP.AmpRt, 0.0f, 1.0f );
    FL_AmpRt.inScale = 0.01f;
    FL_AmpRt.pNextLine = nullptr;// tail

    thePage.setup( " ** Wave Player **", FL_wvLenLt );
    if( !ignoreInput ) updateDisplay();
}

void LevelA::processInput()
{
    if( ignoreInput ) return;

    if( thePage.update() )
    {
        // keep wave periods right
        if( MenuLine::pRotEncDelta && *MenuLine::pRotEncDelta != 0 )
        {
            theWP.periodLt = theWP.wvLenLt/theWP.wvSpdLt;
            theWP.periodRt = theWP.wvLenRt/theWP.wvSpdRt;
            theWP.AmpLt = 1.0f - theWP.AmpRt;
        }
    }
}

void LevelA::update( float dt )
{
   if( !Target_LG.pLt0 ) return;
   processInput();   
        
    if( graphMode )
    {
        theWP.update_tElapOnly(dt);
        theWP.drawGraph();
    }
    else
        theWP.update(dt);

    return;
}

void LevelA::draw()const
{
    if( !Target_LG.pLt0 ) return;
    // draw is done in update()
}


void LevelA::updateDisplay()const
{
    String msg = thePage.draw();
    // write
    pDisplay->clear();
    pDisplay->printAt( 0, 0, msg.c_str(), 1 );
    pDisplay->show();
}