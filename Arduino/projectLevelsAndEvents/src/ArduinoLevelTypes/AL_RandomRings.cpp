#include "AL_RandomRings.h"

//const int AL_RandomRings::numRP = 30;

// must call bindToGrid() first. This does not write to Target_LG.pLt0 Light array
// but the values of rows and cols must be right
// also assign clearLt
bool AL_RandomRings::setup( SSD1306_Display* p_Display )
{
    // validate the target grid
    if( !Target_LG.pLt0 ) return false;
    if( Target_LG.rows <= 0 || Target_LG.cols <= 0 ) return false;

    // all members assigned procedurally. Not from file
  for( unsigned int n = 0; n < numRP; ++n )
    ringPlay_Arr[n].initToGrid( Target_LG.pLt0, Target_LG.rows, Target_LG.cols );
 
    // new for display
    pDisplay = p_Display;
    menuIter = 0;
    updateDisplay();

    return true;
}

bool AL_RandomRings::update( float dt )
{
    if( !Target_LG.pLt0 ) return false;
    // clear entire array
    Target_LG.Clear( clearLt );

    numRPplaying = 0;
    // time across the ring updates
    uint32_t startTime = micros();
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

    updateTime = (  micros() - startTime + 500 )/1000;// to msec
    tElapUp += dt;
    if( tElapUp > 1.0f )
    {
        updateDisplay();
        tElapUp = 0.0f;
    }

    // Start another
    tElapStart += dt;
    if( tElapStart >= tStart )
    {
        tElapStart = 0.0f;
        RingPlayer& RP = ringPlay_Arr[ idxStartNext ];
    
    //    int rC = rand()%48 - 8, cC = rand()%48 - 8;
        int rC = rand()%spawnBound - ( spawnBound - Target_LG.rows )/2;
        int cC = rand()%spawnBound - ( spawnBound - Target_LG.cols )/2;
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

    return true;// if image has moved or changed?
}

void AL_RandomRings::draw()const
{
    if( !Target_LG.pLt0 ) return;
    // draw stuff
}

bool AL_RandomRings::handleEvent( ArduinoEvent& rEvent )// change window position
{
    // handle Quit up front
    if( rEvent.type == -1 && (rEvent.ID == actButtID) && (menuIter == numOptions - 1) )
        return false;// Quit

    // handle menu scroll
    if( rEvent.type == 1 && rEvent.ID == menuButtID )
    {
        menuIter = ( 1 + menuIter )%numOptions;
        updateDisplay();
        return true;
    }

    // return will be true from here
    if( rEvent.type == 1 )// button press
    {
        // zzz
    }
    else if( rEvent.type == -1 )// button release
    {
        // Zzz
    }
    else if( rEvent.type == 2 )// rotary encoder
    {
        if( rEvent.ID == rotEncID )
        {
            if( menuIter == 0 )
            {
                fadeRratio += 0.05f*rEvent.value;
                if( fadeRratio < 0.2f ) fadeRratio = 0.2f;
                updateDisplay();
            }
            else if( menuIter == 1 )
            {
                fadeWratio += 0.05f*rEvent.value;
                if( fadeWratio < 0.2f ) fadeWratio = 0.2f;
                updateDisplay();
            }
            else if( menuIter == 2 )
            {
                int dblRate = 2.0f/spawnTime;
                dblRate += rEvent.value;
                // clamp
                if( dblRate < 1 ) dblRate = 1;
                else if( dblRate > 40 ) dblRate = 40;
                spawnTime = 2.0f/dblRate;

                updateDisplay();
            }
            else if( menuIter == 3 )// spawnBound
            {
                if( rEvent.value > 0 ) ++spawnBound;
                else --spawnBound;

                if( spawnBound < 8 ) spawnBound = 8;
                updateDisplay();
            }
            else if( menuIter == 4 )
            {
                speedFactor += 0.01f*rEvent.value;
                if( speedFactor < 0.1f ) speedFactor = 0.1f;
                updateDisplay();
            }
        }
    }

    return true;
}

void AL_RandomRings::updateDisplay()const
{
    if( !pDisplay ) return;// crash avoidance
    String msg( " ** Random Rings **" );
    msg += ( menuIter == 0 ) ? "\n *" : "\n  ";
    msg += "fadeRratio: ";
    msg += fadeRratio;
    msg += ( menuIter == 1 ) ? "\n *" : "\n  ";
    msg += "fadeWratio: ";
    msg += fadeWratio;
    msg += ( menuIter == 2 ) ? "\n *" : "\n  ";
    msg += "spawnRate: ";
    int spawnRate = 1.0f/spawnTime;
    msg += spawnRate;
    msg += ( menuIter == 3 ) ? "\n *" : "\n  ";
    msg += "spawnBound: ";
    msg += spawnBound;
    msg += ( menuIter == 4 ) ? "\n *" : "\n  ";
    msg += "speedFactor: ";
    msg += speedFactor;

    // Quit
    msg += ( menuIter == numOptions - 1 ) ? "\n *" : "\n  ";
    msg += "QUIT to menu";
    // extra line updates once per second
    msg += "\nRings: ";
    msg += numRPplaying;
    msg += " UpTime: ";
    msg += updateTime;

    // write
    pDisplay->clear();
    pDisplay->printAt( 0, 0, msg.c_str(), 1 );
    pDisplay->show();
}