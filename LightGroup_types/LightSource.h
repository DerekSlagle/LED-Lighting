#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

#include<iostream>// for troubleshooting cout only

#include"LightGrid.h"
#include"../utility/vec2f.h"

// abstract base for LightSource types
class LightSource
{
    public:
    LightGrid* pTgt = nullptr;
    int row0 = 0, col0 = 0;// origin on target
//    int rows = 1;// rows in draw area
//    int cols = 1;// area drawn to
    // user may assign. No setter needed
    int rShift = 0, cShift = 0;// positive = right and down used in drawShifted()
    bool wrapRow = false, wrapCol = false;// wrap on rows x cols sub region. NOT implemented yet
    bool flipX = false, flipY = false;
    // draw an outline
    int outLineThickness = 0;
    Light outlineLight;
    void drawOutline()const;// called by draw()

    void Clear( Light clearLt )const;// it can draw because there is a source. virtual in LightSourcePrinter
    void draw()const;
    // pure virtual
    virtual Light getLt( int r, int c )const = 0;// source will vary in derived
 //   void draw( const LightGrid& LG )const;// bound check here only. NOT everywhere

    void setTarget( LightGrid& tgtGrid )
    { pTgt = &tgtGrid; }

    void setPosition( int Row0, int Col0 )
    {
        row0 = Row0;
        col0 = Col0;
    }
    // other members set in derived classes

    LightSource(){}
    virtual ~LightSource(){}

    // partially in = both false below
    bool isInGrid()const;// true if fully inside of grid
    bool isAllOut()const;// true if fully outside of grid

    int getCols()const { return cols; }
    int getRows()const { return rows; }

    // for animated motion
    vec2f vel;
    bool isMoving = false;
    void updatePosition( float dt );
    void setPosition( float x, float y );
    void setPosition( vec2f Pos ){ setPosition( Pos.x, Pos.y ); }
    vec2f getPosition()const{ return pos; }

    protected:
    // for animated motion
    vec2f pos;
    vec2f ctrOfst;// from col0, row0
    // set by derived
    int rows = 1;// rows in draw area
    int cols = 1;// area drawn to
    void drawFlipped()const;// called by draw()
    void drawShifted()const;// called by draw()

    private:
};

#endif // LIGHTSOURCE_H
