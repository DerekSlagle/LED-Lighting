#include "lvl_DataPlayerControl.h"

bool lvl_DataPlayerControl::init()
{
    std::ifstream fin("include/levels/lvl_DPcontrol/init_data.txt");
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

    // the Light grid and LV
    fin >> GridRows >> GridCols;
    int ArrSz = GridRows*GridCols;
    fin >> rd >> gn >> bu;
    GridClearLt.init(rd,gn,bu);
    LightGrid.resize( ArrSz, GridClearLt );

    float dPosX, dPosY;
    fin >> posX >> posY >> dPosX >> dPosY;
    sf::Vector2f LtSz;
    fin >> LtSz.x >> LtSz.y;
    GridLV.init( LightGrid[0], GridRows, GridCols, posX, posY, dPosX, dPosY, LtSz );
    GridLV.update();

    // the 2nd one
    fin >> GridRows2 >> GridCols2;
    ArrSz = GridRows2*GridCols2;
    fin >> rd >> gn >> bu;
    GridClearLt2.init(rd,gn,bu);
    LightGrid2.resize( ArrSz, GridClearLt2 );

    fin >> posX >> posY >> dPosX >> dPosY;
    fin >> LtSz.x >> LtSz.y;
    GridLV2.init( LightGrid2[0], GridRows2, GridCols2, posX, posY, dPosX, dPosY, LtSz );
    GridLV2.update();

    std::string fileName;
    fin >> fileName;
    if( !initDataPlayers( fileName.c_str() ) ) return false;

    fin >> fileName;
    if( !init_wvPlay( fileName.c_str() ) ) return false;

    fin >> fileName;
    if( !init_wvPlay2( fileName.c_str() ) ) return false;

    fin >> fileName;
    if( !init_pattPlay( fileName.c_str() ) ) return false;

    // last
    fin >> fileName;
    if( !init_controls( fileName.c_str() ) ) return false;

    return true;
}

bool lvl_DataPlayerControl::initDataPlayers( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ) { std::cout << "\nNo DataPlayers data"; return false; }

    // store all of the image file names
    unsigned int numFiles;
    fin >> numFiles;
    std::cout << "\n numFiles = " << numFiles;

    DP_filenames.resize( numFiles );
    for( unsigned int k = 0; k < numFiles; ++k )
    {
        fin >> DP_filenames[k];
        std::cout << '\n' << DP_filenames[k].c_str();
    }

    // init to play from 1st file
    std::ifstream finImage( DP_filenames[0].c_str() );
    if( !finImage ) { std::cout << "\nNo file: " << DP_filenames[0].c_str(); return false; }
    DP_base.pLight = DP_Color;
    initDataPlayer( DP_base, LightGrid[0], GridRows, GridCols, DataVec, finImage );

    return true;
}

bool lvl_DataPlayerControl::init_wvPlay( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\ninit_wvPlay() no file"; return false; }

    unsigned int rd, gn, bu;
    fin >> rd >> gn >> bu;
    Light HiLt(rd,gn,bu);
    fin >> rd >> gn >> bu;
    Light LoLt(rd,gn,bu);

    wvPlay.init( LightGrid[0], GridRows, GridCols, HiLt, LoLt );

    // wave data
    float A_lt, wvLen_lt, wvSpd_lt;
    float wvLen_rt, wvSpd_rt;
    fin >> A_lt;
    fin >> wvLen_rt >> wvSpd_rt;
    fin >> wvLen_lt >> wvSpd_lt;
    wvPlay.setWaveData( A_lt, wvLen_lt, wvSpd_lt, wvLen_rt, wvSpd_rt );

    return true;
}

bool lvl_DataPlayerControl::init_wvPlay2( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\ninit_wvPlay2() no file"; return false; }

    unsigned int rd, gn, bu;
    fin >> rd >> gn >> bu;
    Light HiLt(rd,gn,bu);
    fin >> rd >> gn >> bu;
    Light LoLt(rd,gn,bu);

    wvPlay2.init( LightGrid2[0], GridRows, GridCols, HiLt, LoLt );

    // wave data
    float A_lt, wvLen_lt, wvSpd_lt;
    float wvLen_rt, wvSpd_rt;
    fin >> A_lt;
    fin >> wvLen_rt >> wvSpd_rt;
    fin >> wvLen_lt >> wvSpd_lt;
    wvPlay2.setWaveData( A_lt, wvLen_lt, wvSpd_lt, wvLen_rt, wvSpd_rt );

    // target area within LightGrid2 ?
    bool doSetRect = false;
    fin >> doSetRect;
    if( doSetRect )
    {
        float rows, cols, row0, col0;
        fin >> rows >> cols >> row0 >> col0;
        wvPlay2.setTargetRect( rows, cols, row0, col0 );
    }

    // DP_2
    std::string fName;
    fin >> fName;
    std::ifstream finImage( fName.c_str() );
    if( !finImage ) { std::cout << "\nNo file: " << fName.c_str(); return false; }
    DP_2.pLight = DP2_Color;
    initDataPlayer( DP_2, LightGrid2[0], GridRows2, GridCols2, DataVec2, finImage );

    return true;
}

