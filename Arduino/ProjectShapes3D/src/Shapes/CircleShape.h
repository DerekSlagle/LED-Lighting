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

    // half Radius? 0.5 only
    if( (int)(2*Radius) == 1 )
      return drawHalf();

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
        if( r*r + c*c <= Radius*Radius )
          pRow[c] = LtClr;
      }
    }
  }

  // for case Radius = 0.5f or so
  // draw 4 Lights
  void drawHalf()const
  {
    int Rc = pos.y, Cc = pos.x;
    // one by one
    if( (Rc >= 0 && Rc < gridRows) && (Cc >= 0 && Cc < gridCols)  )
      pLt0[ Rc*gridCols + Cc ] = LtClr;
    // col to right
    if( (Rc >= 0 && Rc < gridRows) && (Cc+1 >= 0 && Cc+1 < gridCols)  )
      pLt0[ Rc*gridCols + Cc+1 ] = LtClr;
    // 2nd row
    if( (Rc+1 >= 0 && Rc+1 < gridRows) && (Cc >= 0 && Cc < gridCols)  )
      pLt0[ (Rc+1)*gridCols + Cc ] = LtClr;
    // col to right
    if( (Rc+1 >= 0 && Rc+1 < gridRows) && (Cc+1 >= 0 && Cc+1 < gridCols)  )
      pLt0[ (Rc+1)*gridCols + Cc+1 ] = LtClr;   
  }

    CircleShape(){}
    virtual ~CircleShape(){}
};

#endif // CIRCLESHAPE_H
