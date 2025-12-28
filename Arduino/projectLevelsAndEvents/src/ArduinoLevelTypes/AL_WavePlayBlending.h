#ifndef AL_WAVEPLAYBLENDING_H
#define AL_WAVEPLAYBLENDING_H

#include "Arduino.h"// for type String in updateDisplay()

#include "ArduinoLevel.h"
#include "../LightTypes/WavePlayer2.h"
#include "../LightSourceTypes/LightGrid.h"
#include "FileParser.h"
#include "SSD_1306Component.h"// use a display

class AL_WavePlayBlending : public ArduinoLevel
{
    public:
        // MUST HAVE members
    LightGrid Target_LG;// owned by user
    void bindToGrid( Light* pGrid0, int GridRows, int GridCols )
    { Target_LG.init( *pGrid0, GridRows, GridCols ); }

    // WavePlayer2. Rotate which is playing
    int numWP = 0;// until allocated. Value from file
    WavePlayer2* wvPlay = nullptr;// allocate array
 //   int numWPinUse = 0;// init_SD()
    // seies coefficients for each player
    int maxCoeffs = 3;// up to 2 harmonics each way
    // array size = 2*numWP*maxCoeffs
    float* wv_Coeff = nullptr;// use a formula for offset to each players left and right coefficients
    int currWP = 0;// index to currently playing
    // player timing
    float tPeriodWP = 5.0f, tElapWP = 0.0f;// play duration and timer
    float tTransWP = 1.0f;// time for transition to next player
    // a buffer for the next wave player during transition. Lights allocated via new
    LightGrid BufferGrid;// wvPlay[ nextWP ] writes to this array
    Light clearLt;// assign before setup()
    bool init_WavePlayer( WavePlayer2& WP, const char* fileName );
    void blendBuffers( float U, float dt );

    // for use with menu
    int menuButtID = 2;
    int rotEncID = 1;
    float rotEncScale = 0.5f;
    float timeScale = 1.0f;
    int numOptions = 4;// Adjust tPeriodWP, tTransWP, timeScale and Quit
    int menuIter = 0;
    SSD1306_Display* pDisplay = nullptr;
    void updateDisplay()const;

    bool setup( const char* setupFilename, SSD1306_Display* p_Display );
    virtual bool update( float dt );
    virtual void draw()const;
    virtual bool handleEvent( ArduinoEvent& rEvent );

    AL_WavePlayBlending(){}
    virtual ~AL_WavePlayBlending()
    {
        if( wvPlay )
        {
            if( numWP == 1 ) delete wvPlay;
            else delete [] wvPlay;
        }

        if( wv_Coeff ) delete [] wv_Coeff;
    }

    protected:

    private:
};

#endif // AL_WAVEPLAYBLENDING_H
