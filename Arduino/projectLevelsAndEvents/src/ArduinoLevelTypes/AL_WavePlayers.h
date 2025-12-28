#ifndef AL_WAVEPLAYERS_H
#define AL_WAVEPLAYERS_H

#include "Arduino.h"// for type String in updateDisplay()
#include "ArduinoLevel.h"
#include "../LightSourceTypes/LightGrid.h"
#include "../LightTypes/WavePlayer2.h"
#include "SSD_1306Component.h"// use a display

class AL_WavePlayers : public ArduinoLevel
{
    public:
    // MUST HAVE members
    LightGrid Target_LG;// owned by user
    void bindToGrid( Light* pGrid0, int GridRows, int GridCols )
    { Target_LG.init( *pGrid0, GridRows, GridCols ); }

    // members for this level
    WavePlayer2 theWP;
    // array storage
    float Coeff_Lt[4], Coeff_Rt[4];// fundamental + 3 harmonics

    // for use with menu
    int menuButtID = 2;
    int rotEncID = 1;
    float lengthScale = 10.0f, speedScale = 10.0f;// *rEvent.value
    float ampScale = 0.01f;
    int numOptions = 6;// len Rt, speed Rt, len Lt, speed Lt, AmpRt and Quit
    int menuIter = 0;
    SSD1306_Display* pDisplay = nullptr;
    void updateDisplay()const;

    bool setup( SSD1306_Display* p_Display );
    virtual bool update( float dt );
    virtual void draw()const;
    virtual bool handleEvent( ArduinoEvent& rEvent );


    AL_WavePlayers();
    virtual ~AL_WavePlayers();

    protected:

    private:
};

#endif // AL_WAVEPLAYERS_H
