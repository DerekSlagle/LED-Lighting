#include "ArdLvlScrolling.h"

// regular members
ArdLvlScrolling::ArdLvlScrolling()
{
    //ctor
}

ArdLvlScrolling::~ArdLvlScrolling()
{
    if( pAni_LG )
    {
        if( numAni_LG == 1 ) delete pAni_LG;
        else delete [] pAni_LG;
    }
}

// must call bindToGrid() first. This does not write to Target_LG.pLt0 Light array
// but the values of rows and cols must be right
// also assign clearLt
bool ArdLvlScrolling::setup( const char* setupFilename, SSD1306_Display* p_Display )
{
    // validate the target grid
    if( !Target_LG.pLt0 ) return false;
    if( Target_LG.rows <= 0 || Target_LG.cols <= 0 ) return false;

    // parse data from file or other
    FileParser fin( setupFilename );

 //   clearLt.setRGB( 0, 0, 128 );
    if( fromFile_ColorsOnly( AniBuff_Src, fin, pAni_LG, numAni_LG, Target_LG, clearLt ) )
    {
        // 2 values after the list of file names
        float SpeedX = 0.0f, SpeedY = 0.0f;
        fin >> SpeedX >> SpeedY;// now scrollVelFinal.mag() and scrollHangTime
        AniBuff_Src.setupScroll( SpeedX, SpeedY );

      // new for display
        pDisplay = p_Display;
        menuIter = 0;
        updateDisplay();

        return true;
    }

    return true;
}

bool ArdLvlScrolling::update( float dt )
{
    if( run_update || step_update )
    {
        step_update = false;
        AniBuff_Src.updateScroll(dt);
        return true;// image has moved
    }

    return false;
}

void ArdLvlScrolling::draw()const
{
    if( !Target_LG.pLt0 ) return;
    AniBuff_Src.draw();
}

bool ArdLvlScrolling::handleEvent( ArduinoEvent& rEvent )// change window position
{
    // handle Quit up front
    if( rEvent.type == -1 && (rEvent.ID == actButtID) && (menuIter == numOptions - 1) )
        return false;// Quit

    // return will be true from here
    if( rEvent.type == 1 )// button press
    {
        if( rEvent.ID == actButtID )
        {
            if( menuIter == 0 )// scroll horizontally
            {
                AniBuff_Src.vel.y = 0.0f;
                AniBuff_Src.rShift = 0;
                AniBuff_Src.pos.y = 0.0f;
            }
            else if( menuIter == 1 )// scroll vertically
            {
                AniBuff_Src.vel.x = 0.0f;
                AniBuff_Src.cShift = 0;
                AniBuff_Src.pos.x = 0.0f;
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
                // no action
            }
        }
    }
    else if( rEvent.type == 2 )// rotary encoder
    {   // which encoder
        if( rEvent.ID == 1 )// just 1 encoder?
        {
            if( menuIter == 0 )// change Vx
            {
                AniBuff_Src.vel.x += speedScale*rEvent.value;// up or down
                updateDisplay();
            }
            else if( menuIter == 1 )// change Vy
            {
                AniBuff_Src.vel.y += speedScale*rEvent.value;
                updateDisplay();
            }
        }
    }

    return true;
}

void ArdLvlScrolling::updateDisplay()const
{
    if( !pDisplay ) return;// crash avoidance
    String msg( "Scroller" );
 //   std::string msgF( AniBuff_Src.vel.x );
    msg += ( menuIter == 0 ) ? "\n *" : "\n  ";
    msg += "Adjust Vx: ";
    msg += AniBuff_Src.vel.x;
    msg += ( menuIter == 1 ) ? "\n *" : "\n  ";
    msg += "Adjust Vy: ";
    msg += AniBuff_Src.vel.y;
    // Quit
    msg += ( menuIter == numOptions - 1 ) ? "\n *" : "\n  ";
    msg += "QUIT to menu";
    // write
    pDisplay->clear();
    pDisplay->printAt( 2, 2, msg.c_str(), 1 );
    pDisplay->show();
}