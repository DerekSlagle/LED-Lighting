
#ifndef POINTPLAYER_H
#define POINTPLAYER_H

#include "../Light.h"
#include "../FileParser.h"
#include "../vec2f.h"

// A zoomie! It leaves a faded trail and times out
class PointPlayer
{
    public:
    Light* pLt0 = nullptr;// to LightArr
    int gridCols = 1, gridRows = 1;// bounding grid
    void bindToGrid( Light* p_Lt0, int GridRows, int GridCols )
    { pLt0 = p_Lt0; gridRows = GridRows; gridCols = GridCols; }

    bool isPlaying = false;

    // zoomie traits
    vec2f pos0, pos, vel;
    float speed = 10.0f;// leds per second
    Light ptColor;
 //   float tPeriod = 3.0f, tFade = 1.0f, tElap = 0.0f;
    // a path to visit on grid only
    uint8_t* pathX = nullptr, *pathY = nullptr;// parallel arrays
    uint8_t numPoints = 0, currPoint = 0, nextPoint = 0;
    float currLength = 0.0f;// distance to next point
    float fadeLength = 8.0f;// leaves a trail
    vec2f uCurr, uPrev;// assign on switch over

    void assignPath( const char* fileName );
    void setup( uint8_t& pathX0, uint8_t& pathY0, uint8_t NumPoints, float speed, Light PtColor );
    void Start();

    bool update( float dt );// true if animating
 //   void update_tElapOnly( float dt );
    void draw()const;// update() = time update

    PointPlayer(){}
    ~PointPlayer(){}
};

#endif // POINTPLAYER_H
