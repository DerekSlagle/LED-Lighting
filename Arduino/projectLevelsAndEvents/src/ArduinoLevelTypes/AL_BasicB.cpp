#include "AL_BasicB.h"

bool AL_BasicB::setup( SSD1306_Display* p_Display )
{
    tFrame = 0.4f;
    tElap = 0.0f;

    // setup home page
    // int line A
    IL_0A.setupBase( "iVal A: ", &bVal_0A );
    iVal_0A = 0;
    IL_0A.setupInt( iVal_0A, 0, 31 );
    IL_0A.pNextLine = &IL_0B;
    // int line B
    IL_0B.setupBase( "iVal B: ", &bVal_0B );
    iVal_0B = 32;
    IL_0B.setupInt( iVal_0B, 32, 63 );
    IL_0B.pNextLine = &ML_01;
    // goto page 1
    ML_01.setupBase( "Goto page 1", gotoPage + 1 );
    ML_01.pNextLine = &ML_02;
    // goto page 2
    ML_02.setupBase( "Goto page 2", gotoPage + 2 );
    ML_02.pNextLine = &ML_Quit;
    ML_Quit.setupBase( " Quit" );
    ML_Quit.pNextLine = nullptr;

    Page[0].setup( "    ** Home Page **", DoUpdateOled, IL_0A );

    // page 1
    // int line A
    IL_1A.setupBase( "iVal 1A: ", &bVal_1A );
    iVal_1A = 32*4;// 5th row
    IL_1A.setupInt( iVal_1A, 32*4, 32*5 - 1 );
    IL_1A.pNextLine = &IL_1B;
    // int line B
    IL_1B.setupBase( "iVal 1B: ", &bVal_1B );
    iVal_1B = 32*5;// 6th row
    IL_1B.setupInt( iVal_1B, 32*5, 32*6-1 );
    IL_1B.pNextLine = &ML_1home;
    ML_1home.setupBase( "Home", gotoPage );
    ML_1home.pNextLine = nullptr;
    // call after
    Page[1].setup( "    ** Page 1 **", DoUpdateOled, IL_1A );
  //  Page[1].lastLine.setupBase( "Go to Home", gotoPage );// go to page 0

    // page 2
  //  Page[2].setup( "Page 2", DoUpdateOled, IL_2A );
  //  Page[2].lastLine.setupBase( "to Home page", gotoPage );// go to page 0
    // int line A
    IL_2A.setupBase( "iVal 2A: ", &bVal_2A );
    iVal_2A = 32*8;// 9th row
    IL_2A.setupInt( iVal_2A, 32*8, 32*9-1 );
    IL_2A.pNextLine = &IL_2B;
    // int line B
    IL_2B.setupBase( "iVal 2B: ", &bVal_2B );
    iVal_2B = 32*9;// 10th row
    IL_2B.setupInt( iVal_2B, 32*9, 32*10-1 );
    IL_2B.pNextLine = &ML_21;
    // goto page 1
    ML_21.setupBase( "Goto page 1", gotoPage + 1 );
    ML_21.pNextLine = &ML_2home;
    ML_2home.setupBase( "Home", gotoPage );
    ML_2home.pNextLine = nullptr;
    // after
    Page[2].setup( "    ** Page 2 **", DoUpdateOled, IL_2A );

    // the page manager
    MMP.setup( Page, gotoPage, numPages );

    // display
    pDisplay = p_Display;
    updateDisplay();

    return true;
}

bool AL_BasicB::update( float dt )
{
    tElap += dt;
    if( tElap > tFrame )
    {
        tElap = 0.0f;
        
    }    

    // after handling goto page event = report to display above
  //  for( int n = 0; n < numPages; ++n ) gotoPage[n] = true;// normal state

    if( !Target_LG.pLt0 ) return false;
    // draw from page 0
    Light temp = bVal_0A ? Light(0,200,0) : Light(200,0,0);// green or red
    for( int n = (iVal_0A/32)*32; n < iVal_0A; ++n )
    {
        Target_LG.pLt0[n] = temp;
    }
    temp = bVal_0B ? Light(0,0,200) : Light(200,0,0);// blue or red
    for( int n = (iVal_0B/32)*32; n < iVal_0B; ++n )
    {
        Target_LG.pLt0[n] = temp;
    }

    // from page 1
    temp = bVal_1A ? Light(0,200,0) : Light(200,0,0);// green or red
    for( int n = (iVal_1A/32)*32; n < iVal_1A; ++n )// one line
    {
        Target_LG.pLt0[n] = temp;
    }
    temp = bVal_1B ? Light(0,0,200) : Light(200,0,0);// blue or red
    for( int n = (iVal_1B/32)*32; n < iVal_1B; ++n )
    {
        Target_LG.pLt0[n] = temp;
    }

    // from page 2
    temp = bVal_2A ? Light(0,200,0) : Light(200,0,0);// green or red
    for( int n = (iVal_2A/32)*32; n < iVal_2A; ++n )// one line
    {
        Target_LG.pLt0[n] = temp;
    }
    temp = bVal_2B ? Light(0,0,200) : Light(200,0,0);// blue or red
    for( int n = (iVal_2B/32)*32; n < iVal_2B; ++n )
    {
        Target_LG.pLt0[n] = temp;
    }

    return true;
}

void AL_BasicB::draw()const
{
    if( !Target_LG.pLt0 ) return;
 //   int numLts = Target_LG.rows*Target_LG.cols;

}

bool AL_BasicB::handleEvent( ArduinoEvent& AE )// change window position
{       
  //  return Page[2].handleEvent( AE );
    if( !MMP.handleEvent( AE ) ) return false;
    if( DoUpdateOled )
    {
        DoUpdateOled = false;
        updateDisplay();
    }

    return true;
}

void AL_BasicB::updateDisplay()const
{
    if( !pDisplay ) return;// crash avoidance
 //   String msg = Page[2].draw();
    String msg = MMP.draw();
    for( int n = 0; n < numPages; ++n )
        if( !gotoPage[n] )
        {
            msg += "\n Go to page ";
            msg += n;
        }

    // write
    pDisplay->clear();
    pDisplay->printAt( 0, 0, msg.c_str(), 1 );
    pDisplay->show();
}