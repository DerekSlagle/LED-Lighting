#ifndef PERSRECTANGLE_H
#define PERSRECTANGLE_H

#include "persPt.h"
#include "../Shapes/RectangleShape.h"

class persRectangle : public persPt
{
   public:
    // data members
    vec3f pt[4];
 //   vec3f Nu;// unit vector perpendicular to Quad
    vec2f vtx[4];// mapped corner positions
    RectangleShape theRect;
    void setRectCorners();// assign pos, Sz, Xu, Yu from the vtx[4]
 //   const sf::Texture* pTxt = nullptr;
    float w, h;// width and height of the quad
    bool facingCamera = false;// enables "billboarding"

    // function members
    vec3f getPosition()const{ return ( pt[0] + pt[2] )/2.0f; }
    virtual void update( float dt );
    virtual void draw() const;
    virtual void setPosition( vec3f Pos );
    void setNu( vec3f nu );// to default orientation: w side is horizontal, quad is tilted upwards
    void setOrientation( vec3f nu, vec3f Tu );// general orientation: pt[0], pt[1] toward Tu
   
    void setColor( Light color );

    persRectangle(){}
    virtual ~persRectangle(){}

    void init( vec3f Pos, float W, float H, vec3f nu, Light color );
    persRectangle( vec3f Pos, float W, float H, vec3f nu, Light color ){ init( Pos, W, H, nu, color ); }
};

#endif // PERSRECTANGLE_H
