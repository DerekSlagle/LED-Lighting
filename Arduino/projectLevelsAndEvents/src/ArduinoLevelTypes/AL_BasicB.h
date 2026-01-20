#ifndef AL_BASICB_H
#define AL_BASICB_H

#include "Arduino.h"// for type String in updateDisplay()
#include "ArduinoLevel.h"
#include "../LightSourceTypes/LightGrid.h"
#include "../SSD_1306Component.h"

#include "MenuMultiPage.h"
#include "IntegerLine.h"

class AL_BasicB : public ArduinoLevel
{
    public:
    // MUST HAVE members
    LightGrid Target_LG;// owned by user
    void bindToGrid( Light* pGrid0, int GridRows, int GridCols )
    { Target_LG.init( *pGrid0, GridRows, GridCols ); }

    Light clearLt;// not must have just useful

    // for this level type
    float tElap = 0.0f;
    float tFrame = 0.5f;

    // a page manager
    MenuMultiPage MMP;
    // for use with menu
    static const int numPages = 3;
    MenuPage Page[ numPages ];
    bool gotoPage[ numPages ];
    // page 0: Home page
    //       value A, B
    int iVal_0A = 0, iVal_0B = 0;// for lines to
    bool bVal_0A = false, bVal_0B = false;// point to
    IntegerLine IL_0A, IL_0B;
    // goto page: 1, 2
    MenuLine ML_01, ML_02;// for Home page
    MenuLine ML_Quit;

    // page 1
    int iVal_1A = 0, iVal_1B = 0;// for lines
    bool bVal_1A = false, bVal_1B = false;// point to
    IntegerLine IL_1A, IL_1B;
    MenuLine ML_1home;// goto 0
    // page 2
    int iVal_2A = 0, iVal_2B = 0;// for lines
    bool bVal_2A = false, bVal_2B = false;// point to
    IntegerLine IL_2A, IL_2B;
    MenuLine ML_21;// goto page 1
    MenuLine ML_2home;// goto page 0

    SSD1306_Display* pDisplay = nullptr;
    void updateDisplay()const;
    bool DoUpdateOled = false;// menu writes

    bool setup( SSD1306_Display* p_Oled );
    virtual bool update( float dt );
    virtual void draw()const;
    virtual bool handleEvent( ArduinoEvent& AE );

    AL_BasicB(){}
    virtual ~AL_BasicB(){}
};

#endif // AL_BASICB_H
