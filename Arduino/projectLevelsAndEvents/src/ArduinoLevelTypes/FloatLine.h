#ifndef FLOATLINE_H
#define FLOATLINE_H

// base class for a line on a menu
// The display may change when an event occurs
// eg. The line displays an integer value (derived type)
// which is changed by a rotary encoder (type=2 event)
// or reset by a button press (type=1 event)

#include "MenuLine.h"

// base class for levels targeting Arduino
class FloatLine : public MenuLine
{
    public:    
    // for type 2 events
    int rotEncID = 1;
    int rotEncButtID = 3;// for value reset
    float inScale = 1.0f;
    float inBoost = 4.0f;// *= inScale when actButtPressed
    float* pfVal = nullptr;
    float minVal = 0.0f;
    float maxVal = 1.0f;
    float iVal = 0;// initial value

    virtual String draw( int LineNum = 0 )const// a toString()
    {
        String retVal;
        if( !( pMenuIter && pfVal ) ) return retVal;// empty
        retVal = ( *pMenuIter == LineNum ) ? "\n* " : "\n  ";
        retVal += label;
        retVal += *pfVal;
        // append remaining lines
        if( pNextLine ) retVal += pNextLine->draw( LineNum + 1 );

        return retVal;
    }

    virtual bool handleEvent( ArduinoEvent AE, int LineNum = 0 )
    {
        if( !( pMenuIter && pfVal ) ) return false;

        if( *pMenuIter == LineNum )
        {
            if( handleActButtEvent( AE, LineNum ) ) return true;// it was an actButt event

            if( AE.type == 2 && AE.ID == rotEncID )
            {
                float inFactor = actButtPressed ? inBoost*inScale : inScale;
                *pfVal += inFactor*AE.value;
                if( *pfVal < minVal ) *pfVal = minVal;
                else if( *pfVal > maxVal ) *pfVal = maxVal;
                return true;
            }
            else if( AE.type == 1 && AE.ID == rotEncButtID )
            {
                *pfVal = iVal;
                return true;
            }
        }
        else if( pNextLine )
        {
            return pNextLine->handleEvent( AE, LineNum + 1 );
        }

        return true;
    }

    

    void setupFloat( float& Value, float MinVal, float MaxVal )
    {
        pfVal = &Value;
        iVal = *pfVal;
        minVal = MinVal;
        maxVal = MaxVal;
    }

    // assign rotEncID and inScale directly

    FloatLine(){}
    virtual ~FloatLine(){}
};

#endif // FLOATLINE_H
