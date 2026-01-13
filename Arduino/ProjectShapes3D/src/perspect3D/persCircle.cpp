#include "persCircle.h"

// persBall

void persCircle::init( vec3f Pos, float R0, Light color, vec3f Vel )
{
    pos = Pos;
    Rbound = R0;
    vel = Vel;
    B.Radius = R0;
    B.LtClr =  color;
    Xu = xHat;
    Yu = yHat;
    Zu = zHat;
    update(0.0f);
}

void persCircle::setPosition( vec3f Pos )
{
    pos = Pos;
    update(0.0f);
}

void persCircle::update_rw()
{
    float U = ( pos - persPt::camPos ).dot( persPt::zuCam );
    if( U < 0.0f ) U *= -1.0f;
    float R = Rbound*persPt::Z0/U;
    B.Radius = R;
    B.pos = get_xyw();
}

void persCircle::update( float dt )
{
    if( !inUse ) return;

    if( isMoving ) pos += vel*dt;
    update_doDraw();
    if( doDraw ) update_rw();
}

void persCircle::update( vec3f grav, float dt )
{
    if( !inUse ) return;

    if( isMoving )
    {
        pos += ( vel + grav*dt/2.0f )*dt;
        vel += grav*dt;
    }

    update_doDraw();
    if( doDraw ) update_rw();
}

void persCircle::reset()
{
    inUse = true;
}

void persCircle::reset( vec3f Pos, vec3f Vel )
{
    inUse = true;
    vel = Vel;
    setPosition( Pos );
}

void persCircle::draw() const
{
    if( inUse && doDraw ) B.draw();
}