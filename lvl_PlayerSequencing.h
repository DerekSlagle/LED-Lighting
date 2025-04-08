#ifndef LVL_PLAYERSEQUENCING_H
#define LVL_PLAYERSEQUENCING_H

#include "Level.h"

#include "../Light_types/LightPlayer.h"
#include "../Light_types/LightPlayer2.h"
#include "../Light_types/LightPlayer4.h"
//#include "../Light_types/DataPlayer.h"// included in PlayerFuncs
#include "../Light_types/WavePlayer.h"
#include "../Light_types/PulsePlayer.h"
#include "../Light_types/LightMover.h"

#include "../Light_types/PlayerFuncs.h"
#include "../LightVisual.h"

#include "../button_types/radioButton.h"
#include "../button_types/pushButton.h"

// for LightPlayer2
struct LP2data// plays on same grid
{
    const patternData* pattData = nullptr;
    unsigned int numPatterns = 0;
    Light onLt, offLt;
    int rows, cols, row0, col0;
};

class lvl_PlayerSequencing : public Level
{
    public:
    std::vector<Light> LightGrid;
    int GridRows = 1, GridCols = 1;
    LightVisual GridLV;
    Light GridClearLt;

    // method 1: Load all patternData at init
    LightPlayer2 LtPlay2;
    std::vector<patternData> pattVec;// container for all
    std::vector<LP2data> LP2dataVec;
    size_t pattSetIdx = 0;
    bool initPlayer2( const char* fileName );
    void nextPatternSet( LightPlayer2& LP2, size_t setIdx );// from LP2dataVec[setIdx]
    // a 2nd player as overlay
    LightPlayer2 LtPlay2_Over;
    std::vector<patternData> pattVec_Over;
    std::vector<LP2data> LP2data_OverVec;// container for overlay singles
    size_t pattSetIdx_Over = 0;
    // for the overlay
    void newPatternSet( LightPlayer2& LP2, const LP2data& LPdata );// from LPdata

    // method 2: Load patternData from a file on demand
    LightPlayer2 LtPlay2_a;
    std::vector<patternData> pattVec_a;// container for the current set, for overlay singles
    bool initPlayer2_a( const char* fileName );
    std::vector<std::string> Play2_aFilenames;
    // next pattern set read from file
    bool nextPatternSet( LightPlayer2& LP2, const char* fileName );// false if no such file
    size_t fNameIter_a = 0;// assigned by patternSetRB_a radio button control

    // on a separate array
    std::vector<Light> LightGrid_a;
    int GridRows_a = 1, GridCols_a = 1;
    LightVisual GridLV_a;
    Light GridClearLt_a;

    // controls
    radioButton patternSetRB, patternSetRB_Over;// select which pattern set
    pushButton patternPB;// play 1 pattern once through
    radioButton patternSetRB_a;//, patternSetRB_aOver;
 //   pushButton patternPB_a;
    bool init_controls( const char* fileName );

    virtual bool init();
    virtual bool handleEvent( sf::Event& rEvent );
    virtual void update( float dt );
    virtual void draw( sf::RenderTarget& RT ) const;

    lvl_PlayerSequencing(){}
    virtual ~lvl_PlayerSequencing(){}

    protected:

    private:
};

#endif // LVL_PLAYERSEQUENCING_H
