#ifndef SHAPE_H
#define SHAPE_H

#include "../Light.h"
#include "../vec2f.h"

class Shape
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

  Light LtClr;
 // int row0 = 2, col0 = 8;
  vec2f pos;// float for row0, col0 above
    
  virtual void draw()const = 0;

    Shape(){}
    virtual ~Shape(){}
};

#endif // SHAPE_H
