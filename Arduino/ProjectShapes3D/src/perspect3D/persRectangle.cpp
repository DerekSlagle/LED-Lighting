#include "persRectangle.h"

// persQuad
void persRectangle::init( vec3f Pos, float W, float H, vec3f nu, Light color )
{
 //   pTxt = p_Txt;
    w = W; h = H;
    pos = Pos;
    Rbound = sqrtf( W*W + H*H )/2.0f;

    theRect.LtClr = color;
    setNu(nu);
}

void persRectangle::setColor( Light color )
{
    theRect.LtClr = color;
}

void persRectangle::setRectCorners()// assign pos, Sz, Xu, Yu from the vtx[4]
{
    theRect.pos = vtx[1];
    theRect.Sz.x = vtx[2].x - vtx[1].x;
    if( theRect.Sz.x < 0.0f ) theRect.Sz.x *= -1.0f;
    theRect.Sz.y = vtx[0].y - vtx[1].y;
    if( theRect.Sz.y < 0.0f ) theRect.Sz.y *= -1.0f;

    theRect.Xu = vtx[2] - vtx[1];
    theRect.Xu /= theRect.Xu.mag();
    theRect.Yu = vtx[0] - vtx[1];
    theRect.Yu /= theRect.Yu.mag();
}

void persRectangle::setPosition( vec3f Pos )
{
    vec3f dPos = Pos - pos;
    pos = Pos;
    for( size_t i = 0; i < 4; ++i )
    {
        pt[i] += dPos;
        vtx[i] = persPt::get_xyw( pt[i] );
    }
    setRectCorners();
    update_doDraw();
}

void persRectangle::update( float dt )
{
    if( isMoving ) pos += vel*dt;
    update_doDraw();

    if( !doDraw ) return;

    if( facingCamera )
    {
        vec3f N = persPt::camPos - pos;
        N /= N.mag();
        setNu( N );
    }
    else
    {
        if( isMoving )
        {
            vec3f dPos = vel*dt;
            for( size_t i = 0; i < 4; ++i )
                pt[i] += dPos;
        }

        for( size_t i = 0; i < 4; ++i )
            vtx[i] = persPt::get_xyw( pt[i] );

        setRectCorners();
    }
}

void persRectangle::draw() const
{
    if( !doDraw ) return;
    theRect.draw();
}

void persRectangle::setNu( vec3f nu )
{
    // from init() code
    Zu = nu;
    if( Zu.y > 0.9f || Zu.y < -0.9f )// vertical
    {
        Xu = persPt::xHat;
        Yu = persPt::zHat;
    }
    else// make level
    {
        Xu = Zu.cross( persPt::yHat );
        Xu /= Xu.mag();
        Yu = Xu.cross( Zu );
    }

    vec3f Pos = pos;
    Pos -= (w/2.0f)*Xu;
    Pos -= (h/2.0f)*Yu;
    pt[0].x = pt[0].y = pt[0].z = 0.0f;// lower left
    pt[1] = h*Yu;// up left
    pt[3] = w*Xu;// lower right
    pt[2] = pt[1] + pt[3];// up right
    for( size_t i = 0; i < 4; ++i )
    {
        pt[i] += Pos;
        vtx[i] = persPt::get_xyw( pt[i] );
    }

    setRectCorners();
}

// unit length of nu and Tu is assumed here
void persRectangle::setOrientation( vec3f nu, vec3f Tu )
{
    Zu = nu;
    Xu = Zu.cross( Tu );// in W direction
    Yu = Xu.cross( Zu );
    float H = 0.5f*h, W = 0.5f*w;
    pt[0] = pos + H*Yu - W*Xu;
    pt[1] = pos + H*Yu + W*Xu;
    pt[2] = pos - H*Yu + W*Xu;
    pt[3] = pos - H*Yu - W*Xu;
    for( size_t i = 0; i < 4; ++i )
        vtx[i] = persPt::get_xyw( pt[i] );

    setRectCorners();
}