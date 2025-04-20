#include "lvl_Lights.h"

lvl_Lights::lvl_Lights()
{
    //ctor
}

lvl_Lights::~lvl_Lights()
{
    //dtor
}

bool lvl_Lights::init()
{
    std::ifstream fin("include/levels/lvl_Lights/init_data.txt");
    if( !fin ) return false;

    float posX, posY;
    fin >> posX >> posY;
    Level::quitButt.setPosition( sf::Vector2f( posX, posY ) );
    button::RegisteredButtVec.push_back( &Level::quitButt );
    fin >> posX >> posY;
    Level::goto_MMButt.setPosition( sf::Vector2f( posX, posY ) );
    button::RegisteredButtVec.push_back( &Level::goto_MMButt );

    unsigned int rd, gn, bu;
    fin >> rd >> gn >> bu;
    Level::clearColor = sf::Color(rd,gn,bu);
    button::setHoverBoxColor( Level::clearColor );

 //   unsigned int Rows, Cols;
    fin >> stepDelay;
//    LightArr.resize( ArrSz );

 //   fin >> LtPlay.numBlinks >> LtPlay.blinkPause;
 //   fin >> LtPlay.k1 >> LtPlay.k2;
 //   LtPlay.init( LightArr[0], Rows, Cols );
//    std::cout << "\nlvl_Lights::init() LtPlay.iterMax = " << LtPlay.getIterMax();
//    uint8_t rd, gn, bu;
    fin >> rd >> gn >> bu;
    onLight.init( rd, gn, bu );
    fin >> rd >> gn >> bu;
    offLight.init( rd, gn, bu );

 //   float dPosX, dPosY;
 //   fin >> posX >> posY >> dPosX >> dPosY;
 //   std::cout << "\nlvl_Lights::init(): posX = " << posX;// check

 //   sf::Vector2f LtSz;
 //   fin >> LtSz.x >> LtSz.y;

//    LtPlayLV.init( LightArr[0], 1, ArrSz, posX, posY, dPosX, dPosY, LtSz );
 //   LtPlayLV.init( LightArr[0], Rows, Cols, posX, posY, dPosX, dPosY, LtSz );
 //   LtPlay.update( onLight, offLight );
 //   LtPlayLV.update();

    std::string fileName;
    fin >> fileName;
    if( !initPlayer2( fileName.c_str() ) ) return false;

    fin >> fileName;
    if( !initPlayer4( fileName.c_str() ) ) return false;

    fin >> fileName;
    if( !initDataPlay( fileName.c_str() ) ) return false;
 //   DataPlay.flipY = true;

    fin >> fileName;
    if( !init_wvPlay( fileName.c_str() ) ) return false;

    fin >> fileName;
    if( !initPulse( fileName.c_str() ) ) return false;

    fin >> fileName;
    if( !initMovers( fileName.c_str() ) ) return false;

    fin >> fileName;
    if( !init_controls( fileName.c_str() ) ) return false;

    return true;
}

bool lvl_Lights::initPlayer2( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\ninitPlayer2() no file"; return false; }

    unsigned int Rows, Cols;
    fin >> Rows >> Cols;
    unsigned int ArrSz2 = Rows*Cols;
//    fin >> ArrSz >> stepDelay;
    LightArr2.resize( ArrSz2 );

    float posX, posY, dPosX, dPosY;
    fin >> posX >> posY >> dPosX >> dPosY;
    sf::Vector2f LtSz;
    fin >> LtSz.x >> LtSz.y;
    LtPlay2LV.init( LightArr2[0], Rows, Cols, posX, posY, dPosX, dPosY, LtSz );

    unsigned int numPatterns;
    fin >> numPatterns;
    pattDataVec.reserve( numPatterns );

    patternData PD;
    unsigned int fI, sP, par;
    for( unsigned int k = 0; k < numPatterns; ++k )
    {
    //    fin >> PD.funcIndex >> PD.stepPause >> PD.param;
        fin >> fI >> sP >> par;
        PD.init( fI, sP, par );
        pattDataVec.push_back( PD );
    }

    LtPlay2.init( LightArr2[0], Rows, Cols, pattDataVec[0], numPatterns );
    LtPlay2.onLt = onLight;
    LtPlay2.offLt = offLight;
    LtPlay2.drawOffLt = false;

    // present initial state
    LtPlay2.update();
    LtPlay2LV.update();

    return true;
}

