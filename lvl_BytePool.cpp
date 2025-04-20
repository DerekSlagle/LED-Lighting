#include "lvl_BytePool.h"

bool lvl_BytePool::init()
{
    std::ifstream fin("include/levels/lvl_BytePool/init_data.txt");
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

    // pool report
    poolMsg.setFont( *button::pFont );
    poolMsg.setCharacterSize(12);
    poolMsg.setFillColor( sf::Color::White );
    fin >> posX >> posY;
    poolMsg.setPosition( posX, posY );

    std::string fileName;
    fin >> fileName;
    if( !initDataPlayers( fileName.c_str() ) ) return false;

    fin >> fileName;
    if( !initPatternPlayers( fileName.c_str() ) ) return false;

    fin >> fileName;
    if( !init_controls( fileName.c_str() ) ) return false;

    return true;
}

bool lvl_BytePool::initDataPlayers( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ) { std::cout << "\nNo DataPlayers data"; return false; }

    fin >> numDP;
//    DPvec.resize( numDP );

    PattPlay.resize( numDP );
    PP_row0.resize( numDP );
    PP_col0.resize( numDP );
    DP_Color.resize( 16*numDP );

    ppBytes.resize( 3*numDP );// includes space for
    pBlockSz.resize( 3*numDP );// the PatternPlayers
    // assign pointers
    // the numDP DataPlayers
    for( unsigned int n = 0; n < numDP; ++n )
    {
        fin >> DProw0[n] >> DPcol0[n];
        ppBytes[n] = &( DParr[n].BA.pByte );
        pBlockSz[n] = &( DParr[n].BA.capBytes );
        DParr[n].pLight = &( DP_Color[ 16*n ] );
    }
    // the numDP PatternPlayers
    for( unsigned int n = 0; n < numDP; ++n )
    {
        fin >> PP_row0[n] >> PP_col0[n];
        ppBytes[ numDP + n ] = &( PattPlay[n].pattData );
        pBlockSz[ numDP + n ] = &( PattPlay[n].numBytes );
    }
    // the numDP LightPalettes
    for( unsigned int n = 0; n < numDP; ++n )
    {
        ppBytes[ 2*numDP + n ] = &( DPpalette[n].pByte );
        pBlockSz[ 2*numDP + n ] = &( DPpalette[n].numBytes );
    }

    unsigned int totalBytes = 0;
    fin >> totalBytes;
    DataVec.resize( totalBytes );
    memPool.init( &( DataVec[0] ), totalBytes );
    memPool.bindUsers( &( ppBytes[0] ), &( pBlockSz[0] ), 3*numDP );
 //   memPool.bindUsers( &( ppBytes[0] ), &( pBlockSz[0] ), numDP );

    // store all of the image file names
    unsigned int numFiles;
    fin >> numFiles;
    std::cout << "\n numFiles = " << numFiles;

    DP_filenames.resize( numFiles );


    for( unsigned int k = 0; k < numFiles; ++k )
    {
        fin >> DP_filenames[k];
    }

    // init to play from 1st file
    for( unsigned int n = 0; n < numDP; ++n )
    {
        unsigned int fileIdx; fin >> fileIdx;
        std::ifstream finImage( DP_filenames[ fileIdx ].c_str() );
        if( !finImage ) { std::cout << "\nNo file: " << DP_filenames[ fileIdx ].c_str(); return false; }

    //    if( loadImageData( n, LightGrid[0], GridRows, GridCols, finImage ) )
        if( loadImageAndPalette( n, LightGrid[0], GridRows, GridCols, finImage ) )
        {
            DParr[n].setRow0( DProw0[n] );
            DParr[n].setCol0( DPcol0[n] );
        }
        else
            std::cout << "\ninitDataPlayers(): bad image load";
    }

    fin.close();

    poolMsg.setString( "poolMsg" );
    makeReport();
    memPool.Report( std::cout );

    return true;
}

void lvl_BytePool::makeReport()
{
    std::ofstream rptOut( "include/levels/lvl_BytePool/BPreport.txt" );
    memPool.Report( rptOut );
    rptOut.close();
    std::ifstream rptIn( "include/levels/lvl_BytePool/BPreport.txt" );
    if( rptIn )
    {
        std::string helpStr, inStr;
        if( getline( rptIn, inStr) )
        {
            helpStr = inStr + '\n';
            while( getline( rptIn, inStr) ) helpStr += '\n' + inStr;
            poolMsg.setString( helpStr.c_str() );
        }
        rptIn.close();
    }
}

bool lvl_BytePool::handleEvent( sf::Event& rEvent )
{
    return true;
}

