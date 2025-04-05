#include "PlayerFuncs.h"

bool initDataPlayer( DataPlayer& DP, Light& rLt0, int GridRows, int GridCols, std::vector<uint8_t>& DataVec, std::istream& is )
{
    DataVec.clear();

 //   int gridRows = GridRows;
 //   int gridCols = GridCols;

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
        DP.Lt[k].init(rd,gn,bu);
    }

    // the data
    bool dataInBits;
    is >> dataInBits;

    if( dataInBits )
    {
        if( numColors == 16 )
        {
            std::cout << "\n initDataPlay(): 16 colors from dblBits";
            unsigned int numQuadBits;
            is >> numQuadBits;
            std::cout << "\n numQuadBits = " << numQuadBits;

            if( numQuadBits%2 == 0 ) DataVec.resize( numQuadBits/2 );
            else DataVec.resize( numQuadBits/2 + 1 );
            DP.init( rLt0, Rows, Cols, DataVec[0], DataVec.size(), 16 );

            std::cout << "\n DataVec.size() = " << DataVec.size();

            unsigned int inVal = 0;
            DP.BA.Clear();
            for( unsigned int k = 0; k < numQuadBits; ++k )
            {
                is >> inVal;
                DP.BA.pushQuad( inVal );
            }
        }
        else if( numColors == 4 )
        {
            std::cout << "\n initDataPlay(): 4 colors from dblBits";
            unsigned int numDblBits;
            is >> numDblBits;

            if( numDblBits%4 == 0 ) DataVec.resize( numDblBits/4 );
            else DataVec.resize( numDblBits/4 + 1 );
            std::cout << "\n DataVec.size() = " << DataVec.size();

            DP.init( rLt0, Rows, Cols, DataVec[0], DataVec.size(), 4 );

            unsigned int inVal = 0;
            DP.BA.Clear();
            for( unsigned int k = 0; k < numDblBits; ++k )
            {
                is >> inVal;
                DP.BA.push( inVal/2 > 0 );// hi bit
                DP.BA.push( inVal%2 > 0 );// lo bit
            }
        }
        else if( numColors == 2 )
        {
            std::cout << "\n initDataPlay(): 2 colors from bits";
            unsigned int numBits;
            is >> numBits;
            if( numBits%8 == 0 ) DataVec.resize( numBits/8 );
            else DataVec.resize( numBits/8 + 1 );
            DP.init( rLt0, Rows, Cols, DataVec[0], DataVec.size(), 2 );

            bool inVal = 0;
            DP.BA.Clear();
            for( unsigned int k = 0; k < numBits; ++k )
            {
                is >> inVal;
                DP.BA.push( inVal );
            }
        }
    }
    else// data is in whole bytes which may represent 2, 4 or 16 colors
    {
        std::cout << "\n initDataPlay(): " << numColors << " colors from Bytes";
        unsigned int numBytes;
        is >> numBytes;
        DataVec.reserve( numBytes );
        unsigned int inVal = 0;
        for( unsigned int k = 0; k < numBytes; ++k )
        {
            is >> inVal;
            DataVec.push_back( inVal );
        }

        DP.init( rLt0, Rows, Cols, DataVec[0], DataVec.size(), numColors );
    }

    DP.setGridBounds( Row0, Col0, GridRows, GridCols );

    std::cout << "\n initDataPlay(): BA.bitSize() = " << DP.BA.bitSize();

    return true;
}

bool initPatternPlayer( LightPlayer2& LP, Light& rLt0, std::vector<patternData>& pattDataVec, std::istream& is )
{
    pattDataVec.clear();

    int Rows, Cols, gridRows, gridCols;
    int Row0, Col0;
    is >> Rows >> Cols >> gridRows >> gridCols;
    is >> Row0 >> Col0;

    unsigned int rd, gn, bu;
    is >> rd >> gn >> bu;
    LP.onLt.init(rd,gn,bu);
    is >> rd >> gn >> bu;
    LP.offLt.init(rd,gn,bu);

    unsigned int numPatterns;
    is >> numPatterns;
    pattDataVec.reserve( numPatterns );

    patternData PD;
    for( unsigned int k = 0; k < numPatterns; ++k )
    {
        is >> PD.funcIndex >> PD.stepPause >> PD.param;
        pattDataVec.push_back( PD );
    }

    LP.init( rLt0, Rows, Cols, pattDataVec[0], numPatterns );
    LP.setGridBounds( Row0, Col0, gridRows, gridCols );

    return true;
}
