#ifndef AL_SHAPES_H
#define AL_SHAPES_H

#include "Arduino.h"// for type String in updateDisplay()
#include "ArduinoLevel.h"
#include "../LightSourceTypes/LightGrid.h"
#include "MenuPage.h"
#include "FloatLine.h"
#include "IntegerLine.h"
#include "Uint8_tLine.h"
#include "../Shapes/LineShape.h"

class AL_Shapes : public ArduinoLevel
{
    public:
    // MUST HAVE members? Or equivalent
    Light* pLt0;// owned by user
    int gridRows = 1, gridCols = 1;
    void bindToGrid( Light* pGrid0, int GridRows, int GridCols )
    { pLt0 = pGrid0; gridRows = GridRows; gridCols = GridCols; }

    // members for this level
    Light clearLt;
    LineShape theLine;

    // menu
    MenuPage thePage;
    FloatLine FL_LinePosX, FL_LinePosY;
    FloatLine FL_LineLength, FL_LineAngle;
    float lineLength = 10.0f, lineAngle = 0.0f;
    MenuLine ML_DrawMethod;
    MenuLine ML_Quit;
    void setupMenu();
    bool do_draw2 = false;

    bool setup();
    virtual bool update( float dt );
    virtual void draw()const;
    virtual bool handleEvent( ArduinoEvent& AE );

    AL_Shapes(){}
    virtual ~AL_Shapes(){}
};

#endif // AL_SHAPES_H
