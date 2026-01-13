#include "persLine.h"

// persLine
void persLine::init( vec3f StartPos, vec3f EndPos, Light color )
{
    startPos = StartPos;
    endPos = EndPos;
    theLine.LtClr = color;
    pt[0] = startPos;
    pt[1] = endPos;
    dir = pt[1] - pt[0];
    dir /= dir.mag();
 //   update(0.0f);
    theLine.pos = persPt::get_xyw( pt[0] );
    theLine.pos2 = persPt::get_xyw( pt[1] );
}

void persLine::update( float dt )
{
    float sf = dir.dot( persPt::zuCam );
    float dDir = 1.0f;// increase or decrease dir component of pt positions
    if( sf < 0.0f ){ sf *= -1.0f; dDir = -1.0f; }
    if( sf < 0.1f ) sf = 0.1f;// NEW
   float U0 = ( startPos - persPt::camPos ).dot( persPt::zuCam )/sf;
   float U1 = ( endPos - persPt::camPos ).dot( persPt::zuCam )/sf;
   if( U0 < 0.0f && U1 < 0.0f )// line is behind camera
   {
 //      bool doDrawLast = doDraw;
       doDraw = false;
       return;
   }

   doDraw = true;
    if( U0 < 0.0f ) pt[0] = startPos - dDir*( U0 + 10.0f )*dir;// pt[0] is behind camera
    else pt[0] = startPos;
    theLine.pos = persPt::get_xyw( pt[0] );
    if( U1 < 0.0f ) pt[1] =   endPos - dDir*( U1 + 10.0f )*dir;// pt[1] is behind camera
    else pt[1] = endPos;
    theLine.pos2 = persPt::get_xyw( pt[1] );
}

void persLine::draw() const
{
    if( doDraw )
        theLine.draw();
}