bool lvl_Lights::initPlayer4( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\ninitPlayer4() no file"; return false; }

    unsigned int Rows, Cols;
    fin >> Rows >> Cols;
    unsigned int ArrSz4 = Rows*Cols;
//    fin >> ArrSz >> stepDelay;
    LightArr4.resize( ArrSz4 );

    float posX, posY, dPosX, dPosY;
    fin >> posX >> posY >> dPosX >> dPosY;
    sf::Vector2f LtSz;
    fin >> LtSz.x >> LtSz.y;
    LtPlay4LV.init( LightArr4[0], Rows, Cols, posX, posY, dPosX, dPosY, LtSz );

    unsigned int rd, gn, bu;
    for( unsigned int k = 0; k < 4; ++k )
    {
        fin >> rd >> gn >> bu;
        LtPlay4.Lt[k].init(rd,gn,bu);
    }

    unsigned int numPatterns;
    fin >> numPatterns;
    pattDataVec4.reserve( numPatterns );

    patternData4 PD;
    for( unsigned int k = 0; k < numPatterns; ++k )
    {
        fin >> PD.funcIndex >> PD.stepPause >> PD.param;
        pattDataVec4.push_back( PD );
    }

    LtPlay4.init( LightArr4[0], Rows, Cols, pattDataVec4[0], numPatterns );

    // pattern 100 data
    unsigned int numBytes;
    fin >> numBytes;
    patt100Vec4.reserve( numBytes );
    unsigned int inVal = 0;
    for( unsigned int k = 0; k < numBytes; ++k )
    {
        fin >> inVal;
        patt100Vec4.push_back( inVal );
    }

    LtPlay4.setStateData( &( patt100Vec4[0] ), patt100Vec4.size() );

    // present initial state
    LtPlay4.update();
    LtPlay4LV.update();

    return true;
}
// draws within LtPlay4
bool lvl_Lights::initDataPlay( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\n initDataPlay() no file"; return false; }
    // init the LV
    unsigned int Rows, Cols;
    fin >> Rows >> Cols;
    unsigned int ArrSz = Rows*Cols;
//    fin >> ArrSz >> stepDelay;
    DataLightArr.resize( ArrSz );

    float posX, posY, dPosX, dPosY;
    fin >> posX >> posY >> dPosX >> dPosY;
    sf::Vector2f LtSz;
    fin >> LtSz.x >> LtSz.y;
    DataPlayLV.init( DataLightArr[0], Rows, Cols, posX, posY, dPosX, dPosY, LtSz );

    DataPlay.pLight = DataPlay_Color;
    if( !initDataPlayer( DataPlay, DataLightArr[0], Rows, Cols, DataVec, fin ) )
    {
        std::cerr << "\ninitDataPlayer() fail";
        return false;
    }

    DataPlay.update();
    DataPlayLV.update();

    return true;
}

bool lvl_Lights::init_wvPlay( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\ninit_wvPlay() no file"; return false; }

    unsigned int Rows, Cols;
    fin >> Rows >> Cols;
    unsigned int ArrSz = Rows*Cols;
//    fin >> ArrSz >> stepDelay;
    LightArrWave.resize( ArrSz );

    float posX, posY, dPosX, dPosY;
    fin >> posX >> posY >> dPosX >> dPosY;
    sf::Vector2f LtSz;
    fin >> LtSz.x >> LtSz.y;
    wvPlayLV.init( LightArrWave[0], Rows, Cols, posX, posY, dPosX, dPosY, LtSz );

    unsigned int rd, gn, bu;
    fin >> rd >> gn >> bu;
    Light HiLt(rd,gn,bu);
    fin >> rd >> gn >> bu;
    Light LoLt(rd,gn,bu);

    wvPlay.init( LightArrWave[0], Rows, Cols, HiLt, LoLt );

    // wave data
    float A_lt, wvLen_lt, wvSpd_lt;
    float wvLen_rt, wvSpd_rt;
    fin >> A_lt;
    fin >> wvLen_rt >> wvSpd_rt;
    fin >> wvLen_lt >> wvSpd_lt;
    wvPlay.setWaveData( A_lt, wvLen_lt, wvSpd_lt, wvLen_rt, wvSpd_rt );

 //   return true;

    // Fourier Coefficients
    // right going wave
    float* pC_Rt = nullptr;
    unsigned int numRt = 0;
    fin >> numRt;
    if( numRt > 0 )
    {
        wvC_Rt.resize( numRt );
        pC_Rt = &( wvC_Rt[0] );
        for( unsigned int k = 0; k < numRt; ++k )
            fin >> wvC_Rt[k];
    }

    // left going wave
    float* pC_Lt = nullptr;
    unsigned int numLt = 0;
    fin >> numLt;
    if( numLt > 0 )
    {
        wvC_Lt.resize( numLt );
        pC_Lt = &( wvC_Lt[0] );
        for( unsigned int k = 0; k < numLt; ++k )
            fin >> wvC_Lt[k];
    }

    wvPlay.setSeriesCoeffs( pC_Rt, numRt, pC_Lt, numLt );

    // initial state
    wvPlay.update( 0.0f );
    wvPlayLV.update();

    // using LtPlay2 as an overlay
    LtPlay2.init( LightArrWave[0], Rows, Cols, pattDataVec[0], pattDataVec.size() );

    return true;
}

