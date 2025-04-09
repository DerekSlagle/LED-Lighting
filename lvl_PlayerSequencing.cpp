#include "lvl_PlayerSequencing.h"

bool lvl_PlayerSequencing::init()
{
    std::ifstream fin("include/levels/lvl_PlaySeq/init_data.txt");
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
    unsigned int ArrSz = GridRows*GridCols;
    fin >> rd >> gn >> bu;
    GridClearLt.init(rd,gn,bu);
    LightGrid.resize( ArrSz, GridClearLt );

    float dPosX, dPosY;
    fin >> posX >> posY >> dPosX >> dPosY;
    sf::Vector2f LtSz;
    fin >> LtSz.x >> LtSz.y;
    GridLV.init( LightGrid[0], GridRows, GridCols, posX, posY, dPosX, dPosY, LtSz );
    GridLV.update();

    std::string fileName;
    fin >> fileName;
    if( !initPlayer2( fileName.c_str() ) ) return false;

    fin >> fileName;
    if( !initPlayer2_a( fileName.c_str() ) ) return false;

    fin >> fileName;
    if( !init_controls( fileName.c_str() ) ) return false;

    std::cerr << "\n init() check";

    return true;
}

bool lvl_PlayerSequencing::initPlayer2( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\n initPlayer2() no file"; return false; }

 //   std::vector<patternData> pattVec;// container for all
 //   std::vector<LP2data> LP2dataVec;
    unsigned int numSets;
    fin >> numSets;
    LP2dataVec.resize( numSets );
    unsigned int rd, gn, bu;

    for( unsigned int k = 0; k < numSets; ++k )
    {
        // all but LP2dataVec[k].pattData Assign must await all push_back
        fin >> LP2dataVec[k].rows;
        fin >> LP2dataVec[k].cols;
        fin >> LP2dataVec[k].row0;
        fin >> LP2dataVec[k].col0;
        fin >> rd >> gn >> bu;
        LP2dataVec[k].onLt.init(rd,gn,bu);
        fin >> rd >> gn >> bu;
        LP2dataVec[k].offLt.init(rd,gn,bu);
        // patterns
        fin >> LP2dataVec[k].numPatterns;
        patternData PD;
        for( unsigned int n = 0; n < LP2dataVec[k].numPatterns; ++n )
        {
            fin >> PD.funcIndex;
            fin >> PD.stepPause;
            fin >> PD.param;
            pattVec.push_back(PD);
        }
    }

    // assign the pattData pointers
    size_t ofst = 0;// offset
    for( LP2data& LPD : LP2dataVec )
    {
        LPD.pattData = &( pattVec[ofst] );
        ofst += LPD.numPatterns;// for next
    }

    // init the player for 1st pattern
    LP2data& rLPD = LP2dataVec[0];
    LtPlay2.init( LightGrid[0], GridRows, GridCols, *(rLPD.pattData), rLPD.numPatterns );
    LtPlay2.onLt = rLPD.onLt;
    LtPlay2.offLt = rLPD.offLt;
    LtPlay2.setRectBounds( rLPD.rows, rLPD.cols, rLPD.row0, rLPD.col0 );

    // the overlay
    fin >> numSets;
    LP2data_OverVec.resize( numSets );
    for( unsigned int k = 0; k < numSets; ++k )
    {
        fin >> LP2data_OverVec[k].rows;
        fin >> LP2data_OverVec[k].cols;
        fin >> LP2data_OverVec[k].row0;
        fin >> LP2data_OverVec[k].col0;
        fin >> rd >> gn >> bu;
        LP2data_OverVec[k].onLt.init(rd,gn,bu);
        fin >> rd >> gn >> bu;
        LP2data_OverVec[k].offLt.init(rd,gn,bu);
        // patterns
        fin >> LP2data_OverVec[k].numPatterns;
        patternData PD;
        for( unsigned int n = 0; n < LP2data_OverVec[k].numPatterns; ++n )
        {
            fin >> PD.funcIndex;
            fin >> PD.stepPause;
            fin >> PD.param;
            pattVec_Over.push_back(PD);
        }
    }

    ofst = 0;// offset
    for( LP2data& LPD : LP2data_OverVec )
    {
        LPD.pattData = &( pattVec_Over[ofst] );
        ofst += LPD.numPatterns;// for next
    }

    LP2data& rLPDo = LP2data_OverVec[0];
    LtPlay2_Over.init( LightGrid[0], GridRows, GridCols, *(rLPDo.pattData), rLPDo.numPatterns );
    LtPlay2_Over.onLt = rLPDo.onLt;
    LtPlay2_Over.offLt = rLPDo.offLt;
    LtPlay2_Over.setRectBounds( rLPDo.rows, rLPDo.cols, rLPDo.row0, rLPDo.col0 );

    return true;
}

/*
// start LP2 on indexed set in LP2dataVec
void lvl_PlayerSequencing::nextPatternSet( LightPlayer2& LP2, size_t setIdx )
{
    // reset iterators
    LP2.patternIter = 0;
    LP2.stepIter = 0;
    LP2.stepTimer = 0;

    // next set to player
    const LP2data& rLPD = LP2dataVec[ setIdx ];// for convenience

    LP2.pattData = rLPD.pattData;
    LP2.numPatterns = rLPD.numPatterns;
    LP2.onLt = rLPD.onLt;
    LP2.offLt = rLPD.offLt;
    LP2.setRectBounds( rLPD.rows, rLPD.cols, rLPD.row0, rLPD.col0 );
}
*/

