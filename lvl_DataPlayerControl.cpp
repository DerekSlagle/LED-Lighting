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
    if( !initDataPlayers( fileName.c_str() ) ) return false;

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
    initDataPlayer( DP_base, LightGrid[0], GridRows, GridCols, DataVec, finImage );

    return true;
}

bool lvl_DataPlayerControl::handleEvent( sf::Event& rEvent )
{
    return true;
}

void lvl_DataPlayerControl::update( float dt )
{
    DP_base.update();
    GridLV.update();
}

void lvl_DataPlayerControl::draw( sf::RenderTarget& RT ) const
{
    GridLV.draw(RT);
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

    return true;
}
