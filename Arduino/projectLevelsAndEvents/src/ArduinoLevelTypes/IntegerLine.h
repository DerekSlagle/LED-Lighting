#ifndef INTEGERLINE_H
#define INTEGERLINE_H

#include "MenuLine.h"

class IntegerLine : public MenuLine
{
    public:
    // for type 2 events
    int rotEncID = 1;
    int rotEncButtID = 3;// for value reset
  //  float inScale = 1.0f;
    int inBoost = 8;// += inBoost when actButtPressed
    int* piVal = nullptr;
    int minVal = 0.0f;
    int maxVal = 1.0f;
    int iVal = 0;// initial value
   

    // each derived defines a setup procedure
    virtual String draw( int LineNum = 0 )const
    {
        String retVal;
        if( !( pMenuIter && piVal ) ) return retVal;// empty        
        retVal = ( *pMenuIter == LineNum ) ? "\n* " : "\n  ";
        retVal += label;
        retVal += *piVal;// pointer to int
        // append remaining lines
        if( pNextLine ) retVal += pNextLine->draw( LineNum + 1 );

        return retVal;
    }

    virtual bool handleEvent( ArduinoEvent AE, int LineNum = 0 )
    {
        if( !( pMenuIter && piVal ) ) return false;

        if( *pMenuIter == LineNum )
        {
            if( handleActButtEvent( AE, LineNum ) ) return true;// it was an actButt event

            if( AE.type == 2 && AE.ID == rotEncID )
            {
                int incAmount = actButtPressed ? inBoost : 1;
                if( AE.value > 0 ) *piVal += incAmount;
                else *piVal -= incAmount;

                if( *piVal < minVal ) *piVal = minVal;
                else if( *piVal > maxVal ) *piVal = maxVal;
                return true;
            }
            // handle other event types
            else if( AE.type == 1 && AE.ID == rotEncButtID )
            {
                *piVal = iVal;
                return true;
            }
        }
        else if( pNextLine )
        {
            return pNextLine->handleEvent( AE, LineNum + 1 );
        }

        return true;
    }

    void setupInt( int& Value, int MinVal, int MaxVal )
    {
        piVal = &Value;
        iVal = *piVal;
        minVal = MinVal;
        maxVal = MaxVal;
    }

    IntegerLine(){}
    virtual ~IntegerLine(){}
};

#endif // INTEGERLINE_H
