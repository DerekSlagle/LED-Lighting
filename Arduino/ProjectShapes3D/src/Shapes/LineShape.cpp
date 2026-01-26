#include "LineShape.h"

void LineShape::draw()const
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
            // LtClr part of blend
            float redU = blendU*LtClr.r, greenU = blendU*LtClr.g, blueU = blendU*LtClr.b;
            float blendW = 1.0f - blendU;// existing color share in blend
            // plus one each side
            cpx = (int)( currPos.x + Uperp.x + 0.5f );
            cpy = (int)( currPos.y + Uperp.y + 0.5f );
            n = gridCols*cpy + cpx;// center
            if( n >= 0 && n < gridRows*gridCols && cpx >= 0 && cpx < gridCols )
            {
            Light blendLt( redU + blendW*pLt0[n].r, greenU + blendW*pLt0[n].g, blueU + blendW*pLt0[n].b );
            pLt0[n] = blendLt;
            }
            // in -Uperp direction
            cpx = (int)( currPos.x - Uperp.x + 0.5f );
            cpy = (int)( currPos.y - Uperp.y + 0.5f );
            n = gridCols*cpy + cpx;// center
            if( n >= 0 && n < gridRows*gridCols && cpx >= 0 && cpx < gridCols )
            {
            Light blendLt( redU + blendW*pLt0[n].r, greenU + blendW*pLt0[n].g, blueU + blendW*pLt0[n].b );
            pLt0[n] = blendLt;
            }
        }
            
        currPos += 0.5f*Udisp;
    }
}

// Bresenham's raster algorithm
void LineShape::draw2()const
{
    if( !pLt0 ) return;// no crash

    int8_t x0 = pos.x,  y0 = pos.y,  x1 = pos2.x,  y1 = pos2.y;
    // bound check
    int8_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int8_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy, e2;

    while (true) {
        pLt0[ x0 +  gridCols*y0 ] = LtClr; // Rasterize: set the pixel
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void LineShape::drawHorizontal( int r, int c0, int cf )const
{
    if( r < 0 || r >= gridRows ) return;// off grid
    if( c0 > cf ){ int k = c0; c0 = cf; cf = k; }// swap
    Light* pRow = pLt0 + r*gridCols;
    for( int c = c0; c <= cf; ++c )
    {
        if( c < 0 ) continue;
        else if( c >= gridCols ) break;
        pRow[c] = LtClr;
    }
}
void LineShape::drawVertical( int c, int r0, int rf )const
{
    if( c < 0 || c >= gridCols ) return;// off grid
    if( r0 > rf ){ int k = r0; r0 = rf; rf = k; }// swap
    for( int r = r0; r <= rf; ++r )
    {
        if( r < 0 ) continue;
        else if( r >= gridRows ) break;
        pLt0[ r*gridCols + c ] = LtClr;
    }
}

vec2f LineShape::get_dIter( vec2f line )const// helper for draw3()
{
    bool steep = ( line.y*line.y > line.x*line.x );
    // correct if line.x > 0 && line.y > 0
    vec2f dIter = steep ? vec2f(line.x/line.y, 1.0f) : vec2f(1.0f, line.y/line.x);
    if( steep && line.y < 0.0f )
    {
        float ux = -line.x/line.y;
        dIter = vec2f( ux, -1.0f );
    }

    if( !steep && line.x < 0.0f )
    {
        float uy = -line.y/line.x;
        dIter = vec2f( -1.0f, uy );
    }

    return dIter;
}

void LineShape::draw3()const// anti alias
{
    // fix ends at integer position
    int x0 = floor(pos.x), y0 = floor(pos.y);
    int xf = floor(pos2.x), yf = floor(pos2.y);
    // handle special cases: horizontal, vertical and +- 45 degrees
    if( y0 == yf ){ drawHorizontal( y0, x0, xf ); return; }
    if( x0 == xf ){ drawVertical( x0, y0, yf ); return; }

    // will write to end Lights last
    vec2f P0(x0,y0), Pf(xf,yf);// ends for here
    vec2f line = Pf - P0;
    bool steep = ( line.y*line.y > line.x*line.x );
    // step +-1.0f in x or y direction each iteration below
    // todo: generalize to other quadrants
    // correct if line.x > 0 && line.y > 0
 //   vec2f dIter = steep ? vec2f(line.x/line.y, 1.0f) : vec2f(1.0f, line.y/line.x);
    vec2f dIter = get_dIter( line );
    // line position iterator
 //   vec2f iter = P0 + dIter;// 1 step from start
  //  Pf -= dIter;// short 1 Light from end
 //   while( ( Pf - iter ).dot( line ) >= 0.0f )// until iter passes Pf
    for( vec2f iter = P0 + dIter; ( Pf - iter ).dot( line ) >= 0.0f;  iter += dIter )// until iter passes Pf
    {
        int r = floor(iter.y), c = floor(iter.x);
        // bound check
        if( r < 0 )
        { if( line.y < 0.0f ) break; else continue; }// break if going up
        else if( r >= gridRows )
        { if( line.y > 0.0f ) break; else continue; }// break if going down
        if( c < 0 )
        { if( line.x < 0.0f ) break; else continue; }
        else if( c >= gridCols )
        { if( line.x > 0.0f ) break; else continue; }

        if( steep )// write to left and right of the line
        {
            // weights for Light colors
            float fx = iter.x - c, rfx = 1.0f - fx;// fraction and remaining fraction
            int n = r*gridCols + c;
            pLt0[n] = Light( rfx*LtClr.r + fx*pLt0[n].r, rfx*LtClr.g + fx*pLt0[n].g, rfx*LtClr.b + fx*pLt0[n].b );
            // 2nd to right since floor went to left            
            if( ++c < gridCols )
            {   // apply other fraction
                ++n;
                pLt0[n] = Light( fx*LtClr.r + rfx*pLt0[n].r, fx*LtClr.g + rfx*pLt0[n].g, fx*LtClr.b + rfx*pLt0[n].b );
            }
        }
        else// write above and below
        {
            float fy = iter.y - r, rfy = 1.0f - fy;// fraction and remaining fraction
            int n = r*gridCols + c;
            pLt0[n] = Light( rfy*LtClr.r + fy*pLt0[n].r, rfy*LtClr.g + fy*pLt0[n].g, rfy*LtClr.b + fy*pLt0[n].b );
            // 2nd below since floor went up            
            if( ++r < gridRows )
            {   // apply other fraction
                n += gridCols;
                pLt0[n] = Light( fy*LtClr.r + rfy*pLt0[n].r, fy*LtClr.g + rfy*pLt0[n].g, fy*LtClr.b + rfy*pLt0[n].b );
            }
        }

     //   iter += dIter;
    }// end for //while

    // will write to end Lights last
    if( x0 >= 0 && x0 < gridCols && y0 >= 0 && y0 < gridRows )
        pLt0[ y0*gridCols + x0 ] = LtClr;

    if( xf >= 0 && xf < gridCols && yf >= 0 && yf < gridRows )
        pLt0[ yf*gridCols + xf ] = LtClr;

    // done!
}