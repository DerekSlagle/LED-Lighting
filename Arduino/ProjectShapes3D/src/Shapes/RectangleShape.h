#ifndef RECTANGLESHAPE_H
#define RECTANGLESHAPE_H

#include "Shape.h"

class RectangleShape : public Shape
{
    public:

 // int W=6, H=10;// vec2f Sz
//  Light LtClr;// in base
 // int row0 = 2, col0 = 8;// vec2f pos
  vec2f Sz;// float for W, H above
  vec2f Xu, Yu;// unit vector in row and column directions

  void setup( int Width, int Height, Light fillColor, int Row0, int Col0 )
  { 
    Sz.x = Width; Sz.y = Height; LtClr = fillColor; pos.y = Row0; pos.x = Col0;
    // default for directions
    Xu.x = 1.0f, Xu.y = 0.0f;
    Yu.x = 0.0f, Yu.y = 1.0f;
  }
    
  virtual void draw()const
  {
    if( !pLt0 ) return;// no crash
    
    int row0 = pos.y, col0 = pos.x;
    vec2f itTop = pos, itLeft = pos;// start in corner
    while( (itTop-pos).mag() < Sz.x )// along rows
    {
        itLeft = itTop;// top of column
        while( (itLeft-itTop).mag() < Sz.y )// along cols
        {
          int r = ( itLeft.y );// + row0;
          int c = ( itLeft.x );// + col0;
          if( (c >= 0 && c < gridCols) && (r >= 0 && r < gridRows) )
            pLt0[ r*gridCols + c ] = LtClr;

          itLeft += 0.5f*Yu;
        }

      itTop += 0.5f*Xu;// next column  
    }
    
  }

    RectangleShape(){}
    virtual ~RectangleShape(){}
};

#endif // RECTANGLESHAPE_H

/*
virtual void draw()const
  {
    if( !pLt0 ) return;// no crash
    
    int row0 = pos.y, col0 = pos.x;
    vec2f itTop = pos, itLeft = pos;// start in corner
    while( (itTop-pos).mag() < Sz.x )// along rows
    {
      int c = ( itTop.x ) + col0;
      if( c >= 0 && c < gridCols )
      {
        itLeft = itTop;// top of column
        while( (itLeft-itTop).mag() < Sz.y )// along cols
        {
          int r = ( itLeft.y ) + row0;
          if( r >= 0 && r < gridRows ) 
            pLt0[ r*gridCols + c ] = LtClr;
          itLeft += Yu;
        }
      }  
      itTop += Xu;// next column  
    }
    
  }

virtual void draw()const
  {
    if( !pLt0 ) return;// no crash
    
    int W = Sz.x, H = Sz.y;
    int row0 = pos.y, col0 = pos.x;
    Light* pBase = pLt0 + row0*gridCols + col0;

    for( int r = 0; r < H; ++r )
    {
      if( r + row0 < 0 ) continue;
      if( r + row0 >= gridRows ) break;
      Light* pRow = pBase + r*gridCols;
      for( int c = 0; c < W; ++c )
      {
        if( c + col0 < 0 ) continue;
        if( c + col0 >= gridCols ) break;
        pRow[c] = LtClr;
      }
    }
  }
*/