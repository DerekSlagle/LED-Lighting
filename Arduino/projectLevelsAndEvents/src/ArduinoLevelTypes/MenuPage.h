#ifndef MENUPAGE_H
#define MENUPAGE_H

#include "ArduinoLevel.h"
#include "Arduino.h"
#include "MenuLine.h"

class MenuPage
{
    public:
    // for a display. Call instead of using bool to signal need to update.
    // function to be defined globally eg. in main.cpp
 //   static void ( *UpdateDisplay )(void);// method 1
    static void ( *UpdateDisplay )( const char* );// method 1

 //   int myPageNum = 0;
    String title;
    MenuLine* pLine = nullptr;
    MenuLine* pCurrLine = nullptr;// until there are lines
    // notify need to update display. false assigned in level after draw
    bool* pDoUpdateDisplay = nullptr;// method 2
  
    // a 2nd list for timed updates or other
    // no call to handleEvent(). Use is in draw() only
    MenuLine* pShowLine = nullptr;

    // button to act on menu selection
    int actButtID = 1;// keep track of state so response
    bool actButtPressed = false;//  to other events (eg type=2 in derived)
    int menuButtID = 2;   

    // List of Lines is provided. Not owner
    void setup( const char* Title, bool& doUpdateDisplay, MenuLine& headLine )
    {
        pLine = &headLine;
        pCurrLine = pLine;// 1st line
     //   ownsLines = false;
        title = Title;
        pDoUpdateDisplay = &doUpdateDisplay;
        // assign all of the pLine pActButtPressed
        MenuLine* iter = pLine;
        while( iter )
        {
            iter->pActButtPressed = &actButtPressed;
            iter = iter->pNextLine;
        }

        return;
    }

    void setup( const char* Title, MenuLine& headLine )
    {
        pLine = &headLine;
        pCurrLine = pLine;// 1st line
     //   ownsLines = false;
        title = Title;
        pDoUpdateDisplay = nullptr;// the static function pointer UpdateDisplay to be used
        // assign all of the pLine pActButtPressed
        MenuLine* iter = pLine;
        while( iter )
        {
            iter->pActButtPressed = &actButtPressed;
            iter = iter->pNextLine;
        }

        return;
    }
  

    String draw()const
    {
        String retVal = title;

        const MenuLine* LineIter = pLine;// read only. may re bind
        while( LineIter )
        {
            retVal += ( pCurrLine == LineIter ) ? "\n* " : "\n  ";// NEW
            retVal += LineIter->draw();
            LineIter = LineIter->pNextLine;
        }

        LineIter = pShowLine;// read only
     //   if( pShowLine ) retVal += '\n';// one only
        while( LineIter )
        {
            // setup lines with new line as needed
            retVal += LineIter->draw();
            LineIter = LineIter->pNextLine;
        }        
        
        return retVal;
    }

    void Display()const
    {
        if( MenuPage::UpdateDisplay ) 
            MenuPage::UpdateDisplay( draw().c_str() );
    }

    bool handleEvent( ArduinoEvent AE )
    {
        // no lines? Just a Quit option is ok
        if( !pLine ) return false;// non viable page

        bool retVal = true;// unless Quit?

        // NEW: handle menu scroll
        if( AE.ID == menuButtID )
        {
            if( AE.type == 1 )
            {
                if( pCurrLine->pNextLine ) pCurrLine = pCurrLine->pNextLine;// to next line in list
                else pCurrLine = pLine;// to 1st line in list

                if( pDoUpdateDisplay ) *pDoUpdateDisplay = true;
                else if( MenuPage::UpdateDisplay ) MenuPage::UpdateDisplay( draw().c_str() );
            }
            else if( AE.type == -1 )
            {
                // nothing on release
              //  return true;
            }
        }

        // handle rotEncButt release?

        // handle actButtPressed
        if( AE.ID == actButtID )
        {
            if( AE.type == 1 ) actButtPressed = true;
            else if( AE.type == -1 ) actButtPressed = false;
            // pass on for now. each line has a pDoAct to write to
        }

        if( pCurrLine )
        {
        //    if( pCurrLine->handleEvent( AE ) && pDoUpdateDisplay )// old
            if( pCurrLine->handleEvent( AE ) )// NEW
            {
                if( pDoUpdateDisplay ) *pDoUpdateDisplay = true;
                else if( MenuPage::UpdateDisplay ) MenuPage::UpdateDisplay( draw().c_str() );
            }
        }

        // handle Quit last so lastLine can write true to its bool (change page)
        if( AE.type == -1 && (AE.ID == actButtID) && ( pCurrLine->pNextLine == nullptr ) )
            retVal = false;// Quit

        return retVal;
    }

    MenuPage(){}
    ~MenuPage(){}
};

#endif // MENUPAGE_H
