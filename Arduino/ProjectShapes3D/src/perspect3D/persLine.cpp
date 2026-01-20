#include "persLine.h"

// static function
void persLine::fillBetween( const LineShape& Line1, const LineShape& Line2 )
{
    vec2f u1 = Line1.pos2 - Line1.pos;
    float length1 = u1.mag();
    u1 /= length1;
    vec2f u2 = Line2.pos2 - Line2.pos;
    float length2 = u2.mag();
    u2/= length2;
    vec2f it1 = Line1.pos, it2 = Line2.pos;
    // cross iterator goes from it1 to it2
    vec2f uX = it2 - it1;
    float lengthX = uX.mag();// changes
    uX /= uX.mag();// changes
    vec2f itX = it1;

    while( true )
    {
        bool DoWrite1 = ( it1 - Line1.pos ).mag() <= length1;
        bool DoWrite2 = ( it2 - Line2.pos ).mag() <= length2;
        // make a crossing
        if( DoWrite1 || DoWrite2 )
        {
            uX = it2 - it1;
            lengthX = uX.mag();
            uX /= uX.mag();
            itX = it1;
            float sX = 0.0f;
            while( sX <= lengthX )
            {
                int r = ( itX.y );// + row0;
                int c = ( itX.x );// + col0;
                if( (c >= 0 && c < Shape::gridCols) && (r >= 0 && r < Shape::gridRows) )
                    Shape::pLt0[ r*Shape::gridCols + c ] = Line1.LtClr;// Blend!
                // for next iter
                itX += 0.5f*uX;// half steps assure no lamp is missed
                sX = ( itX - it1 ).mag();
            }

            // for next iteration
            if( DoWrite1 ) it1 += 0.5f*u1;
            if( DoWrite2 ) it2 += 0.5f*u2;
        }
        else break;// done
    }
}

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
