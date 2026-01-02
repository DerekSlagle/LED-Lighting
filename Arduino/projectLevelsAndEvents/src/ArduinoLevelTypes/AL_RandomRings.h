#ifndef AL_RANDOMRINGS_H
#define AL_RANDOMRINGS_H

#include "Arduino.h"// for type String in updateDisplay()
#include "ArduinoLevel.h"
#include "../LightSourceTypes/LightGrid.h"
#include "../LightTypes/RingPlayer.h"
#include "../SSD_1306Component.h"// use a display

#include "FloatLine.h"
#include "IntegerLine.h"

class AL_RandomRings : public ArduinoLevel
{
    public:
    // MUST HAVE members
    LightGrid Target_LG;// owned by user
    void bindToGrid( Light* pGrid0, int GridRows, int GridCols )
    { Target_LG.init( *pGrid0, GridRows, GridCols ); }
    Light clearLt;

    // RingPlayer
    static const int numRP = 30;
    int numRPplaying = 0;// varies with time
    RingPlayer ringPlay_Arr[numRP];
    float tStart = 0.14f;// Start() 1 per second
    float tElapStart = 0.0f;
    int idxStartNext = 0;

    // vary these numbers
    // 4x float
    float fadeRratio = 1.6f;
    FloatLine fl_fadeRratio;
    float fadeWratio = 1.6f;
    FloatLine fl_fadeWratio;
    float spawnTime = 0.5f;// average rate of 2 per second
    FloatLine fl_spawnTime;
    float speedFactor = 1.0f;// modulates randomly assigned value
    FloatLine fl_speedFactor;
    // 1x int
    int spawnBound = 48;
    IntegerLine IL_spawnBound;
    // 1x action: Light a test lamp when actButt held and this line selected
    bool doTestLamp = false;// to test MenuLine
    MenuLine ML_testLamp;

    // for use with menu
 //   int actButtId = 1;// inherited
    int menuButtID = 2;
    int rotEncID = 1;
    static const int numOptions = 7;// the 4 floats above, spawnBound, test lamp and Quit
    int menuIter = 0;
    SSD1306_Display* pDisplay = nullptr;
    void updateDisplay()const;
    // timed update for Home page
    int updateTime = 0;
    float tElapUp = 0.0f;// 1.0f limit in update()

    // define in every Level type
    bool setup( SSD1306_Display* p_Display );
    virtual bool update( float dt );
    virtual void draw()const;
    virtual bool handleEvent( ArduinoEvent& AE );

    AL_RandomRings(){}
    virtual ~AL_RandomRings(){}
};

#endif // AL_RANDOMRINGS_H