void lvl_BytePool::update( float dt )
{
    for( Light& Lt : LightGrid )
        Lt = GridClearLt;

    for( unsigned int n = 0; n < numDP; ++n )
    {
        if( DParr[n].BA.pByte )
            DParr[n].update();
    }

    for( unsigned int n = 0; n < numDP; ++n )
    {
        if( PattPlay[n].pattData )
            PattPlay[n].update();
    }

    GridLV.update();
}

void lvl_BytePool::draw( sf::RenderTarget& RT ) const
{
    GridLV.draw(RT);
    RT.draw( poolMsg );
}

bool lvl_BytePool::loadImageAndPalette( unsigned int idxDP, Light& rLt0, int GridRows, int GridCols, std::istream& is )
{
    DataPlayer& DP = DParr[ idxDP ];

    int Rows, Cols;
    int Row0, Col0;
    is >> Rows >> Cols;
    is >> Row0 >> Col0;
    is >> DP.stepPause;
    is >> DP.drawOff;
    is >> DP.fadeAlong;
    unsigned int NumColors = 2;
 //   bool Is4color;
    is >> NumColors;

    // allocate for storage
    const unsigned int idxPalette = idxDP + 2*numDP;
    if( memPool.Alloc( idxPalette, 3*NumColors ) )
    {
        DP.numColors = NumColors;
        DP.pLight = new ( *memPool.ppBlock[ idxPalette ] ) Light[ NumColors ];
        unsigned int rd, gn, bu;
        for( unsigned int k = 0; k < NumColors; ++k )
        {
            is >> rd >> gn >> bu;
            DP.pLight[k].init(rd,gn,bu);
        }
    }
    else
    {
        std::cout << "\n loadImageAndPalette(): palette alloc fail";
        return false;
    }

    unsigned int ByteCap = 0;
    // the data
    bool dataInBits;
    is >> dataInBits;

    if( dataInBits )
    {
        if( NumColors > 4 && NumColors <= 16 )
        {
        //    std::cout << "\n loadImageData(): 16 colors from dblBits";
            unsigned int numQuadBits;
            is >> numQuadBits;
        //    std::cout << "\n numQuadBits = " << numQuadBits;

            if( numQuadBits%2 == 0 ) ByteCap = numQuadBits/2;
            else ByteCap = numQuadBits/2 + 1;
        //    std::cout << "\n ByteCap = " << ByteCap;
            if( memPool.Alloc( idxDP, ByteCap ) )
            {
                DP.init( rLt0, Rows, Cols, **ppBytes[ idxDP ], ByteCap, NumColors );
                unsigned int inVal = 0;
                DP.BA.Clear();
                for( unsigned int k = 0; k < numQuadBits; ++k )
                {
                    is >> inVal;
                    DP.BA.pushQuad( inVal );
                }
            }
            else
                std::cout << "\n loadImageData() bad Alloc";

        }
        else if( NumColors > 2 )// 3 or 4
        {
       //     std::cout << "\n initDataPlay(): 4 colors from dblBits";
            unsigned int numDblBits;
            is >> numDblBits;

            if( numDblBits%4 == 0 ) ByteCap = numDblBits/4;
            else ByteCap = numDblBits/4 + 1;
       //     std::cout << "\n ByteCap = " << ByteCap;
            if( memPool.Alloc( idxDP, ByteCap ) )
            {
                DP.init( rLt0, Rows, Cols, **ppBytes[ idxDP ], ByteCap, NumColors );
                unsigned int inVal = 0;
                DP.BA.Clear();
                for( unsigned int k = 0; k < numDblBits; ++k )
                {
                    is >> inVal;
                    DP.BA.push( inVal/2 > 0 );// hi bit
                    DP.BA.push( inVal%2 > 0 );// lo bit
                }
            }
            else
                std::cout << "\n loadImageData() bad Alloc";

        }
        else if( NumColors == 2 )
        {
      //      std::cout << "\n initDataPlay(): 2 colors from bits";
            unsigned int numBits;
            is >> numBits;
            if( numBits%8 == 0 ) ByteCap = numBits/8;
            else ByteCap = numBits/8 + 1;
       //     std::cout << "\n ByteCap = " << ByteCap;
            if( memPool.Alloc( idxDP, ByteCap ) )
            {
                DP.init( rLt0, Rows, Cols, **ppBytes[ idxDP ], ByteCap, 2 );
                bool inVal = 0;
                DP.BA.Clear();
                for( unsigned int k = 0; k < numBits; ++k )
                {
                    is >> inVal;
                    DP.BA.push( inVal );
                }
            }
            else
                std::cout << "\n loadImageData() bad Alloc";

        }
        else// numColors bad value
            std::cout << "\n loadImageData() bad NumColors = " << NumColors;
    }
    else// data is in whole bytes which may represent 2, 4 or 16 colors
    {
        std::cout << "\n initDataPlay(): " << NumColors << " colors from Bytes";
        unsigned int numBytes;
        is >> numBytes;
        if( memPool.Alloc( idxDP, numBytes ) )
        {
            unsigned int inVal = 0;
            for( unsigned int k = 0; k < numBytes; ++k )
            {
                is >> inVal;
                *( *ppBytes[ idxDP ] + k ) = inVal;
            }

            DP.init( rLt0, Rows, Cols, **ppBytes[ idxDP ], numBytes, NumColors );
        //    DP.init( rLt0, Rows, Cols, DataVec[0], DataVec.size(), numColors );
        }

    }

    DP.setGridBounds( Row0, Col0, GridRows, GridCols );

    std::cout << "\n initDataPlay(): BA.bitSize() = " << DP.BA.bitSize();

    return true;
}

