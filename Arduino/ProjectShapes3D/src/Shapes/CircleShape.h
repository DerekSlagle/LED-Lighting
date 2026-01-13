#ifndef CIRCLESHAPE_H
#define CIRCLESHAPE_H

#include "Shape.h"

class CircleShape : public Shape
{
    public:
  float Radius = 1.0f;
  // member vec2f pos = center

  void setup( float radius, Light fillColor, int Row0, int Col0 )
  { Radius = radius; LtClr = fillColor; pos.y = Row0; pos.x = Col0; }
    
  virtual void draw()const
  {
    if( !pLt0 ) return;// no crash

    int R = Radius;
    int Rc = pos.y, Cc = pos.x;
    Light* pBase = pLt0 + Rc*gridCols + Cc;

    for( int r = -R; r <= R; ++r )
    {
      if( r + Rc < 0 ) continue;
      if( r + Rc >= gridRows ) break;
      Light* pRow = pBase + r*gridCols;

      for( int c = -R; c <= R; ++c )
      {
        if( c + Cc < 0 ) continue;
        if( c + Cc >= gridCols ) break;
        if( r*r + c*c <= R*R )
          pRow[c] = LtClr;
      }
    }
  }

    CircleShape(){}
    virtual ~CircleShape(){}
};

#endif // CIRCLESHAPE_H
