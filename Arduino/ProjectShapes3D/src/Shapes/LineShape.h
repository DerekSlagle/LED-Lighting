#ifndef LINESHAPE_H
#define LINESHAPE_H

#include "Shape.h"

class LineShape : public Shape
{
    public:
  // member vec2f pos = left end
    vec2f pos2;// right end
    bool doBlend = true;
    float blendU = 0.25f;// 0.0f to 1.0f

  void setup( Light fillColor, int Row0, int Col0, int RowF, int ColF )
  { LtClr = fillColor; pos.y = Row0; pos.x = Col0; pos2.y = RowF; pos2.x = ColF; }
    
  virtual void draw()const;
  void draw2()const;// Bresenham's raster algorithm

  vec2f get_dIter( vec2f line )const;// helper for draw3()
  void draw3()const;// anti alias
  
  void drawHorizontal( int r, int c0, int cf )const;
  void drawVertical( int c, int r0, int rf )const;

  LineShape(){}
  virtual ~LineShape(){}
};

#endif // LINESHAPE_H

/*
 virtual void draw()const
  {
    if( !pLt0 ) return;// no crash

    vec2f Udisp = pos2 - pos;
    float Length = Udisp.mag();
    Udisp /= Length;// unit vector
    vec2f Uperp = Udisp.get_LH_norm();// off each way by one for blended Light
    vec2f currPos = pos;
    
    while( (currPos - pos).mag() <= Length )
    {
        int cpx = (int)( currPos.x + 0.5f );
        int cpy = (int)( currPos.y + 0.5f );

        int n = gridCols*cpy + cpx;// center
        if( n >= 0 && n < gridRows*gridCols && cpx >= 0 && cpx < gridCols )
          pLt0[n] = LtClr;

        if( doBlend )
        {
          float blendW = 1.0f - blendU;
          // plus one each side
          cpx = (int)( currPos.x + Uperp.x + 0.5f );
          cpy = (int)( currPos.y + Uperp.y + 0.5f );
          n = gridCols*cpy + cpx;// center
          if( n >= 0 && n < gridRows*gridCols && cpx >= 0 && cpx < gridCols )
          {
            Light blendLt( 0.3f*LtClr.r + 0.7f*pLt0[n].r, 0.3f*LtClr.g + 0.7f*pLt0[n].g, 0.3f*LtClr.b + 0.7f*pLt0[n].b );
            pLt0[n] = blendLt;
          }
          // in -Uperp direction
          cpx = (int)( currPos.x - Uperp.x + 0.5f );
          cpy = (int)( currPos.y - Uperp.y + 0.5f );
          n = gridCols*cpy + cpx;// center
          if( n >= 0 && n < gridRows*gridCols && cpx >= 0 && cpx < gridCols )
          {
            Light blendLt( 0.25f*LtClr.r + 0.75f*pLt0[n].r, 0.25f*LtClr.g + 0.75f*pLt0[n].g, 0.25f*LtClr.b + 0.75f*pLt0[n].b );
            pLt0[n] = blendLt;
          }
        }
          
        currPos += 0.5f*Udisp;
    }
  }
*/