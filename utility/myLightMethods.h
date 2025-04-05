#ifndef MYLIGHTMETHODS_H_INCLUDED
#define MYLIGHTMETHODS_H_INCLUDED

#include<vector>
#include<iostream>
#include<functional>
#include "myLight.h"

// on a group
// same effect on all: discrete group or contiguous storage
void blinkGroup( myLight* pLt0, unsigned int numLts, float& tElap, float onTime, float offTime, float dt,
        float tRamp = 0.0f, std::function<float(float)> Y = nullptr  );
//void blinkGroup( std::vector<myLight*>& pLightVec, float& tElap, float onTime, float offTime, float dt );
void blinkGroup( lightGroup& LtGrp, float onTime, float offTime, float dt,
        float tRamp = 0.0f, std::function<float(float)> Y = nullptr );

bool dimGroup( myLight* pLt0, unsigned int numLts, float& tElap, float rate, float dt );// false if fully dimmed already
//bool dimGroup( std::vector<myLight*>& pLightVec, float rate, float dt );// false if fully dimmed already
bool dimGroup( lightGroup& LtGrp, float rate, float dt );

bool brightenGroup( myLight* pLt0, unsigned int numLts, float& tElap, float rate, float dt );// false if fully bright already
//bool brightenGroup( std::vector<myLight*>& pLightVec, float rate, float dt );// false if fully bright already
bool brightenGroup( lightGroup& LtGrp, float rate, float dt );

// apparent motion across the group
// cycles states round robin
void shiftGroup( std::vector<myLight*>& pLtVec, float& tElap, float tShift, float dt );
void shiftGroup( myLight* pLt0, unsigned int numLts, float& tElap, float tShift, float dt );
void shiftGroup( lightGroup& LtGrp, float dt );

// fills 1 by 1 across group. LtF "moves" across others in state Lt0. du = period between motion starts
// working
bool fillAcrossGroup( std::vector<myLight*>& pLightVec, myLight Lt0, myLight LtF, float& tElap, float tTransit, float du, float dt, char Mode );
// animate 1 light across group. u: 0 to 1. true when complete
bool moveAcrossGroup( myLight* pLt0, unsigned int numLts, float& tElap, float tShift,
        std::function<float(float)> Y, myLight LtOn, myLight LtOff, float dt );
bool moveAcrossGroup( lightGroup& LtGrp, std::function<float(float)> Y, myLight LtOn, myLight LtOff, float dt );

// instant response
// 1 color fill
void vuMeter( myLight* pLt0, unsigned int numLts, myLight Lt0, myLight LtF, float u );
//void vuMeter( std::vector<myLight*>& pLightVec, myLight Lt0, myLight LtF, float u );
void vuMeter( lightGroup& LtGrp, myLight Lt0, myLight LtF, float u );
// 3 color fill. LtF1 0 to uCut1, LtF2 uCut1 to uCut2, LtF3 u > uCut2
void vuMeter( myLight* pLt0, unsigned int numLts, myLight Lt0,
    myLight LtF1, float uCut1, myLight LtF2, float uCut2, myLight LtF3, float u );
//void vuMeter( std::vector<myLight*>& pLightVec, myLight Lt0,
//    myLight LtF1, float uCut1, myLight LtF2, float uCut2, myLight LtF3, float u );// u: 0.0f to 1.0f
void vuMeter( lightGroup& LtGrp, myLight Lt0,
    myLight LtF1, float uCut1, myLight LtF2, float uCut2, myLight LtF3, float u );

// on single myLight
myLight interpolate( const myLight& Lt0, const myLight& LtF, float u );// u: 0.0f to 1.0f
// easing animation. returns true when complete.
// Mode: 'L' = linear, 'B' = Bezier
bool transit( const myLight& Lt0, const myLight& LtF, myLight& LtNow, float tElap, float tShift, float dt, char Mode );
// varies only brightness. true when complete
bool pulse( myLight& LtNow, float tElap, float tShift, float dt, char Mode );

#endif // MYLIGHTMETHODS_H_INCLUDED
