#include "AL_BasicA.h"

bool AL_BasicA::setup( SSD1306_Display* p_Display )
{
    

    Light LtColor[4];
    LtColor[0].setRGB(200,0,100);
    LtColor[1].setRGB(0,200,40);
    LtColor[2].setRGB(100,0,200);
    LtColor[3].setRGB(80,100,60);

    // A 16 point circle for PP[0] path
    int Rc = gridRows/2, Cc = gridCols/2;
    float Radius = Rc - 2.0f;
    for( int k = 0; k < nPtEach; ++k )
    {
        PathX[k] = Cc + Radius*sinf( 0.3927f*k );// pi/8
        PathY[k] = Rc - Radius*cosf( 0.3927f*k );
    }
    PP[0].bindToGrid( pLt0, gridRows, gridCols );
    PP[0].setup( PathX[0], PathY[0], nPtEach, 40.0f, LtColor[0] );

    for( int n = 1; n < numPlayers; ++n )
    {
      //  for( int j = 0; j < 4; ++j )
      //  {
      //      PathX[4*n+j] = PathX[j];// start as same
      //      PathY[4*n+j] = PathY[j];// start as same
      //  }

        PathX[nPtEach*n] = 2; PathY[nPtEach*n] = 2;
        PathX[nPtEach*n+1] = gridCols/2; PathY[nPtEach*n+1] = 2;
        PathX[nPtEach*n+2] = gridCols/2; PathY[nPtEach*n+2] = gridRows/2;
        PathX[nPtEach*n+3] = gridCols/2; PathY[nPtEach*n+3] = gridRows - 3;
        for( int j = 4; j < nPtEach; ++j )// safe off
        {   // back and forth near the top
             PathX[nPtEach*n+j] = ( j%2 == 1 ) ? 2 : gridCols - 3;
             PathY[nPtEach*n+j] = 2;
        }

        PP[n].bindToGrid( pLt0, gridRows, gridCols );
        PP[n].setup( PathX[nPtEach*n], PathY[nPtEach*n], nPtEach, 40.0f, LtColor[n] );
     //   PP[n].isPlaying = true;
    }
    
        
    clearLt.setRGB( 0, 0, 0 );
    
    pDisplay = p_Display;
    setupMenu();   
    updateDisplay();
    
    return true;
}

void AL_BasicA::setupMenu()
{
    // Page[0]
    FLa_Speed.setupBase( "Speed a: ", nullptr, &FLa_FadeLength );
    FLa_Speed.setupFloat( PP[0].speed, 1.0f, 100.0f );
    FLa_FadeLength.setupBase( "Fade a: ", nullptr, &FLb_Speed );
    FLa_FadeLength.setupFloat( PP[0].fadeLength, 2.0f, 60.0f );
    // make work for 1,2 and 3
    FLb_Speed.setupBase( "Speed b: ", &doUpSpeed, &FLb_FadeLength );
    FLb_Speed.setupFloat( PP[1].speed, 1.0f, 100.0f );
    FLb_FadeLength.setupBase( "Fade b: ", &doUpLength, &ML_More );
    FLb_FadeLength.setupFloat( PP[1].fadeLength, 2.0f, 60.0f );
    ML_More.setupBase( "More...", gotoPage + 1, &ML_Quit );// goto Page[1]
    ML_Quit.setupBase( "Quit" );
    Page[0].setup( " -- Zoomie --", DoUpdateOled, FLa_Speed );

    // Page[1] startAll, startAll 2, clearLt colors
    ML_StartAll.setupBase( "StartAll ", &doStartAll, &ML_StartPairs );
    ML_StartPairs.setupBase( "StartX2 ", &doStartPairs, &UL_Red );
    UL_Red.setupBase( "Red: ", nullptr, &UL_Green );
    UL_Red.setupUint8_t( clearLt.r, 0, 255 );
    UL_Green.setupBase( "Green: ", nullptr, &UL_Blue );
    UL_Green.setupUint8_t( clearLt.g, 0, 255 );
    UL_Blue.setupBase( "Blue: ", nullptr, &ML_Back );
    UL_Blue.setupUint8_t( clearLt.b, 0, 255 );

    ML_Back.setupBase( "Back", gotoPage, nullptr );// go home
    Page[1].setup( " ~ More ~", DoUpdateOled, ML_StartAll );
    MMP.setup( Page, gotoPage, 2 );
}

bool AL_BasicA::update( float dt )
{    
    const float minDistSq = 64.0f;// minimum distance between points = 8
    int nLimit = showFirstOnly ? 1 : numPlayers;
    for( int n = 0; n < nLimit; ++n )
    {
        uint8_t cp = PP[n].currPoint;
        PP[n].update(dt);
        if( n == 0 ) continue;// no path mod for PP[0]

        if( PP[n].currPoint != cp )
        {
            // assign 2 ahead. nextPoint is already in use
            uint8_t cp1 = ( PP[n].currPoint + 1 )%PP[n].numPoints;
            uint8_t cp2 = ( cp1 + 1 )%PP[n].numPoints;
            int LsQ = 0;// dist squared from cp1 to cp2
            numTries = 0;
            do
            {
                PP[n].pathX[ cp2 ] = 2 + rand()%( gridCols - 3 );
                PP[n].pathY[ cp2 ] = 2 + rand()%( gridRows - 3 );
                int dx = PP[n].pathX[ cp2 ]; dx -= PP[n].pathX[ cp1 ];
                int dy = PP[n].pathY[ cp2 ]; dy -= PP[n].pathY[ cp1 ];
                LsQ = dx*dx + dy*dy;
                ++numTries;
            } while ( LsQ < minDistSq );    
            
            if( n == 0 ) updateDisplay();
        }
    }    
    
    return true;
}

