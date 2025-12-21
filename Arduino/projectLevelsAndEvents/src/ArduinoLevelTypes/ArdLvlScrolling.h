#ifndef ARDLVLSCROLLING_H
#define ARDLVLSCROLLING_H

#include "Arduino.h"// for type String in updateDisplay()
#include "ArduinoLevel.h"
#include "../LightSourceTypes/BufferSource.h"
#include "../LightSourceTypes/BuffSrcLoadfromFile.h"
#include "../SSD_1306Component.h"// use a display

// Only 1 instance in use at a time
// A Singleton
class ArdLvlScrolling : public ArduinoLevel
{
    public:
    // MUST HAVE members
    LightGrid Target_LG;// owned by user
    void bindToGrid( Light* pGrid0, int GridRows, int GridCols )
    { Target_LG.init( *pGrid0, GridRows, GridCols ); }

    // an animation on BufferSource with a LightGrid for each frame
    LightGrid* pAni_LG = nullptr;// delete in dtor
    int numAni_LG = 0;// until allocated in function below
    BufferSource AniBuff_Src;
    Light clearLt;

    // for use with menu
    int menuButtID = 2;
    int numOptions = 3;// Reduce Vx, reduce Vy and Quit
    int menuIter = 0;
    float speedScale = 0.5f;
    SSD1306_Display* pDisplay = nullptr;
    void updateDisplay()const;

    // define in every Level type
    bool setup( const char* setupFilename, SSD1306_Display* p_Display );
    virtual bool update( float dt );
    virtual void draw()const;
    virtual bool handleEvent( ArduinoEvent& rEvent );

    ArdLvlScrolling();
    virtual ~ArdLvlScrolling();

    protected:

    private:
};

#endif // ARDLVLSCROLLING_H
