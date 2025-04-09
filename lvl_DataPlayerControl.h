#ifndef LVL_DATAPLAYERCONTROL_H
#define LVL_DATAPLAYERCONTROL_H

#include "Level.h"
#include "../Light_types/PlayerFuncs.h"
#include "../LightVisual.h"
#include "../button_types/radioButton.h"

class lvl_DataPlayerControl : public Level
{
    public:
    std::vector<Light> LightGrid;
    int GridRows = 1, GridCols = 1;
    LightVisual GridLV;
    Light GridClearLt;

    DataPlayer DP_base;
    std::vector<std::string> DP_filenames;
    std::vector<uint8_t> DataVec;// storage for bitArray data
    size_t fNameIdx = 0;
    bool initDataPlayers( const char* fileName );

    radioButton imageRadioButt;
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