bool lvl_BytePool::loadImageData( unsigned int idxDP, Light& rLt0, int GridRows, int GridCols, std::istream& is )
{
    DataPlayer& DP = DParr[ idxDP ];

    int Rows, Cols;
    int Row0, Col0;
    is >> Rows >> Cols;
    is >> Row0 >> Col0;
    is >> DP.stepPause;
    is >> DP.drawOff;
    is >> DP.fadeAlong;
    unsigned int numColors = 2;
 //   bool Is4color;
    is >> numColors;

    unsigned int rd, gn, bu;
    for( unsigned int k = 0; k < numColors; ++k )
    {
        is >> rd >> gn >> bu;
    //    DP.Lt[k].init(rd,gn,bu);
        DP.pLight[k].init(rd,gn,bu);
    }

    unsigned int ByteCap = 0;
    // the data
    bool dataInBits;
    is >> dataInBits;

    if( dataInBits )
    {
        if( numColors > 4 && numColors <= 16 )
        {
        //    std::cout << "\n loadImageData(): 16 colors from dblBits";
            unsigned int numQuadBits;
            is >> numQuadBits;
        //    std::cout << "\n numQuadBits = " << numQuadBits;

            if( numQuadBits%2 == 0 ) ByteCap = numQuadBits/2;
            else ByteCap = numQuadBits/2 + 1;
        //    std::cout << "\n ByteCap = " << ByteCap;
            if( memPool.Alloc( idxDP, ByteCap ) )
            {
                DP.init( rLt0, Rows, Cols, **ppBytes[ idxDP ], ByteCap, numColors );
                unsigned int inVal = 0;
                DP.BA.Clear();
                for( unsigned int k = 0; k < numQuadBits; ++k )
                {
                    is >> inVal;
                    DP.BA.pushQuad( inVal );
                }
            }
            else
                std::cout << "\n loadImageData() bad Alloc";

        }
        else if( numColors > 2 )// 3 or 4
        {
       //     std::cout << "\n initDataPlay(): 4 colors from dblBits";
            unsigned int numDblBits;
            is >> numDblBits;

            if( numDblBits%4 == 0 ) ByteCap = numDblBits/4;
            else ByteCap = numDblBits/4 + 1;
       //     std::cout << "\n ByteCap = " << ByteCap;
            if( memPool.Alloc( idxDP, ByteCap ) )
            {
                DP.init( rLt0, Rows, Cols, **ppBytes[ idxDP ], ByteCap, numColors );
                unsigned int inVal = 0;
                DP.BA.Clear();
                for( unsigned int k = 0; k < numDblBits; ++k )
                {
                    is >> inVal;
                    DP.BA.push( inVal/2 > 0 );// hi bit
                    DP.BA.push( inVal%2 > 0 );// lo bit
                }
            }
            else
                std::cout << "\n loadImageData() bad Alloc";

        }
        else if( numColors == 2 )
        {
      //      std::cout << "\n initDataPlay(): 2 colors from bits";
            unsigned int numBits;
            is >> numBits;
            if( numBits%8 == 0 ) ByteCap = numBits/8;
            else ByteCap = numBits/8 + 1;
       //     std::cout << "\n ByteCap = " << ByteCap;
            if( memPool.Alloc( idxDP, ByteCap ) )
            {
                DP.init( rLt0, Rows, Cols, **ppBytes[ idxDP ], ByteCap, 2 );
                bool inVal = 0;
                DP.BA.Clear();
                for( unsigned int k = 0; k < numBits; ++k )
                {
                    is >> inVal;
                    DP.BA.push( inVal );
                }
            }
            else
                std::cout << "\n loadImageData() bad Alloc";

        }
        else// numColors bad value
            std::cout << "\n loadImageData() bad numColors = " << numColors;
    }
    else// data is in whole bytes which may represent 2, 4 or 16 colors
    {
        std::cout << "\n initDataPlay(): " << numColors << " colors from Bytes";
        unsigned int numBytes;
        is >> numBytes;
        if( memPool.Alloc( idxDP, numBytes ) )
        {
            unsigned int inVal = 0;
            for( unsigned int k = 0; k < numBytes; ++k )
            {
                is >> inVal;
                *( *ppBytes[ idxDP ] + k ) = inVal;
            }

            DP.init( rLt0, Rows, Cols, **ppBytes[ idxDP ], numBytes, numColors );
        //    DP.init( rLt0, Rows, Cols, DataVec[0], DataVec.size(), numColors );
        }

    }

    DP.setGridBounds( Row0, Col0, GridRows, GridCols );

    std::cout << "\n initDataPlay(): BA.bitSize() = " << DP.BA.bitSize();

    return true;
}

