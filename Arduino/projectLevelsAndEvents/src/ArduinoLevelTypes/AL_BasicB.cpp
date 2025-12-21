#include "AL_BasicB.h"

bool AL_BasicB::setup( SSD1306_Display* p_Display )
{
    row0 = 0;
    col0 = 0;
    tFrame = 0.4f;

    for( int n = 0; n < 4; ++n )
        quadLt[n].setRGB( 70*n, 60*(3-n), 60 + 50*n );

    iterLt = 0;
    tElap = 0.0f;
    // display
    pDisplay = p_Display;
    menuIter = 0;
    updateDisplay();

    return true;
}

bool AL_BasicB::update( float dt )
{
    tElap += dt;
    if( tElap > tFrame )
    {
        tElap = 0.0f;
        iterLt = ( 1 + iterLt )%numQLt;
        return true;
    }
    return false;
}

void AL_BasicB::draw()const
{
    if( !Target_LG.pLt0 ) return;
    int numLts = Target_LG.rows*Target_LG.cols;

    for( int r = 0; r < 16; ++r )
    {
        int Nrow = ( r + row0 )*Target_LG.cols;
        for( int c = 0; c < 8; ++ c )
        {
            int N = Nrow + col0 + c;
            if( N < 0 ) continue;
            if( N >= numLts ) break;
            int it = ( iterLt + c/2 )%numQLt;// 2 columns in each color
            Target_LG.pLt0[N] = quadLt[ it ];
        }
    }
}

bool AL_BasicB::handleEvent( ArduinoEvent& rEvent )// change window position
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
                col0 += 4;
                if( col0 + cols >= Target_LG.cols )
                {
                    col0 -= Target_LG.cols;// back to left
                }
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
                row0 += 2;
                if( row0 + rows >= Target_LG.rows )
                {
                    row0 -= Target_LG.rows;// back to left
                }
            }
        }
    }

    return true;
}

void AL_BasicB::updateDisplay()const
{
    if( !pDisplay ) return;// crash avoidance
    String msg( "Shifter" );
    msg += ( menuIter == 0 ) ? "\n *" : "\n  ";
    msg += "Shift cols+rows";
    // Quit
    msg += ( menuIter == 1 ) ? "\n *" : "\n  ";
    msg += "QUIT to menu";
    // write
    pDisplay->clear();
    pDisplay->printAt( 2, 2, msg.c_str(), 1 );
    pDisplay->show();
}