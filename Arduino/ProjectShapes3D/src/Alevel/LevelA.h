#ifndef LEVELA_H
#define LEVELA_H

#include "BaseLevel.h"
#include "LightTypes/WavePlayer2.h"
#include "MenuPage.h"
#include "FloatLine.h"

class LevelA : public BaseLevel
{
    public:
    // lots of inherited members
    WavePlayer2 theWP;
    float Coeff_Lt[4], Coeff_Rt[4];
    bool graphMode = false;

    // input to control theWP members
    MenuPage thePage;
    FloatLine FL_wvLenLt, FL_wvSpdLt;
    FloatLine FL_wvLenRt, FL_wvSpdRt;
    FloatLine FL_AmpRt;

    void setup();
    
    virtual void updateDisplay()const;
    virtual void processInput();
    
    virtual void update( float dt );
    virtual void draw()const;

    LevelA(){}
    virtual ~LevelA(){}
};

#endif // LEVELA_H