bool lvl_Lights::initPulse( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\ninit_wvPlay() no file"; return false; }

//    return true;

    unsigned int Rows, Cols;
    fin >> Rows >> Cols;
    unsigned int ArrSz = Rows*Cols;
//    fin >> ArrSz >> stepDelay;
    LightArrPulse.resize( ArrSz );

    int W_pulse; fin >> W_pulse;
    float Speed; fin >> Speed;
    unsigned int FuncIdx; fin >> FuncIdx;

    unsigned int rd, gn, bu;
    fin >> rd >> gn >> bu;
    Light HiLt(rd,gn,bu);
    fin >> rd >> gn >> bu;
    Light LoLt(rd,gn,bu);

    pulsePlay.init( LightArrPulse[0], Rows, Cols, HiLt, LoLt, W_pulse, Speed, 2.0f, false );
    pulsePlay.funcIdx = FuncIdx;

    // a 2nd player
    fin >> W_pulse;
    fin >> Speed;
    fin >> FuncIdx;
    fin >> rd >> gn >> bu;
    Light HiLt2(rd,gn,bu);
    pulsePlay2.init( LightArrPulse[0], Rows, Cols, HiLt2, LoLt, W_pulse, Speed, 2.0f, false );
    pulsePlay2.funcIdx = FuncIdx;

    // the LightVisual for this Light array
    float posX, posY, dPosX, dPosY;
    fin >> posX >> posY >> dPosX >> dPosY;
    sf::Vector2f LtSz;
    fin >> LtSz.x >> LtSz.y;
    pulsePlayLV.init( LightArrPulse[0], Rows, Cols, posX, posY, dPosX, dPosY, LtSz );

    // initial state draw
    pulsePlay.update(0.0f);
    pulsePlayLV.update();

    // graph shape of pulse above pulse strip
    pulse_yVec.resize( ArrSz, 0.0f );
    fin >> graphAmp;
    float ofstY; fin >> ofstY;

    fin >> rd >> gn >> bu;// graph
    pulseGraph.resize( ArrSz + 1 );// line strip

//    sf::FloatRect pulseRect = pulsePlayLV.getRect();

    for( unsigned int n = 0; n < pulseGraph.size(); ++n )
    {
        pulseGraph[n].color = sf::Color(rd,gn,bu);
        pulseGraph[n].position = sf::Vector2f( posX + n*( LtSz.x + dPosX ), posY + ofstY );
    }

    fin >> rd >> gn >> bu;// axis
    graphAxis[0].color = graphAxis[1].color = sf::Color(rd,gn,bu);// at y = 0.0f
    graphAxis[0].position = sf::Vector2f( posX, posY + ofstY );
    graphAxis[1].position = sf::Vector2f( posX + ArrSz*( LtSz.x + dPosX ), posY + ofstY );

    return true;
}

bool lvl_Lights::initMovers( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\n init_Movers() no file"; return false; }

    unsigned int Rows, Cols;
    fin >> Rows >> Cols;
    unsigned int ArrSz = Rows*Cols;
//    fin >> ArrSz >> stepDelay;
    LightArrMove.resize( ArrSz );

    // visual
    float posX, posY, dPosX, dPosY;
    fin >> posX >> posY >> dPosX >> dPosY;
    sf::Vector2f LtSz;
    fin >> LtSz.x >> LtSz.y;
    LtMoverLV.init( LightArrMove[0], Rows, Cols, posX, posY, dPosX, dPosY, LtSz );

    LightMover LM;
    LM.init( LightArrMove[0], ArrSz );
    unsigned int rd=0, gn=0, bu=0;
    fin >> rd >> gn >> bu;
    moveOffLight.init(rd,gn,bu);

    unsigned int numMovers;
    fin >> numMovers;
    fin >> move_tShift;
    LtMoverVec.reserve( numMovers );
    for( unsigned int n = 0; n < numMovers; ++n )
    {
        fin >> LM.tLimit;
        LM.tElap = -move_tShift*n;// shift start times
        fin >> LM.funcIdx;
        fin >> rd >> gn >> bu;
        LM.onLt.init(rd,gn,bu);
        LtMoverVec.push_back(LM);
    }

    // initial state draw
    if( !LtMoverVec.empty() )
    {
        LtMoverVec[0].Clear( moveOffLight );
        LtMoverLV.update();
    }


    return true;
}

