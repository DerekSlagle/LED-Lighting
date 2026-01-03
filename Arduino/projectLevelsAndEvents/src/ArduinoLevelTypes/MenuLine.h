#ifndef MENULINE_H
#define MENULINE_H

// base class for a line on a menu
// The display may change when an event occurs
// eg. The line displays an integer value (derived type)
// which is changed by a rotary encoder (type=2 event)
// or reset by a button press (type=1 event)

#include "ArduinoLevel.h"
#include "Arduino.h"

// base class for levels targeting Arduino
class MenuLine
{
    public:
    int* pMenuIter = nullptr;
  //  int myIterVal = 0;// which line in menu
    String label;
    MenuLine* pNextLine = nullptr;

    // button to act on menu selection
    int actButtID = 1;// keep track of state so response
    bool actButtPressed = false;//  to other events (eg type=2 in derived)
    bool* pDoAct = nullptr;// so user can follow through with code execution if any

    //void setupBase( int& MenuIter, int myIterValue, const char* Label )
    void setupBase( int& MenuIter, const char* Label )
    {
        pMenuIter = &MenuIter;
     //   myIterVal = myIterValue;
        label = Label;
    }

    // each derived defines a setup procedure
    virtual String draw( int LineNum = 0 )const
    {
        String retVal;
        if( !( pMenuIter ) ) return retVal;// empty        
        retVal = ( *pMenuIter == LineNum ) ? "\n* " : "\n  ";
        retVal += label;
        if( pDoAct ) retVal += *pDoAct ? " ON" : " OFF";

        // append remaining lines
        if( pNextLine ) retVal += pNextLine->draw( LineNum + 1 );
        return retVal;
    }

    bool handleActButtEvent( ArduinoEvent AE, int LineNum = 0 )
    {
        if( !pMenuIter ) return false;

        if( *pMenuIter == LineNum )
        {            
            if( AE.ID == actButtID )
            {
                if( AE.type == 1 )// press
                {
                    actButtPressed = true;
                    if( pDoAct ) *pDoAct = true;
                }
                else if( AE.type == -1 )// release
                {
                    actButtPressed = false;
                    if( pDoAct ) *pDoAct = false;
                }
                else return false;

                return true;
            }

            return false;
        }

        return false;
    }

    virtual bool handleEvent( ArduinoEvent AE, int LineNum = 0 )
    {
        if( !pMenuIter ) return false;

        if( handleActButtEvent( AE, LineNum ) )
        {       
            return true;
        }
        else if( pNextLine )
        {
            return pNextLine->handleEvent( AE, LineNum + 1 );
        }

        return true;
    }

    MenuLine(){}
    virtual ~MenuLine(){}
};

#endif // MENULINE_H
