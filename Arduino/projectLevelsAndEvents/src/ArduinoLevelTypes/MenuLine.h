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
    // int* pMenuIter = nullptr;// dep
  //  int myIterVal = 0;// which line in menu
    String label;
    MenuLine* pNextLine = nullptr;

    // button to act on menu selection
    int actButtID = 1;// keep track of state so response
    bool actButtPressed = false;//  to other events (eg type=2 in derived)
    bool* pDoAct = nullptr;// so user can follow through with code execution if any

    //void setupBase( int& MenuIter, int myIterValue, const char* Label )
 //   void setupBase( int& MenuIter, const char* Label )// OLD
    void setupBase( const char* Label, bool* p_DoAct = nullptr )// NEW
    {
     //   pMenuIter = &MenuIter;
     //   myIterVal = myIterValue;
        label = Label;
        pDoAct = p_DoAct;
    }

    // each derived defines a setup procedure
 //   virtual String draw( int LineNum = 0 )const// OLD
    virtual String draw()const// NEW
    {
        String retVal = label;
        if( pDoAct ) retVal += *pDoAct ? " ON" : " OFF";

        return retVal;
    }

   // bool handleActButtEvent( ArduinoEvent AE, int LineNum = 0 )// OLD
    bool handleActButtEvent( ArduinoEvent AE )//NEW
    {
        if( AE.ID == actButtID )
        {
            if( AE.type == 1 )// press
            {
                actButtPressed = true;
                if( pDoAct ) *pDoAct = true;
                return true;
            }

            if( AE.type == -1 )// release
            {
                actButtPressed = false;
                if( pDoAct ) *pDoAct = false;
                return true;
            }            
        }

        return false;
    }

 //   virtual bool handleEvent( ArduinoEvent AE, int LineNum = 0 )// OLD
    virtual bool handleEvent( ArduinoEvent AE )// NEW
    {
        return handleActButtEvent( AE );// NEW This is only event handled by the base line type 
    }

    MenuLine(){}
    virtual ~MenuLine(){}
};

#endif // MENULINE_H
