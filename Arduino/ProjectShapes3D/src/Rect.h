#ifndef RECT_H
#define RECT_H

#include "Light.h"
#include "vec2f.h"

class Rect
{
    public:
  static Light* pLt0;
  static int gridRows, gridCols;
  static void bindToGrid( Light* p_Lt0, int GridRows, int GridCols )
  {
    pLt0 = p_Lt0;
    gridRows = GridRows;
    gridCols = GridCols;
  }

  int W=6, H=10;
  Light LtClr;
 // int row0 = 2, col0 = 8;
  vec2f pos;// float for row0, col0 above

  void setup( int Width, int Height, Light fillColor, int Row0, int Col0 )
  { W = Width; H = Height; LtClr = fillColor; pos.y = Row0; pos.x = Col0; }
    
  void draw()const
  {
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

    Rect(){}
    ~Rect(){}
};

#endif // RECT_H
