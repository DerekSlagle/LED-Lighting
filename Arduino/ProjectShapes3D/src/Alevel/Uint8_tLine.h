#ifndef UINT8_TLINE_H
#define UINT8_TLINE_H

#include "MenuLine.h"

class Uint8_tLine : public MenuLine
{
    public:
    // for type 2 events
    int inBoost = 8;// += inBoost when actButtPressed
    uint8_t* pVal = nullptr;
    uint8_t minVal = 0.0f;
    uint8_t maxVal = 1.0f;
    uint8_t iVal = 0;// initial value
    int dblVal = 0;// to halve the delta
   

    // each derived defines a setup procedure
    virtual String draw()const
    {
        String retVal = label;
        if( pVal ) retVal += (int)(*pVal);
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

            *pVal = (uint8_t)(dblVal/2);
            retVal = true;
        }
        // handle other event types
        else if( pRotEncButt->pollEvent() == 1 )
        {
            *pVal = iVal;
            retVal = true;
        }

        return retVal; 
    }

    void setupUint8_t( uint8_t& Value, uint8_t MinVal, uint8_t MaxVal )
    {
        pVal = &Value;
        iVal = Value;
        minVal = MinVal;
        maxVal = MaxVal;
    }

    Uint8_tLine(){}
    virtual ~Uint8_tLine(){}
};

#endif // UINT8_TLINE_H
