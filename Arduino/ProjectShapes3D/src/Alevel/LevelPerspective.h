#ifndef LEVELPERSPECTIVE_H
#define LEVELPERSPECTIVE_H

#include "BaseLevel.h"
#include "../perspect3D/persCircle.h"
#include "../perspect3D/persRectangle.h"
#include "../perspect3D/persLine.h"// not a persPt type
#include "MenuPage.h"

class LevelPerspective : public BaseLevel
{
    public:
    // lots of inherited members

    // 3D
    const int numCircles = 5;
    persCircle circleArr[ 5 ];
    persRectangle rectArr[2];
    persLine lineArr[4];

    const int numShapes = 10;
    persPt* pPersShape[ 10 ];// all
    persPt* pPersSorted[ 10 ];// doDraw only
    int numToDraw = 0;// found in update() used in draw()

    // input
    MenuPage thePage;
    FloatLine FL_CamPosX, FL_CamPosY, FL_CamPosZ;
    FloatLine FL_Yaw, FL_Pitch, FL_Roll, FL_Z0;
    float compassHeading = 0.0f, pitchAngle = 0.0f, rollAngle = 0.0f;

    void setup();
    void setup3D();
    
    virtual void updateDisplay()const;
    virtual void processInput();
    
    virtual void update( float dt );
    virtual void draw()const;

    LevelPerspective(){}
    virtual ~LevelPerspective(){}
};

#endif // LEVELPERSPECTIVE_H
