#include "LightPanel.h"

/*
void LightPanel::update()const// write to target array
{
    // Light in source
    const Light* pSrcBase = pSrc0 + row0*srcCols + col0;

    for( int r = 0; r < rows; ++r )
    {
        const Light* pSrcRow = pSrcBase + r*srcCols;
        Light* pTgtRow = pTgt0 + r*cols;
        for( int c = 0; c < cols; ++c )
        {
          if( rotIdx == 0 )
            pTgtRow[c] = pSrcRow[c];
          else
            *( mapLightPosition( r, c ) ) = pSrcRow[c];
        }
    }

    if( type == 2 ) reverseOddRows( true );
}
*/

void LightPanel::update()const// write to target array
{
  if( rotIdx == 0 )// no rotation
  {
     const Light* pSrcBase = pSrc0 + row0*srcCols + col0;
    for( int r = 0; r < rows; ++r )
    {
        const Light* pSrcRow = pSrcBase + r*srcCols;
        Light* pTgtRow = pTgt0 + r*cols;
        for( int c = 0; c < cols; ++c )
        {
            pTgtRow[c] = pSrcRow[c];
        }
    }

    if( type == 2 ) reverseOddRows( true );
     return;
  }

  // rotate
  switch( rotIdx )
  {
    case 1 :// 90 degrees cw
     rotateCW();
    break;
    case -1 :// 90 degrees ccw
     rotateCCW();
    break;

    case 2 :// 180 degrees
    case -2 :// 180 degrees
     rotate180();
    break;

    default: break;

  }

  if( type == 2 ) reverseOddRows( true );
     return;
}

void LightPanel::reverseOddRows( bool inTarget )const// for type = 2
{
  Light* itBegin = inTarget ? pTgt0 : pSrc0;
  for( int r = 0; r < rows; r += 2 )
  {
    Light *itLt = itBegin + r*cols;
    Light* itRt = itLt + cols - 1;
    while( itLt < itRt )
    {
      Light tempLt = *itLt;
      *itLt = *itRt;
      *itRt = tempLt;
      ++itLt;
      --itRt;
    }
  }
}

// r, c are relative to panel Light[0]
// returns pointer to Light in target at rotated position
Light* LightPanel::mapLightPosition( int r, int c )const
{
  if( rotIdx == 0 )// no rotation  
    return pTgt0 + r*cols + c;
  // rotate
  Light* pLtCorner = pTgt0;// home corner
  switch( rotIdx )
  {
    case 1 :// 90 degrees cw
    pLtCorner = pTgt0 + cols - 1;// upper right corner
    return pLtCorner + c*cols - r;
    case -1 :// 90 degrees ccw
    pLtCorner = pTgt0 + cols*( rows - 1 );// lower left corner
    return pLtCorner - c*cols + r;

    case 2 :// 180 degrees
    case -2 :// 180 degrees
    pLtCorner = pTgt0 + rows*cols - 1;// lower right corner
    return pLtCorner - r*cols - c;

    default :// no rotation
    return pTgt0 + r*cols + c;

  }

  return pTgt0 + r*cols + c;// execution should not reach here. JIC
}

// entire panel
void LightPanel::rotateCW()const// rotate image 90 degrees clockwise
{
  const Light* pSrcBase = pSrc0 + row0*srcCols + col0;// Light in source
    Light* pLtCorner = pTgt0 + cols - 1;// lower left corner of target

    for( int r = 0; r < rows; ++r )
    {
        const Light* pSrcRow = pSrcBase + r*srcCols;// source        
        for( int c = 0; c < cols; ++c )
        {
          Light* pTgtRow = pLtCorner + c*cols;// target
          pTgtRow[-r] = pSrcRow[c];
        }
    }
}

  void LightPanel::rotateCCW()const// 90 degrees counter clockwise
  {    
    const Light* pSrcBase = pSrc0 + row0*srcCols + col0;// Light in source
    Light* pLtCorner = pTgt0 + cols*( rows - 1 );// lower left corner of target

    for( int r = 0; r < rows; ++r )
    {
        const Light* pSrcRow = pSrcBase + r*srcCols;// source             
        for( int c = 0; c < cols; ++c )
        {          
          Light* pTgtRow = pLtCorner - c*cols;// target 
          pTgtRow[r] = pSrcRow[c];
        }
    }
  }

  void LightPanel::rotate180()const// rotate image 180 degrees
  {
    const Light* pSrcBase = pSrc0 + row0*srcCols + col0;// Light in source
    Light* pLtCorner = pTgt0 + rows*cols - 1;// lower right corner of target

    for( int r = 0; r < rows; ++r )
    {
        const Light* pSrcRow = pSrcBase + r*srcCols;// source   
        Light* pTgtRow = pLtCorner - r*cols;// target     
        for( int c = 0; c < cols; ++c )
        {          
          pTgtRow[-c] = pSrcRow[c];
        }
    }
  }