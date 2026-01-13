#ifndef PERSCIRCLE_H
#define PERSCIRCLE_H

#include "persPt.h"
#include "../Shapes/CircleShape.h"

class persCircle : public persPt
{
public:
    CircleShape B;
    float mass = 1.0f;

    void reset();
    void reset( vec3f Pos, vec3f Vel );

    void update_rw();
 //   bool hitFixed( vec3f posA, float r1, vec3f posB, vec3f& P, vec3f& vu, float Cr = 1.0f )const;// *this is stationary
 //   bool hitFree( persBall& rB, float Cr, float dt );// both moving or 1 fixed (calls above). Collision is handled within function
 //   bool isSighted( vec3f& hitPt, float R = 0.0f )const;
    void init( vec3f Pos, float R0, Light color, vec3f Vel = vec3f() );
    virtual void setPosition( vec3f Pos );
    void setPosition( float X, float Y, float Z ) { setPosition( vec3f(X,Y,Z) ); }
    virtual void update( float dt );
    virtual void update( vec3f grav, float dt );// other version in shotBurst, persBallTimed, which are derived from persBall
    virtual void draw() const;
    persCircle(){}
    persCircle( vec3f Pos, float R0, Light color, vec3f Vel = vec3f() ){ init( Pos, R0, color, Vel ); }
    virtual ~persCircle(){}
};

#endif // PERSCIRCLE_H