bool lvl_DataPlayerControl::init_pattPlay( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\ninitPlayer2() no file"; return false; }

    int Rows, Cols, Row0, Col0;
    fin >> Rows >> Cols >>  Row0 >> Col0;
    unsigned int rd, gn, bu;
    fin >> rd >> gn >> bu;
    pattPlay.onLt.init(rd,gn,bu);
    fin >> rd >> gn >> bu;
    pattPlay.offLt.init(rd,gn,bu);

    unsigned int numPatterns;
    fin >> numPatterns;
    pattDataVec.reserve( numPatterns );

    patternData PD;
    for( unsigned int k = 0; k < numPatterns; ++k )
    {
        fin >> PD.funcIndex >> PD.stepPause >> PD.param;
        pattDataVec.push_back( PD );
    }

    pattPlay.init( LightGrid2[0], GridRows2, GridCols2, pattDataVec[0], numPatterns );
    pattPlay.setTargetRect( Rows, Cols, Row0, Col0 );

    // an array of floats for an equalizer visual
    EQvals.resize( Cols );
    float dx = 1.0f/Cols;

    for( int c = 0; c < Cols; ++c )
    {
        EQvals[c] = ( c + 1 )*dx;
    }

    // present initial state
    pattPlay.updateSub();

    return true;
}

bool lvl_DataPlayerControl::handleEvent( sf::Event& rEvent )
{
    return true;
}

void lvl_DataPlayerControl::update( float dt )
{
    for( Light& Lt : LightGrid ) Lt = GridClearLt;

    if( ZoomButt.sel )
        DP_base.updateZoomOut();
    else
        DP_base.update();

 //   pattPlay.updateSub();

    // replace image Lt[1] with wvPlay Light there
    if( OverlayButt.sel )
    {
        wvPlay.update_tElapOnly(dt);

        for( int r = DP_base.getRow0(); r < DP_base.getRow0() + DP_base.getRows(); ++r )
            for( int c = DP_base.getCol0(); c < DP_base.getCol0() + DP_base.getCols(); ++c )
            {
                int n = r*GridCols + c;
            //    if( LightGrid[n] == DP_base.Lt[1] )
                if( LightGrid[n] == DP_base.pLight[1] )
                    LightGrid[n] = wvPlay.getLt( r, c );
            }

     //   for( int r = 0; r < GridRows; ++r )
     //       for( int c = 0; c < GridCols; ++c )
     //       {
      //          int n = r*GridCols + c;
      //          if( LightGrid[n] == pattPlay.onLt )
      //              LightGrid[n] = wvPlay.getLt( r, c );
      //      }
    }


    GridLV.update();

    for( Light& Lt : LightGrid2 ) Lt = GridClearLt2;
    wvPlay2.update(dt);
    DP_2.update();

 //   pattPlay.updateSubOnOnly();
    if( pattPlay.getCols() <= (int)EQvals.size() )
        pattPlay.updateAsEq( &( EQvals[0] ) );

    GridLV2.update();
}

void lvl_DataPlayerControl::draw( sf::RenderTarget& RT ) const
{
    GridLV.draw(RT);
    GridLV2.draw(RT);
}

bool lvl_DataPlayerControl::init_controls( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ) { std::cout << "\nNo control data"; return false; }

    std::string label; fin >> label;
    sf::Text Title;
    Title.setFont( *button::pFont );
    Title.setCharacterSize(12);
    Title.setFillColor( sf::Color::Black );
    Title.setString( label.c_str() );
    float x, y, W, H;
    fin >> x >> y >> W >> H;
    // for LtPlay2 using method 1
    imageRadioButt.init( x, y, W, H, DP_filenames.size(), Title, 0 );
    button::RegisteredButtVec.push_back( &imageRadioButt );
    // define the Lambda
    imageRadioButt.pFuncIdx = [this](size_t n)
    {
        for( Light& Lt : LightGrid ) Lt = GridClearLt;// clear grid

        std::ifstream finImage( DP_filenames[n].c_str() );
        if( finImage )
            initDataPlayer( DP_base, LightGrid[0], GridRows, GridCols, DataVec, finImage );
    };

    // ZoomButt
    fin >> label;
    Title.setString( label.c_str() );
    fin >> x >> y >> W >> H;
    ZoomButt.init( x, y, W, H, [this](){ for( Light& Lt : LightGrid ) Lt = GridClearLt; }, Title );
    button::RegisteredButtVec.push_back( &ZoomButt );

    // PauseButt
    fin >> label;
    Title.setString( label.c_str() );
    fin >> x >> y >> W >> H;
    PauseButt.init( x, y, W, H, [this](){ DP_base.isPlaying = PauseButt.sel; }, Title );
    button::RegisteredButtVec.push_back( &PauseButt );

    // OverlayButt
    fin >> label;
    Title.setString( label.c_str() );
    fin >> x >> y >> W >> H;
    OverlayButt.init( x, y, W, H, nullptr, Title );
    button::RegisteredButtVec.push_back( &OverlayButt );

    return true;
}
