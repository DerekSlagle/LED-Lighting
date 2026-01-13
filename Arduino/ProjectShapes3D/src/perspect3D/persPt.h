#ifndef PERSPT_H
#define PERSPT_H

#include "../vec2f.h"
#include "../Shapes/CircleShape.h"

class persPt
{
    public:
    static float X0, Yh;// view point in window
    static float Z0;// for perspective scaling of x and y coordinates
    static vec3f camPos;// camera position in scene
    static vec3f zuCam;// direction pointed
    static vec3f yuCam;    // unit vector "up" (camera orientation)
    static vec3f xuCam;    // unit vector "to right" : xu = camDir.cross( yu ) maintain in update()
    static const vec3f xHat, yHat, zHat;// these form the "world" or "global" coordinate vector basis
 //   const vec3f yHat(0.0f,1.0f,0.0f);
 //   const vec3f zHat(0.0f,0.0f,1.0f);
    static float angle;// yaw
    static float pitchAngle;
    static float rollAngle;

    static vec2f get_xyw( vec3f Pos );
    static float set_rw( vec3f Pos, float R, CircleShape& rCS );// utility. Returns scaled value of R
    static float changeCamDir( float dAy, float dAp, float dAr );// returns compass heading
    static float changeCamDir_mat( float dAy, float dAp, float dAr );// uses a matrix
    static void calibrateCameraAxes();// maintenance
    static void sortByDistance( persPt** pPtVec, size_t k );// use array of pointers. sort only 1st k elements
 //   static bool init_stat( std::istream& is );// initialize static members

    static float zScale( vec3f P );// Z0/Z

    // automate camera motion
    static void updatePOI( float R, float angleSpeed, float dt );
    static void pitchCam( float dAngle );
    static void yawCam( float dAngle );
    static void rollCam( float dAngle );
    static void keepXuLevel();// counter roll induced by pitching and yawing
    static void updateHorizonLine( vec2f& pt0, vec2f& pt1 );

    // regular members
    vec3f pos;// position in scene
    vec3f vel;
    vec3f Xu, Yu, Zu;// coordinate basis per instance
    float Rbound = 1.0f;// boundary in front of camera. Closer = don't draw
    bool doDraw = true;// calculate in update() and use in zSort() and draw()
    bool isMoving = false;// no need to update position of object while off screen or behind camera
    bool inUse = true;

    // change orientation of basis
    void pitch( float dAngle );
    void yaw( float dAngle );
    void roll( float dAngle );

    void init( vec3f Pos ){ pos = Pos; }
    vec2f get_xyw()const { return persPt::get_xyw( pos ); }
    float getDistance()const{ return ( pos - persPt::camPos ).mag(); }
    virtual void setPosition( vec3f Pos ){ pos = Pos; }
    void setPosition( float X, float Y, float Z ) { setPosition( vec3f(X,Y,Z) ); }
    virtual void update( float dt ) = 0;
    virtual void draw() const = 0;
    virtual void update_doDraw();
    virtual bool hit( vec3f posA, vec3f posB, vec3f& P, vec3f& vu )const;// also persQuad, persBox_quad, persMat
    virtual bool hit_image( float xHit, float yHit )const;// also persQuad
    bool isSighted()const;// in line with camDir?


    bool operator<( const persPt& rPt )const{ return getDistance() < rPt.getDistance(); }
    static bool compare( const persPt* pPtA, const persPt* pPtB ) { return *pPtB < *pPtA; }// for std::sort() on a container of pointers. This is operator <

    persPt(){ Xu = xHat; Yu = yHat; Zu = zHat; }
    virtual ~persPt(){}
};

#endif // PERSPT_H