void AL_BasicA::draw()const
{
    if( !pLt0 ) return;
    for( int n = 0; n < gridRows*gridCols; ++n )
        pLt0[n] = clearLt;
    
    int nLimit = showFirstOnly ? 1 : numPlayers;
    for( int n = 0; n < nLimit; ++n )
        PP[n].draw();
}

bool AL_BasicA::handleEvent( ArduinoEvent& AE )
{    
  //  if( !thePage.handleEvent( AE ) ) return false;
    if( !MMP.handleEvent( AE ) ) return false;
    if( DoUpdateOled )
    {
        DoUpdateOled = false;
        updateDisplay();
    }

    const MenuLine* pCurrLn = MMP.pPage[ MMP.currPage ].pCurrLine;
    // assign new vel right away
    if( AE.ID == FLa_Speed.rotEncID && AE.type == 2 )
    {
        if( pCurrLn == &FLa_Speed ) PP[0].vel = PP[0].uCurr*PP[0].speed;
        else if( pCurrLn == &FLb_Speed ) PP[1].vel = PP[1].uCurr*PP[1].speed;
    }

    // actButt pressed
    if( pCurrLn == Page[0].pLine && AE.ID == FLa_Speed.actButtID && AE.type == 1 )// 1st line = speed
    {
        showFirstOnly = !showFirstOnly;// toggle
    }

    if( doStartAll )
    {
        doStartAll = false;
        showFirstOnly = false;
        uint8_t startX = ( rand()%2 == 1 ) ? gridCols - 3 : 2;
        uint8_t startY = ( rand()%2 == 1 ) ? gridRows - 3 : 2;
        for( int n = 1; n < numPlayers; ++n )
        {
            // random start corner
            PathX[nPtEach*n] = startX; 
            PathY[nPtEach*n] = startY;
            PathX[nPtEach*n+1] = gridCols/2; PathY[nPtEach*n+1] = 2;
            PathX[nPtEach*n+2] = gridCols/2; PathY[nPtEach*n+2] = gridRows/2;
            PathX[nPtEach*n+3] = gridCols/2; PathY[nPtEach*n+3] = gridRows - 3;
            for( int j = 4; j < nPtEach; ++j )// safe off
            {   // back and forth near the top
                PathX[nPtEach*n+j] = ( j%2 == 1 ) ? 2 : gridCols - 3;
                PathY[nPtEach*n+j] = 2;
            }

            PP[n].Start();
        }
    }
    else if( doStartPairs )// 1 triplet
    {
        doStartPairs = false;
        showFirstOnly = false;
        // one pair from each bottom corner
        uint8_t stXa = ( rand()%2 == 1 ) ? gridCols - 3 : 2;// 29 or 2 for PP 0,1        
        // start point: for a: 0,1
      //  PathX[0] = stXa; PathY[0] = gridRows - 3;// 1st point for PP[0]
        PathX[nPtEach] = stXa; PathY[nPtEach] = gridRows - 3;// 1st point for PP[1]
        // for b: 2,3
    //    uint8_t stXb = ( stXa == 2 ) ? gridCols - 3 : 2;// 29 or 2  for PP 2,3
        PathX[2*nPtEach] = stXa; PathY[2*nPtEach] = gridRows - 3;// 1st point for PP[2]
        PathX[3*nPtEach] = stXa; PathY[3*nPtEach] = gridRows - 3;// 1st point for PP[3]
        // 2nd point = middle of grid = same for all
        for( int n = 1; n < numPlayers; ++n )// numPlayers
        {
            PathX[nPtEach*n+1] = gridCols/2;
            PathY[nPtEach*n+1] = gridRows/2;
        }
        // 3rd point to middle of opposing quadrant: y = 8 = gridRows/4
        uint8_t endXa = ( stXa == 2 ) ? gridCols - 3 : 2;
        // end point: for a: 0,1
  //      PathX[2] = endXa; PathY[2] = gridRows/4;// 3rd point for PP[0]
        PathX[2+nPtEach] = endXa; PathY[2+nPtEach] = gridRows/4;// 3rd point for PP[1]
        // end point: for b: 2,3
    //    uint8_t endXb = ( endXa == 2 ) ? gridCols - 3 : 2;
        PathX[2*nPtEach+2] = endXa; PathY[2*nPtEach+2] = gridRows/4;// 3rd point for PP[2]
        PathX[3*nPtEach+2] = endXa; PathY[3*nPtEach+2] = gridRows/4;// 3rd point for PP[3]

        // for Safety: assign last points to up left so last leg length != 0
        for( int n = 1; n < numPlayers; ++n )
        {
            for( int j = 3; j < nPtEach; ++j )
            {   // alternate ends
                PathX[nPtEach*n+j] = ( j%2 == 1 ) ? 2 : gridCols - 3;
                PathY[nPtEach*n+j] = 2;// near the top
            }
            PP[n].Start();
        }
    }
    else if( doUpSpeed )
    {
        doUpSpeed = false;
     //   PP[1].vel = PP[1].uCurr*PP[1].speed;// already done above
        for( int n = 2; n < numPlayers; ++n )// match PP[1]
        {       
            PP[n].speed = PP[1].speed;
            PP[n].vel = PP[n].uCurr*PP[n].speed;
        }
    }
    else if( doUpLength )
    {
        doUpLength = false;
        for( int n = 2; n < 4; ++n )// match PP[1]        
            PP[n].fadeLength = PP[1].fadeLength;        
    }

    return true;
}

void AL_BasicA::updateDisplay()const
{
    if( !pDisplay ) return;// crash avoidance
    String msg = MMP.draw();
    // write
    msg += "  ";
    msg += numTries;
    pDisplay->clear();
    pDisplay->printAt( 0, 0, msg.c_str(), 1 );
    pDisplay->show();
}