#ifndef INTEGERLINE_H
#define INTEGERLINE_H

#include "MenuLine.h"

class IntegerLine : public MenuLine
{
    public:
    // for type 2 events
    int inBoost = 8;// += inBoost when actButtPressed
    int* piVal = nullptr;
    int minVal = 0.0f;
    int maxVal = 1.0f;
    int iVal = 0;// initial value
    int dblVal = 0;// to halve the delta
   

    // each derived defines a setup procedure
    virtual String draw()const
    {
        String retVal = label;
        if( piVal ) retVal += *piVal;
        else retVal += "NULL";

        if( pDoAct ) retVal += *pDoAct ? " ON" : " OFF";       
        return retVal;
    }

    virtual bool update()
    {
        if( !piVal ) return false;// NEW

        bool retVal = MenuLine::update();

        if( pRotEncDelta && *pRotEncDelta != 0 )
        {
            int incAmount = pActButt->isClosed ? inBoost : 1;
            if( *pRotEncDelta > 0 ) dblVal += incAmount;
            else dblVal -= incAmount;

            if( dblVal < 2*minVal ) dblVal = 2*minVal;
            else if( dblVal > 2*maxVal ) dblVal = 2*maxVal;

            *piVal = dblVal/2;
            retVal = true;
        }
        // handle other event types
        else if( pRotEncButt->pollEvent() == 1 )
        {
            *piVal = iVal;
            retVal = true;
        }

        return retVal; 
    }

    void setupInt( int& Value, int MinVal, int MaxVal )
    {
        piVal = &Value;
        iVal = Value;
        minVal = MinVal;
        maxVal = MaxVal;
        dblVal = 2*Value;
    }

    IntegerLine(){}
    virtual ~IntegerLine(){}
};

#endif // INTEGERLINE_H
