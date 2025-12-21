#ifndef AL_BASICA_H
#define AL_BASICA_H

#include "Arduino.h"// for type String in updateDisplay()
#include "ArduinoLevel.h"
#include "../LightSourceTypes/LightGrid.h"
#include "../SSD_1306Component.h"// use a display
//#include "../switchSPST.h"

class AL_BasicA : public ArduinoLevel
{
    public:
    // MUST HAVE members
    LightGrid Target_LG;// owned by user
    void bindToGrid( Light* pGrid0, int GridRows, int GridCols )
    { Target_LG.init( *pGrid0, GridRows, GridCols ); }

    // members for this level
    Light hiLt, loLt;
    bool drawHiLt = true;
    float tElap = 0.0f;
    float tFrame = 2.0f;
    int row0 = 0, col0 = 0;
    int rows = 6, cols = 12;
    // for use with menu
    int menuButtID = 2;
    int numOptions = 2;// Quit is last option
    int menuIter = 0;
    SSD1306_Display* pDisplay = nullptr;
    void updateDisplay()const;

    bool setup( SSD1306_Display* p_Display );
    virtual bool update( float dt );
    virtual void draw()const;
    virtual bool handleEvent( ArduinoEvent& rEvent );

    AL_BasicA(){}
    virtual ~AL_BasicA(){}
};

#endif // AL_BASICA_H