bool lvl_Lights::handleEvent( sf::Event& rEvent )
{
    if ( rEvent.type == sf::Event::KeyPressed )
    {
        // List: P
        if( rEvent.key.code == sf::Keyboard::P )
        {
            DataPlay.isPlaying = !DataPlay.isPlaying;
        }
        else if( rEvent.key.code == sf::Keyboard::R )// reset player
        {
            DataPlay.isPlaying = true;
            DataPlay.reStart();
        }
    }
    else if ( rEvent.type == sf::Event::KeyReleased )
    {
        if( rEvent.key.code == sf::Keyboard::Right )
        {
            if( !DataPlay.isPlaying ) DataPlay.nextImage();
        }
        else if( rEvent.key.code == sf::Keyboard::Left )
        {
            if( !DataPlay.isPlaying ) DataPlay.prevImage();
        }
    }

    return true;
}

void lvl_Lights::update( float dt )
{
    wvPlay.update(dt);
    if( pLtPlay2_Butt.sel ) LtPlay2.update();
    wvPlayLV.update();

    LtPlay4.update();
    LtPlay4LV.update();

    DataPlay.update();
 //   DataPlay.showColors();
    DataPlayLV.update();


    if( !LtMoverVec.empty() )
    {
        LtMoverVec[0].Clear( moveOffLight );
        for( LightMover& LM : LtMoverVec ) LM.update(dt);
        LtMoverLV.update();
    }


    // ani
    const float tElap_ani_Lt = ( idxLt + pulsePlay.hfW )/pulsePlay.speed;
    const float tElap_ani_Rt = ( idxRt + pulsePlay.hfW )/pulsePlay.speed;
    const float ani_Period  = tElap_ani_Rt - tElap_ani_Lt;
    float ani_u = tElap_ani/ani_Period;

    if( ani_rev )
    {
        tElap_ani -= dt;
        if( tElap_ani*pulsePlay.speed - pulsePlay.hfW < idxLt )
        {
       //     tElap_ani = ( idxLt + pulsePlay.hfW )/pulsePlay.speed;
            tElap_ani = tElap_ani_Lt;
            ani_rev = false;
            pulsePlay2.tElap = 0.0f;
        }
    }
    else
    {
        tElap_ani += dt;
        if( tElap_ani*pulsePlay.speed - pulsePlay.hfW > idxRt )
        {
     //       tElap_ani = ( idxRt + pulsePlay.hfW )/pulsePlay.speed;
            tElap_ani = tElap_ani_Rt;
            ani_rev = true;
        }
    }

    ani_u = ( tElap_ani - tElap_ani_Lt )/ani_Period;// sb 0 to 1
//    float ani_y = ani_u*ani_u*( 3.0f - 2.0f*ani_u );// cubic
    float ani_y = ani_u*( 2.0f - ani_u );// quadratic

    pulsePlay.tElap = tElap_ani_Lt + ani_y*ani_Period;

    pulsePlay.update( pulse_yVec, 0.0f );
    pulsePlay2.updateLeft( dt, false );

    pulsePlayLV.update();
    // pulse graph
    for( unsigned int n = 0; n < pulse_yVec.size(); ++n )
        pulseGraph[n].position.y = graphAxis[0].position.y - graphAmp*pulse_yVec[n];
}

void lvl_Lights::draw( sf::RenderTarget& RT ) const
{
 //   LtPlayLV.draw(RT);
//    LtPlay2LV.draw(RT);
    LtPlay4LV.draw(RT);
    wvPlayLV.draw(RT);
    pulsePlayLV.draw(RT);
    LtMoverLV.draw(RT);
    DataPlayLV.draw(RT);
    // pulse graph
 //   RT.draw( graphAxis, 2, sf::Lines );

 //   RT.draw( &( pulseGraph[0] ), pulseGraph.size(), sf::LinesStrip );
}

