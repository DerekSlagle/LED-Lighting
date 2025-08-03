#include "LightPanel.h"

bool LightPanel::init( Light *p_Src0, int GridRows, int GridCols, int SrcRows, int SrcCols, int SrcRow0, int SrcCol0, int TgtRows, int TgtCols )
{  
    if( !p_Src0 ) return false;// nullptr

    pSrc0 = p_Src0;
    gridRows = GridRows;
    gridCols = GridCols;
    srcRows = SrcRows; 
    srcCols = SrcCols; 
    row0 = SrcRow0; 
    col0 = SrcCol0;
    tgtRows =  TgtRows; 
    tgtCols = TgtCols;
    if( srcRows*srcCols != tgtRows*tgtCols ) return false;// number of Lights must be equal
    if( srcRows == tgtRows || srcCols == tgtCols ) return true;// matching
    if( srcRows == tgtCols || srcCols == tgtRows ) return true;// cross wise
    return false;
  }

void LightPanel::update()const// write to target array
{
  if( !pTgt0 ) return;
  
  if( rotIdx == 0 )// no rotation
  {
     const Light* pSrcBase = pSrc0 + row0*gridCols + col0;
    if( srcRows == tgtRows )// dimensions are the same
    {
      for( int r = 0; r < srcRows; ++r )
      {
          const Light* pSrcRow = pSrcBase + r*gridCols;
          Light* pTgtRow = pTgt0 + r*tgtCols;
          for( int c = 0; c < srcCols; ++c )
          {
              pTgtRow[c] = pSrcRow[c];
          }
      }
    }
    else// dimensions are perpendicular
    {
      updateSideways();
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

void LightPanel::updateSideways()const// if tgtCols == srcRows, etc
{
  const Light* pSrcBase = pSrc0 + row0*gridCols + col0;// read only
  

  for( int r = 0; r < srcRows; ++r )
  {
    const Light* pSrcRow = pSrcBase + r*gridCols;

   // Light* pTgtCol = pTgt0 + r;// backwards
    // a 2nd try
    Light* pTgtCol = pTgt0 + r + ( tgtRows - 1 )*tgtCols;

    for( int c = 0; c < srcCols; ++c )
    {
   //   pTgtCol[ c*tgtCols ] = pSrcRow[c];// backwards
      // 2nd try
      pTgtCol[ -c*tgtCols ] = pSrcRow[c];
    }
  }
}

void LightPanel::reverseOddRows( bool inTarget )const// for type = 2
{
  Light* itBegin = inTarget ? pTgt0 : pSrc0;
  for( int r = 0; r < tgtRows; r += 2 )
  {
    Light *itLt = itBegin + r*tgtCols;
    Light* itRt = itLt + tgtCols - 1;
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

// entire panel
void LightPanel::rotateCW()const// rotate image 90 degrees clockwise
{
  if( tgtRows != tgtCols ) return;// square panels only

  const Light* pSrcBase = pSrc0 + row0*srcCols + col0;// Light in source
    Light* pLtCorner = pTgt0 + tgtCols - 1;// lower left corner of target

    for( int r = 0; r < tgtRows; ++r )
    {
        const Light* pSrcRow = pSrcBase + r*srcCols;// source        
        for( int c = 0; c < tgtCols; ++c )
        {
          Light* pTgtRow = pLtCorner + c*tgtCols;// target
          pTgtRow[-r] = pSrcRow[c];
        }
    }
}

  void LightPanel::rotateCCW()const// 90 degrees counter clockwise
  {    
    if( tgtRows != tgtCols ) return;// square panels only

    const Light* pSrcBase = pSrc0 + row0*srcCols + col0;// Light in source
    Light* pLtCorner = pTgt0 + tgtCols*( tgtRows - 1 );// lower left corner of target

    for( int r = 0; r < tgtRows; ++r )
    {
        const Light* pSrcRow = pSrcBase + r*srcCols;// source             
        for( int c = 0; c < tgtCols; ++c )
        {          
          Light* pTgtRow = pLtCorner - c*tgtCols;// target 
          pTgtRow[r] = pSrcRow[c];
        }
    }
  }

  void LightPanel::rotate180()const// rotate image 180 degrees
  {
    const Light* pSrcBase = pSrc0 + row0*srcCols + col0;// Light in source
    Light* pLtCorner = pTgt0 + tgtRows*tgtCols - 1;// lower right corner of target

    for( int r = 0; r < tgtRows; ++r )
    {
        const Light* pSrcRow = pSrcBase + r*srcCols;// source   
        Light* pTgtRow = pLtCorner - r*tgtCols;// target     
        for( int c = 0; c < tgtCols; ++c )
        {          
          pTgtRow[-c] = pSrcRow[c];
        }
    }
  }