#ifndef LIGHTMOVER_H
#define LIGHTMOVER_H

#include<iostream>
#include "Light.h"

// A time based 1d effect
class LightMover
{
    public:
    Light* pLt0 = nullptr;
    unsigned int numLts = 1;
    unsigned int funcIdx = 0;// index to equation of motion
    Light onLt;
    bool doRepeat = true;

    float tElap = 0.0f, tLimit = 1.0f;
    void init( Light& rLt0, unsigned int NumLts );

    // animate position
    void update( float dt );
    void update( Light offLt, float dt );
    // assign position
    void moveTo( float u )const;// u: 0 to 1
    void moveTo( Light offLt, float u )const;// u: 0 to 1
    void Clear( Light offLt )const;// draw all as offLt

    int getOnIndex( float u )const;

    LightMover();
    ~LightMover();

    protected:

    private:
};

#endif // LIGHTMOVER_H
