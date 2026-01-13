#ifndef PERSLINE_H
#define PERSLINE_H

#include "persPt.h"
#include "../Shapes/LineShape.h"

class persLine
{
    public:
    LineShape theLine;// pos and pos2 assigned in update()
    vec3f startPos, endPos;
    vec3f pt[2];
    vec3f dir;// line direction
    bool doDraw = true;
    void draw() const;
    void init( vec3f StartPos, vec3f EndPos, Light color );
    void update( float dt );

    persLine(){}
    persLine( vec3f StartPos, vec3f EndPos, Light color ){ init( StartPos, EndPos, color ); }
};

#endif // PERSLINE_H
