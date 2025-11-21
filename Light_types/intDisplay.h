#ifndef INTDISPLAY_H
#define INTDISPLAY_H

#include "DataPlayer.h"

class intDisplay
{
    public:
    DataPlayer DP;
    uint8_t digitData[19];// color index storage
    Light color[2];// back and foreground
    int value = 0;// no setter needed. Call update() to draw
    int gridRows = 1, gridCols = 1, row0 = 0, col0 = 0;
    void init( Light& rLt0, int GridRows, int GridCols, int Row0, int Col0, Light backLt, Light foreLt )
    {
        gridRows = GridRows;
        gridCols = GridCols;
        row0 = Row0;
        col0 = Col0;
        uint8_t* X = digitData;
        X[0] = 111; X[1] = 251; X[2] = 73; X[3] = 250; X[4] = 249;
        X[5] = 252; X[6] = 60; X[7] = 223; X[8] = 158; X[9] = 124;
        X[10] = 206; X[11] = 39; X[12] = 239; X[13] = 79; X[14] = 146;
        X[15] = 239; X[16] = 251; X[17] = 247; X[18] = 36;
        color[0] = backLt;
        color[1] = foreLt;

        DP.init( rLt0, gridRows, gridCols, digitData[0], 19, 2 );
        DP.pLight = color;
        DP.isPlaying = false;// do not animate. just draw
        value = 0;
        DP.stepIter = 0;// display 0
        DP.setTargetRect( 5, 3, row0, col0 );// 1-s digit
    }

    intDisplay(){}
    intDisplay( Light& rLt0, int GridRows, int GridCols, int Row0, int Col0, Light backLt, Light foreLt )
    { init( rLt0, GridRows, GridCols, Row0, Col0, backLt, foreLt ); }

    void drawMinus()
    {
        // not ready
    }

    void update()
    {
        int numDigits = 1;
        int temp = value;
        if( temp < 0 ) temp *= -1;// positive
        int digitCol = col0;
        while( temp > 9 )
        {
            ++numDigits;
            temp /= 10;
            digitCol += 3;
        }

        temp = value;// again to devour value
        if( temp < 0 )
        {
            temp *= -1;// positive
            digitCol += 3;// room for - sigh
            drawMinus();
        }

        // draw 1-s digit
        DP.stepIter = temp%10;
        DP.col0 = digitCol;
        DP.update();
        temp /= 10;
        DP.col0 -= 3;

        while( temp > 0 )
        {
            DP.stepIter = temp%10;
            DP.update();
            temp /= 10;
            DP.col0 -= 3;
        }
    }
};

#endif // INTDISPLAY_H