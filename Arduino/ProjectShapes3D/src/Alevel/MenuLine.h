#ifndef MENULINE_H
#define MENULINE_H

#include "../SSD_1306Component.h"// use a display

// base class for a line on a menu
// The display may change when an event occurs
// eg. The line displays an integer value (derived type)
// which is changed by a rotary encoder (type=2 event)
// or reset by a button press (type=1 event)

#include "Arduino.h"
#include "../switchSPST.h"
#include "../SSD_1306Component.h"

class MenuLine
{
    public:
    static switchSPST *pActButt, *pMenuButt, *pRotEncButt;
    static int* pRotEncDelta;
    static SSD1306_Display* pDisplay;
    static void setupInOut( SSD1306_Display& r_Oled, switchSPST *p_ActButt,
    switchSPST *p_MenuButt, switchSPST *p_RotEncButt, int& rotEncDelta )
    {
        pDisplay = &r_Oled;// There is no need to ever check this pointer. It is VALID
        pActButt = p_ActButt;
        pMenuButt = p_MenuButt;
        pRotEncButt = p_RotEncButt;
        pRotEncDelta = &rotEncDelta;
    }

    // regular members
    String label;
    // button to act on menu selection
    bool* pDoAct = nullptr;// so user can follow through with code execution if any
    MenuLine* pNextLine = nullptr;// to form linked list

    void setupBase( const char* Label, bool* p_DoAct = nullptr )
    {
        label = Label;
        pDoAct = p_DoAct;
    }

    virtual String draw()const
    {
        String retVal = label;
        if( pDoAct ) retVal += *pDoAct ? " ON" : " OFF";

        return retVal;
    }

    virtual bool update()
    {
        if( pActButt->pollEvent() == 1 )// press
        {
            if( pDoAct ) { *pDoAct = true; return true; }
        }

        else if( pActButt->pollEvent() == -1 )// release
        {
            if( pDoAct ) { *pDoAct = false; return true; }
        }
        return false;
    }

    MenuLine(){}
    virtual ~MenuLine(){}
};

#endif // MENULINE_H
