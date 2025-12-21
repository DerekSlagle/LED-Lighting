#ifndef AL_BASICB_H
#define AL_BASICB_H

#include "Arduino.h"// for type String in updateDisplay()
#include "ArduinoLevel.h"
#include "../LightSourceTypes/LightGrid.h"
#include "../SSD_1306Component.h"

class AL_BasicB : public ArduinoLevel
{
    public:
    // MUST HAVE members
    LightGrid Target_LG;// owned by user
    void bindToGrid( Light* pGrid0, int GridRows, int GridCols )
    { Target_LG.init( *pGrid0, GridRows, GridCols ); }

    // for this level type
    int numQLt = 4;
    Light quadLt[4];
    int iterLt = 0;
    float tElap = 0.0f;
    float tFrame = 0.5f;
    int row0 = 0, col0 = 0;
    int rows = 12, cols = 8;
    // for use with menu
    int menuButtID = 2;
    int numOptions = 2;// Quit is last option
    int menuIter = 0;
    SSD1306_Display* pDisplay = nullptr;
    void updateDisplay()const;

    bool setup( SSD1306_Display* p_Oled );
    virtual bool update( float dt );
    virtual void draw()const;
    virtual bool handleEvent( ArduinoEvent& rEvent );

    AL_BasicB(){}
    virtual ~AL_BasicB(){}
};

#endif // AL_BASICB_H
