#ifndef LINEPULSER_H
#define LINEPULSER_H

#include<cmath>
#include "Light.h"
#include "vec2f.h"

class LinePulser
{
    public:
    Light* pLt0 = nullptr;// to LightArr
    int gridRows = 1, gridCols = 1;// dimensions of LightArr array
    Light hiLt, loLt;
    
    float tElap = 0.0f;
    float speed = 100.0f;// in Lights per second
    float pulseWidth = 2.0f;// in Lights
    float rDist = 50.0f;// no fade for this distance = speed*tElap
    float fadeDist = 4.0f;// fade occurs for Dist < R < fadeDist
    float Amp = 1.0f;// limit blending of hiLt and loLt

    // origin and direction
    vec2f Pa, Pb;// the line ends
    vec2f Ta, Tb;// direction from corresponding point
    float Sa = 1.0f, Sb = 1.0f;// distance scale, eg: Qa = Pa + ( Dist*Sa )*Ta

    // cached
    vec2f Tp;// unit vector along pulse in direction Pb - Pa
    vec2f Tw;// unit vector in direction of propagation

    void initToGrid( Light* p_Lt0, int GridRows, int GridCols )
    { pLt0 = p_Lt0; gridRows = GridRows; gridCols = GridCols; }

    void setLineEnds( int rowA, int colA, int rowB, int colB )
    {
        Pa.x = (float)colA; Pa.y = (float)rowA;
        Pb.x = (float)colB; Pb.y = (float)rowB;
        // Ta, Tb = left heand norm of Pb - Pa by default
        Tp = Pb - Pa;
        Tp /= Tp.mag();// unit along pulse
        Ta.x = Tp.y;
        Ta.y = -Tp.x;
        Tb = Tw = Ta;// all same by default
        Sa = Sb = 1.0f;
    }

    void setPulseProps( float Speed, float PulseWidth, float TravelDist, float FadeDist )
    {
        speed = Speed;// center radius = ringSpeed*tElap
        pulseWidth = PulseWidth;// Light spaces
        rDist = TravelDist;// no fade for high value
        fadeDist = FadeDist;// fade rate
    }

    bool isPlaying = false;
    bool isVisible = false;// allows off grid wave to grow
    bool onePulse = true;
    bool isRadiating = false;// wave
    int direction = 1;
    float stopTime = 0.0f;// no write for R < stopTime*ringSpeed
    void StopWave()
    {
        stopTime = 0.0f;
        isRadiating = false;
    }

    void Start()
    {
        stopTime = 0.0f;
        isPlaying = true;
        isVisible = false;
        if( onePulse )
        {
            tElap = 0.0f;
            isRadiating = false;
        }
        else// radiating wave
        {
            tElap = 0.0f;
            isRadiating = true;
        }
    }

    bool update( float dt );// true if animating
    // for each process
    void updatePulse( float dt );
    void updateWave( float dt );

    LinePulser(){}
    ~LinePulser(){}

    private:
};

#endif // LINEPULSER