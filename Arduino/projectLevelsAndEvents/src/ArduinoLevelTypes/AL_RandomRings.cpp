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

    // menu lines
    /*
    // line 0
    fl_fadeRratio.setupBase( menuIter, "fadeRratio: " ) ;
    fl_fadeRratio.setupFloat( fadeRratio, 0.2f, 5.0f );
    fl_fadeRratio.inScale = 0.05f;
    fl_fadeRratio.pNextLine = &fl_fadeWratio;
    // line 1  fadeWratio
    fl_fadeWratio.setupBase( menuIter, "fadeWratio: " ) ;
    fl_fadeWratio.setupFloat( fadeWratio, 0.2f, 5.0f );
    fl_fadeWratio.inScale = 0.05f;
    fl_fadeWratio.pNextLine = &fl_spawnTime;
    // line 2  spawnTime
    fl_spawnTime.setupBase( menuIter, "spawnTime: " ) ;
    fl_spawnTime.setupFloat( spawnTime, 0.1f, 5.0f );
    fl_spawnTime.inScale = 0.05f;
    fl_spawnTime.pNextLine = &fl_speedFactor;
    // line 3  speedFactor
    fl_speedFactor.setupBase( menuIter, "speedFactor: " ) ;
    fl_speedFactor.setupFloat( speedFactor, 0.1f, 5.0f );
    fl_speedFactor.inScale = 0.01f;
    fl_speedFactor.pNextLine = &IL_spawnBound;// to IntLine when it exists
    // line 4 int spawnBound
    IL_spawnBound.setupBase( menuIter, "spawnBound: " ) ;
    IL_spawnBound.setupInt( spawnBound, 8, 60 );
    IL_spawnBound.pNextLine = &ML_testLamp;
    // line 5 test lamp
    ML_testLamp.setupBase( menuIter, "Test Lamp: " ) ;
    ML_testLamp.pDoAct = &doTestLamp;
    ML_testLamp.pNextLine = nullptr;
    */

    // all in 1 menu page
    thePage.setup( " ** Random Rings **", 6 );
    // line 0: float fadeRratio
    thePage.addFloatLine( "fadeRratio: ", fadeRratio, 0.2f, 5.0f, 0.05f, nullptr );
    // line 1: float fadeWratio
    thePage.addFloatLine( "fadeWratio: ", fadeWratio, 0.2f, 5.0f, 0.05f, nullptr );
    // line 2: float spawnTime
    thePage.addFloatLine( "spawnTime: ", spawnTime, 0.1f, 5.0f, 0.05f, nullptr );
    // line 3: float speedFactor
    thePage.addFloatLine( "speedFactor: ", speedFactor, 0.1f, 5.0f, 0.01f, nullptr );
    // line 4: int spawnBound
    thePage.addIntegerLine( "spawnBound: ", spawnBound, 8, 60, nullptr );
    // line 5: base type
    thePage.addBaseLine( "Test Lamp: ", &doTestLamp );

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

    // light a test lamp
    if( doTestLamp )
    {
        Target_LG.pLt0[4].setRGB(255,0,0);
        Target_LG.pLt0[5].setRGB(0,255,0);
        Target_LG.pLt0[6].setRGB(0,0,255);
    }

    return true;// if image has moved or changed?
}

void AL_RandomRings::draw()const
{
    if( !Target_LG.pLt0 ) return;
    // draw stuff
}

bool AL_RandomRings::handleEvent( ArduinoEvent& AE )// change window position
{
    // handle Quit up front
 //   if( AE.type == -1 && (AE.ID == actButtID) &&  (menuIter == numOptions - 1) )
 //       return false;// Quit

    // handle menu scroll
  //  if( AE.type == 1 && AE.ID == menuButtID )
  //  {
  //      menuIter = ( 1 + menuIter )%numOptions;
  //      updateDisplay();
  //      return true;
 //   }

    if( thePage.handleEvent( AE ) )
    {
        updateDisplay();
        return true;
    }
    else return false;
    
    return true;
}

void AL_RandomRings::updateDisplay()const
{
    if( !pDisplay ) return;// crash avoidance
  //  String msg( " ** Random Rings **" );
    String msg = thePage.draw();// 5 lines in 1

    // Quit
  //  msg += ( menuIter == numOptions - 1 ) ? "\n *" : "\n  ";
 //   msg += "QUIT to menu";

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