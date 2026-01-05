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
    virtual String draw()const
    {
        String retVal = label;
        if( piVal ) retVal += *piVal;
        else retVal += "NULL";

        if( pDoAct ) retVal += *pDoAct ? " ON" : " OFF";       
        return retVal;
    }

    virtual bool handleEvent( ArduinoEvent AE )
    {
        if( !piVal ) return false;// NEW        
        if( handleActButtEvent( AE ) ) return true;// it was an actButt event

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

        return false; 
    }

    void setupInt( int& Value, int MinVal, int MaxVal )
    {
        piVal = &Value;
        iVal = Value;
        minVal = MinVal;
        maxVal = MaxVal;
    }

    IntegerLine(){}
    virtual ~IntegerLine(){}
};

#endif // INTEGERLINE_H