// start LP2 on set in LPdata
void lvl_PlayerSequencing::newPatternSet( LightPlayer2& LP2, const LP2data& LPdata )
{
    // reset iterators
    LP2.patternIter = 0;
    LP2.stepIter = 0;
    LP2.stepTimer = 0;
    // next set to player
    LP2.pattData = LPdata.pattData;
    LP2.numPatterns = LPdata.numPatterns;
    LP2.onLt = LPdata.onLt;
    LP2.offLt = LPdata.offLt;
    LP2.setRectBounds( LPdata.rows, LPdata.cols, LPdata.row0, LPdata.col0 );
}

// player to LightGrid_a
bool lvl_PlayerSequencing::initPlayer2_a( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\n initPlayer2_a() no file"; return false; }

    // the Light grid and LV
    fin >> GridRows_a >> GridCols_a;
    unsigned int ArrSz = GridRows_a*GridCols_a;
    unsigned int rd, gn, bu;
    fin >> rd >> gn >> bu;
    GridClearLt_a.init(rd,gn,bu);
    LightGrid_a.resize( ArrSz, GridClearLt_a );

    float posX, posY, dPosX, dPosY;
    fin >> posX >> posY >> dPosX >> dPosY;
    sf::Vector2f LtSz;
    fin >> LtSz.x >> LtSz.y;
    GridLV_a.init( LightGrid_a[0], GridRows_a, GridCols_a, posX, posY, dPosX, dPosY, LtSz );

    unsigned int numFiles;
    fin >> numFiles;
    Play2_aFilenames.resize( numFiles );
    for( unsigned int k = 0; k < numFiles; ++k )
    {
        fin >> Play2_aFilenames[k];
    }

    patternData fakePD;
    LtPlay2_a.init( LightGrid_a[0], GridRows_a, GridCols_a, fakePD, 0 );

    // start with 1st file
    return nextPatternSet( LtPlay2_a, Play2_aFilenames[0].c_str() );
}

// read a pattern set from a file. Start LP2 playing this set
bool lvl_PlayerSequencing::nextPatternSet( LightPlayer2& LP2, const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\n nextPatternSet() no file"; return false; }

    int rows, cols, row0, col0;
    fin >> rows >> cols >> row0 >> col0;
    unsigned int rd, gn, bu;
    fin >> rd >> gn >> bu;
    LP2.onLt.init(rd,gn,bu);
    fin >> rd >> gn >> bu;
    LP2.offLt.init(rd,gn,bu);

    unsigned int numPatts;
    fin >> numPatts;

    pattVec_a.clear();
    for( unsigned int k = 0; k < numPatts; ++k )
    {
        patternData PD;
        fin >> PD.funcIndex;
        fin >> PD.stepPause;
        fin >> PD.param;
        pattVec_a.push_back(PD);
    }

    // reset iterators
    LP2.patternIter = 0;
    LP2.stepIter = 0;
    LP2.stepTimer = 0;

    // next set to player
    LP2.pattData = &( pattVec_a[0] );
    LP2.numPatterns = numPatts;
    LP2.setRectBounds( rows, cols, row0, col0 );

    GridLV_a.update();

    return true;
}

bool lvl_PlayerSequencing::handleEvent( sf::Event& rEvent )
{
    return true;
}

void lvl_PlayerSequencing::update( float dt )
{
    // method 1
    for( Light& Lt : LightGrid ) Lt = GridClearLt;// clear
    LtPlay2.updateSub();// 1st layer
    // restrict draw to exclude last pattern so it stops
    if( LtPlay2_Over.patternIter + 1 < LtPlay2_Over.numPatterns )
        LtPlay2_Over.updateSubOnOnly();// overlay
    // update the LightVisual
    GridLV.update();

    // method 2
    LtPlay2_a.updateSub();
    GridLV_a.update();
}

void lvl_PlayerSequencing::draw( sf::RenderTarget& RT ) const
{
    GridLV.draw(RT);
    GridLV_a.draw(RT);
}

bool lvl_PlayerSequencing::init_controls( const char* fileName )
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
    patternSetRB.init( x, y, W, H, LP2dataVec.size(), Title, 0 );
    button::RegisteredButtVec.push_back( &patternSetRB );
    // define the Lambda
    patternSetRB.pFuncIdx = [this](size_t n)
    {
        for( Light& Lt : LightGrid ) Lt = GridClearLt;// clear grid
   //     nextPatternSet( LtPlay2, n );//next set
        newPatternSet( LtPlay2, LP2dataVec[n] );//next set
    };
    // overlay
    fin >> label;
    Title.setString( label.c_str() );
    fin >> x >> y >> W >> H;
    patternSetRB_Over.init( x, y, W, H, LP2data_OverVec.size(), Title, 0 );
    button::RegisteredButtVec.push_back( &patternSetRB_Over );
    // lambda for overlay
    patternSetRB_Over.pFuncIdx = [this](size_t n){ pattSetIdx_Over = n; };
    // fire off a pattern
    patternPB.init( fin, 'O', [this](){ newPatternSet( LtPlay2_Over, LP2data_OverVec[pattSetIdx_Over] ); }, nullptr );
    button::RegisteredButtVec.push_back( &patternPB );

    // for LtPlay2_a using method 2
    fin >> label;
    Title.setString( label.c_str() );
    fin >> x >> y >> W >> H;
    patternSetRB_a.init( x, y, W, H, Play2_aFilenames.size(), Title, 0 );
    button::RegisteredButtVec.push_back( &patternSetRB_a );
    patternSetRB_a.pFuncIdx = [this](size_t n)
    {
        for( Light& Lt : LightGrid_a ) Lt = GridClearLt_a;
        nextPatternSet( LtPlay2_a, Play2_aFilenames[n].c_str() );
    };

    return true;
}
