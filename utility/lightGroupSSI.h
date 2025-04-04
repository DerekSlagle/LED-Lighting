#ifndef LIGHTGROUPSSI_H
#define LIGHTGROUPSSI_H

#include "myLight.h"
#include "spriteSheetImage.h"


class lightGroupSSI : public lightGroupGrid
{
    public:
    const spriteSheetImage* pSSI = nullptr;
    unsigned int setNum = 0, frDelay = 1;
    unsigned int frIdx = 0, delayCnt = 0;
    bool doAnimate = true;// play pause?
    float scale = 1.0f;// in writing to lights
    sf::IntRect srcRect;// assigned in update() and used in draw()const

    void update( float dt );// update frIdx, delayCnt and sceRect tElap?
    void update();// update frIdx, delayCnt and sceRect but not tElap
    // non const because frIdx, delayCnt and sceRect are changed
    void draw()const;// general scale and transparency

    void nextFrame( bool rndRobin = true );// assigns srcRect
    void prevFrame( bool rndRobin = true );
    void nextFrameSet( bool rndRobin = true );// assigns srcRect
    void prevFrameSet( bool rndRobin = true );

    void init( const spriteSheetImage& rSSI, unsigned int SetNum, myLight* pGrid, myLight* p_Lt0,
        unsigned int GridRows, unsigned int GridCols, unsigned int FrDelay, float Scale = 1.0f  );

    lightGroupSSI( const spriteSheetImage& rSSI, unsigned int SetNum, myLight* pGrid, myLight* p_Lt0,
        unsigned int GridRows, unsigned int GridCols, unsigned int FrDelay, float Scale = 1.0f  )
        { init( rSSI, SetNum, pGrid, p_Lt0, GridRows, GridCols, FrDelay, Scale ); }

    lightGroupSSI();
    virtual ~lightGroupSSI();

    protected:

    private:
    // called by draw() when it is time
    void drawNow()const;// general: branches to below

    void drawTransparent()const;
    void drawTrue()const;// scale = 1
    void drawTrueTransparent()const;// scale = 1
};

#endif // LIGHTGROUPSSI_H
