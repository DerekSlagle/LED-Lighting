#ifndef LVL_DATAPLAYERCONTROL_H
#define LVL_DATAPLAYERCONTROL_H

#include "Level.h"
#include "../Light_types/PlayerFuncs.h"
#include "../Light_types/WavePlayer.h"
#include "../Light_types/WavePlayer2.h"
#include "../Light_types/LightPlayer2.h"
#include "../LightVisual.h"
#include "../button_types/radioButton.h"

class lvl_DataPlayerControl : public Level
{
    public:
    std::vector<Light> LightGrid;
    int GridRows = 1, GridCols = 1;
    LightVisual GridLV;
    Light GridClearLt;

    // a WavePlayer
    DataPlayer DP_base;
    Light DP_Color[16];// external Light storage for type DataPlayer now
    std::vector<std::string> DP_filenames;
    std::vector<uint8_t> DataVec;// storage for bitArray data
 //   size_t fNameIdx = 0;
    bool initDataPlayers( const char* fileName );

    // a WavePlayer to layer upon
    WavePlayer wvPlay;
    bool init_wvPlay( const char* fileName );

    // a LightPlayer2 to layer upon
    LightPlayer2 pattPlay;
    std::vector<patternData> pattDataVec;// storage for patterns
    bool init_pattPlay( const char* fileName );
    std::vector<float> EQvals;

    // a 2nd grid
    std::vector<Light> LightGrid2;
    int GridRows2 = 1, GridCols2 = 1;
    LightVisual GridLV2;
    Light GridClearLt2;
    // for WavePlayer2
    WavePlayer2 wvPlay2;
    bool init_wvPlay2( const char* fileName );
    // init also in above
    DataPlayer DP_2;
    Light DP2_Color[16];
    std::vector<uint8_t> DataVec2;// storage for bitArray data

    radioButton imageRadioButt;
    buttonRect ZoomButt;// call updateZoomOut() if selected
    buttonRect PauseButt;// pause animation
    buttonRect OverlayButt;// pause animation
    bool init_controls( const char* fileName );

    virtual bool init();
    virtual bool handleEvent( sf::Event& rEvent );
    virtual void update( float dt );
    virtual void draw( sf::RenderTarget& RT ) const;

    lvl_DataPlayerControl(){}
    virtual ~lvl_DataPlayerControl(){}

    protected:

    private:
};

#endif // LVL_DATAPLAYERCONTROL_H
