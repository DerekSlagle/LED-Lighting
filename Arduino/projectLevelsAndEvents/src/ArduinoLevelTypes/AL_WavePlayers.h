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
    bool graphMode = false;// toggle on Home page

    // for use with menu
    int menuButtID = 2;
    int rotEncID = 1;
    const int numPages = 4;// page 0 is to select others or Quit
    int numOptions[4] = { 5, 6, 7, 7 };
    int currPage = 0;// Home
    int menuIter = 0;

    // page 0 = Home. 5: List of other 3 pages + toggle graphMode + Quit
    String updatePage_0()const;
    void handleMenuEvent_0( ArduinoEvent AE  );

    // page 1
  //  int numOptions = 6;// len Rt, speed Rt, len Lt, speed Lt, AmpRt and Quit
    float lengthScale = 10.0f, speedScale = 10.0f;// factors *rEvent.value
    float ampScale = 0.01f;// for AmpRt
    String updatePage_1()const;
    void handleMenuEvent_1( ArduinoEvent AE  );

    // page 2 = Colors: r,g,b for left and right
    String updatePage_2()const;
    void handleMenuEvent_2( ArduinoEvent AE  );// 6 x type 2

    // page 3 = Coefficients: 3 each for left and right
    String updatePage_3()const;
    void handleMenuEvent_3( ArduinoEvent AE  );// 6 x type 2
    
    
    SSD1306_Display* pDisplay = nullptr;
    void updateDisplay()const;

    bool setup( SSD1306_Display* p_Display );
    virtual bool update( float dt );
    virtual void draw()const;
    virtual bool handleEvent( ArduinoEvent& AE );


    AL_WavePlayers();
    virtual ~AL_WavePlayers();

    protected:

    private:
};

#endif // AL_WAVEPLAYERS_H