multiSelector* lvl_Lights::init_controlMS( controlSurface& rCS, buttonValOnHit*& pStrip, sf::Vector2f HBpos, sf::Vector2f HBsz, sf::Vector2f sfcOfst, std::istream& is )
{
    std::string inStr; is >> inStr;// label for home button
    sf::Vector2f PosSurf( HBpos + sfcOfst );
    sf::Vector2f SzSurf; is >> SzSurf.x >> SzSurf.y;
    rCS.init( HBpos, PosSurf, SzSurf, inStr.c_str(), HBsz.x, HBsz.y );
    rCS.ownButts = true;

    sf::Vector2f Pos; is >> Pos.x >> Pos.y;// as offset from PosSurf
    Pos += PosSurf;
    pStrip = new buttonValOnHit();
    multiSelector* pMS = new multiSelector( Pos, pStrip, is );
    pMS->ownsStrip = true;
    rCS.pButtVec.push_back( pMS );

    is >> inStr;
    std::cout << '\n' << inStr;
    return pMS;
}

bool lvl_Lights::init_MultiControl( multiSelector* pMS, buttonValOnHit* pStrip, std::istream& is )
{
    if( pMS->sdataVec.size() != 5 ) { std::cout << "\nbad multiSel data"; return false; }
    stripData* pSD = &( pMS->sdataVec.front() );
    // wvPlay.AmpRt
    pSD->pSetFunc =     [this](float x){ wvPlay.AmpRt = x; wvPlay.AmpLt = 1.0f - wvPlay.AmpRt; };
    pSD->pSetFunc( pSD->xCurr );
    pStrip->reInit( *pSD );

    // wvPlay.wvLenRt
    (++pSD)->pSetFunc = [this](float x){ wvPlay.wvLenRt = x; wvPlay.periodRt = wvPlay.wvLenRt/wvPlay.wvSpdRt; wvPlay.tElapRt = 0.0f; };
    pSD->pSetFunc( pSD->xCurr );
    // wvPlay.wvSpdRt
    (++pSD)->pSetFunc = [this](float x){ wvPlay.wvSpdRt = x; wvPlay.periodRt = wvPlay.wvLenRt/wvPlay.wvSpdRt; wvPlay.tElapRt = 0.0f; };
    pSD->pSetFunc( pSD->xCurr );

    // wvPlay.wvLenLt
    (++pSD)->pSetFunc = [this](float x){ wvPlay.wvLenLt = x; wvPlay.periodLt = wvPlay.wvLenLt/wvPlay.wvSpdLt; wvPlay.tElapLt = 0.0f; };
    pSD->pSetFunc( pSD->xCurr );
    // wvPlay.wvSpdLt
    (++pSD)->pSetFunc = [this](float x){ wvPlay.wvSpdLt = x; wvPlay.periodLt = wvPlay.wvLenLt/wvPlay.wvSpdLt; wvPlay.tElapLt = 0.0f; };
    pSD->pSetFunc( pSD->xCurr );

    // check
    std::string inStr; is >> inStr;
    std::cout << '\n' << inStr;

    return true;
}

bool lvl_Lights::init_controls( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ) { std::cout << "\nNo control data"; return false; }

    // control surfaces
    sf::Vector2f HBpos, HBsz;
    std::string fName;
    multiSelector* pMS = nullptr;
    buttonValOnHit* pVOH = nullptr;
    std::ifstream finControl;
    sf::Vector2f sfcOfst;

    // multi control
    fin >> fName >> HBpos.x >> HBpos.y >> HBsz.x >> HBsz.y >> sfcOfst.x >> sfcOfst.y;
    finControl.open( fName.c_str() );
    if( !finControl ) { std::cout << "\n init_controls() could not open " << fName; return false; }
    pMS = init_controlMS( multiCS, pVOH, HBpos, HBsz, sfcOfst, finControl );
    init_MultiControl( pMS, pVOH, finControl );
    std::cout << "\n initMC done";
    finControl.close();
    button::RegisteredButtVec.push_back( &multiCS );

    // toggle LtPlay2 on / off
    float x, y, W,  H;
    fin >> x >> y >> W >> H;
    pLtPlay2_Butt.init( x, y, W, H, [this](){ if( pLtPlay2_Butt.sel ) LtPlay2.Reset(); }, "Play" );
    pLtPlay2_Butt.setSel( true );
    button::RegisteredButtVec.push_back( &pLtPlay2_Butt );

    return true;
}
