#include "AL_BasicA.h"

bool AL_BasicA::setup( SSD1306_Display* p_Display )
{
    row0 = 0;
    col0 = 0;
    tFrame = 2.0f;
    hiLt.setRGB( 200, 0, 100 );
    loLt.setRGB( 0, 100, 200 );
    drawHiLt = true;
    tElap = 0.0f;

    pDisplay = p_Display;
    menuIter = 0;
    updateDisplay();
    
    return true;
}

bool AL_BasicA::update( float dt )
{
    tElap += dt;
    if( tElap > tFrame )
    {
        tElap = 0.0f;
        drawHiLt = !drawHiLt;
        return true;
    }    

    return false;
}

void AL_BasicA::draw()const
{
    if( !Target_LG.pLt0 ) return;
    int numLts = Target_LG.rows*Target_LG.cols;

    Light tempLt = drawHiLt ? hiLt : loLt;
    for( int r = 0; r < rows; ++r )
    {
        int Nrow = ( r + row0 )*Target_LG.cols;
        for( int c = 0; c < cols; ++ c )
        {
            int N = Nrow + col0 + c;
            if( N < 0 ) continue;
            if( N >= numLts ) break;
            Target_LG.pLt0[N] = tempLt;
        }
    }
}

bool AL_BasicA::handleEvent( ArduinoEvent& rEvent )
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
                tFrame = 0.2f;
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
                tFrame = 1.0f;
            }
        }
    }

    return true;
}

void AL_BasicA::updateDisplay()const
{
    if( !pDisplay ) return;// crash avoidance
    String msg( "Blinker" );
    msg += ( menuIter == 0 ) ? "\n *" : "\n  ";
    msg += "Press for Turbo";
    // Quit
    msg += ( menuIter == 1 ) ? "\n *" : "\n  ";
    msg += "QUIT to menu";
    // write
    pDisplay->clear();
    pDisplay->printAt( 2, 2, msg.c_str(), 1 );
    pDisplay->show();
}