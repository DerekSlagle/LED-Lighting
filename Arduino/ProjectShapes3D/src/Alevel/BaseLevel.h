#ifndef BASELEVEL_H
#define BASELEVEL_H

#include "Arduino.h"// for type String in updateDisplay()
#include "../LightSourceTypes/LightGrid.h"
#include "../SSD_1306Component.h"
#include "../switchSPST.h"

class BaseLevel
{
    public:
    // MUST HAVE members
    LightGrid Target_LG;// owned by user
    void bindToGrid( Light* pGrid0, int GridRows, int GridCols )
    { Target_LG.init( *pGrid0, GridRows, GridCols ); }

    static switchSPST *pActButt, *pMenuButt, *pRotEncButt;
    static int* pRotEncDelta;
    static void setupBase( SSD1306_Display& r_Oled, switchSPST *p_ActButt,
    switchSPST *p_MenuButt, switchSPST *p_RotEncButt, int& rotEncDelta );

    Light clearLt;// not must have just useful    
    static SSD1306_Display* pDisplay;
    virtual void updateDisplay()const;
    bool ignoreInput = false;// true if not the level "in focus"
    virtual void processInput();

    // members unrelated to input and output to oled
    int rows = 1, cols = 1, row0 = 0, col0 = 0;
    void setTargetRectangle( int Rows, int Cols, int Row0, int Col0 )
    { rows = Rows; cols = Cols; row0 = Row0; col0 = Col0; }
    
    virtual void update( float dt );
    virtual void draw()const;

    BaseLevel(){}
    virtual ~BaseLevel(){}
};

#endif // BASELEVEL_H