bool lvl_BytePool::initPatternPlayers( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ) { std::cout << "\nNo PatternPlayers data"; return false; }

 //   PattPlay.resize( numDP );
 //   PP_row0.resize( numDP );
  //  PP_col0.resize( numDP );

    // assign the other pointers
  //  for( unsigned int n = 0; n < numDP; ++n )
  //  {
   //     fin >> PP_row0[n] >> PP_col0[n];
  //      ppBytes[ numDP + n ] = &( PattPlay[n].pattData );
  //      pBlockSz[ numDP + n ] = &( PattPlay[n].numPatterns );
  //  }

    // store all of the pattern file names
    unsigned int numFiles;
    fin >> numFiles;
    std::cout << "\ninitPatternPlayers(): numFiles = " << numFiles;
    PattPlay_filenames.resize( numFiles );
    for( unsigned int k = 0; k < numFiles; ++k )
        fin >> PattPlay_filenames[k];

    // init to play from files
    for( unsigned int n = 0; n < numDP; ++n )
    {
        unsigned int fileIdx; fin >> fileIdx;
        std::ifstream finImage( PattPlay_filenames[ fileIdx ].c_str() );
        if( !finImage ) { std::cout << "\nNo file: " << PattPlay_filenames[ fileIdx ].c_str(); return false; }

        if( loadPatternData( n, LightGrid[0], GridRows, GridCols, finImage ) )
        {
            PattPlay[n].drawOffLt = false;
       //     PattPlay[n].setCol0( PP_col0[n] );
        }
        else
            std::cout << "\ninitPatternPlayers(): bad pattern load";
    }

    fin.close();

    makeReport();
    memPool.Report( std::cout );

    return true;
}

bool lvl_BytePool::loadPatternData( unsigned int idxPP, Light& rLt0, int GridRows, int GridCols, std::istream& is )
{
    PatternPlayer& PP = PattPlay[ idxPP ];

    int Rows, Cols;
    int Row0, Col0;
    is >> Rows >> Cols;
    is >> Row0 >> Col0;

    unsigned int rd, gn, bu;
    is >> rd >> gn >> bu;
    PP.onLt.init(rd,gn,bu);
    is >> rd >> gn >> bu;
    PP.offLt.init(rd,gn,bu);

    unsigned int NumPatterns;
    is >> NumPatterns;
    std::cout << "\n loadPatternData() NumPatterns = " << NumPatterns;

    if( memPool.Alloc( numDP + idxPP, 3*NumPatterns ) )
    {
        PP.init( rLt0, Rows, Cols, *ppBytes[ numDP + idxPP ], NumPatterns );
        PP.setGridBounds( PP_row0[ idxPP ], PP_col0[ idxPP ], GridRows, GridCols );

        unsigned int funcIdx = 0, Delay = 0, Param = 0;
   //     PP.Clear();
        for( unsigned int k = 0; k < NumPatterns; ++k )
        {
            is >> funcIdx >> Delay >> Param;
            PP.pattData[ 3*k ] = funcIdx;
            PP.pattData[ 3*k+1 ] = Delay;
            PP.pattData[ 3*k+2 ] = Param;
        }

        return true;
    }

    std::cout << "\n loadPatternData() bad Alloc";


    return false;
}

