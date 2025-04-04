#ifndef LVL_LIGHTS_H
#define LVL_LIGHTS_H

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

#include "../button_types/buttonValOnHit.h"
#include "../button_types/controlSurface.h"

class lvl_Lights : public Level
{
    public:
 //   std::vector<Light> LightArr;
    Light onLight, offLight;
    // 1st kind
 //   LightPlayer LtPlay;
 //   LightVisual LtPlayLV;

    // 2nd type
    std::vector<Light> LightArr2;// on another array
    LightPlayer2 LtPlay2;
    LightVisual LtPlay2LV;
    // storage for player pattern data
    std::vector<patternData> pattDataVec;
    bool initPlayer2( const char* fileName );
    // storage for pattern #100
 //   std::vector<uint8_t> patt100Vec;// numLts*patternLength elements

    // WavePlayer
    std::vector<Light> LightArrWave;
    WavePlayer wvPlay;
    LightVisual wvPlayLV;
    std::vector<float> wvC_Rt, wvC_Lt;
    bool init_wvPlay( const char* fileName );

    // pulses
    std::vector<Light> LightArrPulse;
    PulsePlayer pulsePlay;
    LightVisual pulsePlayLV;
    bool initPulse( const char* fileName );
    // graph shape of pulse above pulse strip
    std::vector<float> pulse_yVec;// y: 0.0f to 1.0f
    float graphAmp = 40.0f;
    std::vector<sf::Vertex> pulseGraph;// line strip
    sf::Vertex graphAxis[2];// at y = 0.0f
    // animate position of pulsePlay by controlling its tElap
    int idxLt = 10, idxRt = 54;
    float tElap_ani = 2.0f;
    bool ani_rev = false;

    // a 2nd pulse player
    PulsePlayer pulsePlay2;// will draw after pulsePlayer

    // a 4 color pattern player
    std::vector<Light> LightArr4;// on another array
    LightPlayer4 LtPlay4;
    LightVisual LtPlay4LV;
    // storage for player pattern data
    std::vector<patternData4> pattDataVec4;
    bool initPlayer4( const char* fileName );
    // storage for pattern #100
    std::vector<uint8_t> patt100Vec4;

    // a DataPlayer with a grid
    DataPlayer DataPlay;
    std::vector<Light> DataLightArr;
    LightVisual DataPlayLV;
    std::vector<uint8_t> DataVec;// pattern 100
    bool initDataPlay( const char* fileName );

    // LightMovers
    std::vector<Light> LightArrMove;
    std::vector<LightMover> LtMoverVec;
    LightVisual LtMoverLV;
    Light moveOffLight;// background color
    float move_tShift = 0.1f;
    bool initMovers( const char* fileName );

    // throttle updates
    unsigned int stepDelay = 4, stepIdx = 0;

    // controls
    controlSurface multiCS;
    multiSelector* init_controlMS( controlSurface& rCS, buttonValOnHit*& pStrip, sf::Vector2f HBpos, sf::Vector2f HBsz, sf::Vector2f sfcOfst, std::istream& is );
    bool init_MultiControl( multiSelector* pMS, buttonValOnHit* pStrip, std::istream& is );
    bool init_controls( const char* fileName );

    virtual bool init();
    virtual bool handleEvent( sf::Event& rEvent );
    virtual void update( float dt );
    virtual void draw( sf::RenderTarget& RT ) const;

    lvl_Lights();
    virtual ~lvl_Lights();

    protected:

    private:
};

#endif // LVL_LIGHTS_H
