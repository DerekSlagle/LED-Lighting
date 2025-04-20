#ifndef LVL_BYTEPOOL_H
#define LVL_BYTEPOOL_H

#include "Level.h"

#include "../Light_types/PlayerFuncs.h"
#include "../Light_types/PatternPlayer.h"
#include "../Light_types/LightPalette.h"
#include "../LightVisual.h"

#include "../utility/BytePool.h"

#include "../button_types/radioButton.h"
#include "../button_types/pushButton.h"

class lvl_BytePool : public Level
{
    public:
    std::vector<Light> LightGrid;
    int GridRows = 1, GridCols = 1;
    LightVisual GridLV;
    Light GridClearLt;

    // DataPlayers
//    std::vector<DataPlayer> DPvec;
    DataPlayer DParr[20];
    int DProw0[20], DPcol0[20];
    unsigned int numDP = 5;
    std::vector<Light> DP_Color;
    std::vector<std::string> DP_filenames;
    bool initDataPlayers( const char* fileName );
    bool loadImageData( unsigned int idxDP, Light& rLt0, int GridRows, int GridCols, std::istream& is );
    bool loadImageAndPalette( unsigned int idxDP, Light& rLt0, int GridRows, int GridCols, std::istream& is );
    LightPalette DPpalette[20];// users 2*numDP t0 3*numDP-1

    // PatternPlayers
    std::vector<PatternPlayer> PattPlay;// users numDP t0 2*numDP-1
    std::vector<int> PP_row0, PP_col0;
  //  unsigned int num_PP = 5;// 1 to 1 with the DataPlayers
    std::vector<std::string> PattPlay_filenames;
    bool initPatternPlayers( const char* fileName );
    bool loadPatternData( unsigned int idxPP, Light& rLt0, int GridRows, int GridCols, std::istream& is );

    // BytePool
    std::vector<uint8_t> DataVec;// storage for memPool below
    BytePool memPool;// storage for bitArray data
    // pointer arrays for memPool
    std::vector<uint8_t**> ppBytes;
    std::vector<unsigned int*> pBlockSz;
    // BytePool reports
    sf::Text poolMsg;
    void makeReport();

    // LightPool

    // controls
    // for the DataPlayers
    std::vector<radioButton> imageSetRB;// select which image to loadfor each user
    std::vector<pushButton> imageClearPB;// Free the block
    // for the PatternPlayers
    std::vector<radioButton> patternSetRB;
    std::vector<pushButton> patternClearPB;

    pushButton poolDefragPB;
 //   pushButton patternPB_a;
    bool init_controls( const char* fileName );



    virtual bool init();
    virtual bool handleEvent( sf::Event& rEvent );
    virtual void update( float dt );
    virtual void draw( sf::RenderTarget& RT ) const;

    lvl_BytePool(){}
    virtual ~lvl_BytePool(){}

    protected:

    private:
};

#endif // LVL_BYTEPOOL_H