bool lvl_BytePool::init_controls( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ) { std::cout << "\nNo control data"; return false; }

    imageSetRB.resize( numDP );
    imageClearPB.resize( numDP );
    patternSetRB.resize( numDP );
    patternClearPB.resize( numDP );

    // position them
    sf::FloatRect GridRect = GridLV.getRect();
    sf::Vector2f LtSz = GridLV.getLtSize();

    sf::Text Title;
    Title.setFont( *button::pFont );
    Title.setCharacterSize(14);
    Title.setFillColor( sf::Color::Black );
    Title.setString( "Load Image" );

    float Wrb, Hrb, ofstYrb, ofstYpb;
    fin >> Wrb >> Hrb >> ofstYrb;// strip dimension
    sf::Vector2f PBsz;
    fin >> PBsz.x >> PBsz.y >> ofstYpb;// button dimension
    const unsigned int numFiles = DP_filenames.size();
    std::string PBlabel;
    PBlabel = "Clear";

    for( unsigned int k = 0; k < numDP; ++k )
    {
        Title.setString( "Load Image" );

        float x = GridRect.left + ( LtSz.x + 2.0f )*DParr[k].getCol0();
        float y = GridRect.top + GridRect.height + ofstYrb;

        imageSetRB[k].init( x, y, Wrb, Hrb, numFiles, Title, k%numFiles );
        imageSetRB[k].pFuncIdx = [this, k](size_t n)
        {
       //     memPool.Free( k );
            DParr[k].Clear( GridClearLt );

            std::ifstream finImage( DP_filenames[n].c_str() );
        //    if( finImage && loadImageData( k, LightGrid[0], GridRows, GridCols, finImage ) )
            if( finImage && loadImageAndPalette( k, LightGrid[0], GridRows, GridCols, finImage ) )
            {
                finImage.close();
                DParr[k].reStart();
                DParr[k].setRow0( DProw0[k] );
                DParr[k].setCol0( DPcol0[k] );
                makeReport();
            }
        };
        button::RegisteredButtVec.push_back( &( imageSetRB[k] ) );

        imageClearPB[k].init( sf::Vector2f( x + Wrb + 4.0f, y ), PBsz, 'O', PBlabel, nullptr, nullptr );
        imageClearPB[k].pFuncClose = [this, k]()
        {
            memPool.Free( k );
            DParr[k].reStart();
            DParr[k].Clear( GridClearLt );
            // free the palette too
            memPool.Free( 2*numDP + k );
            makeReport();
        };
        button::RegisteredButtVec.push_back( &( imageClearPB[k] ) );

        // for the patterns
        const unsigned int numPattFiles = PattPlay_filenames.size();
        Title.setString( "Set Pattern" );
        patternSetRB[k].init( x, y + ofstYpb, Wrb, Hrb, numPattFiles, Title, k%numPattFiles );
        patternSetRB[k].pFuncIdx = [this, k](size_t n)
        {
            memPool.Free( k + numDP );
            PattPlay[k].numPatterns = 0;

            std::ifstream finPatt( PattPlay_filenames[n].c_str() );
            if( finPatt && loadPatternData( k, LightGrid[0], GridRows, GridCols, finPatt ) )
            {
                finPatt.close();
           //     PattPlay[k].setRow0( PP_row0[k] );
           //     PattPlay[k].setCol0( PP_col0[k] );
                makeReport();
            }
        };
        button::RegisteredButtVec.push_back( &( patternSetRB[k] ) );

        patternClearPB[k].init( sf::Vector2f( x + Wrb + 4.0f, y + ofstYpb ), PBsz, 'O', PBlabel, nullptr, nullptr );
        patternClearPB[k].pFuncClose = [this, k]()
        {
            memPool.Free( k + numDP );
            PattPlay[k].numPatterns = 0;
            makeReport();
        };
        button::RegisteredButtVec.push_back( &( patternClearPB[k] ) );
    }

    // the defrag button
    PBlabel = "Defrag";
    sf::Vector2f PBpos;
    fin >> PBpos.x >> PBpos.y;
    poolDefragPB.init( PBpos, PBsz, 'O', PBlabel, nullptr, nullptr );
    poolDefragPB.pFuncClose = [this]()
    {
        memPool.DeFrag();

        for( unsigned int k = 0; k < numDP; ++k )
        {
        //    DParr[k].numColors = *( memPool.pBlockSz[ 2*numDP + k ] )/3;
        //    DParr[k].pLight = new ( *memPool.ppBlock[ 2*numDP + k ] ) Light[ DParr[k].numColors ];
        //    DParr[k].pLight = new ( DPpalette[ k ].pByte ) Light[ DParr[k].numColors ];
            DParr[k].pLight = reinterpret_cast<Light*>( *memPool.ppBlock[ 2*numDP + k ] );
        }

        makeReport();
    };
    button::RegisteredButtVec.push_back( &poolDefragPB );

    return true;
}
