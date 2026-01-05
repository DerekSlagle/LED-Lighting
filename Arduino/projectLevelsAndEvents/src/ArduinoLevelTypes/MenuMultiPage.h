#ifndef MENUMULTIPAGE_H
#define MENUMULTIPAGE_H

#include "ArduinoLevel.h"
#include "Arduino.h"
#include "MenuPage.h"

class MenuMultiPage
{
    public:
    int actButtID = 1;

    int numPages = 0;
    MenuPage* pPage = nullptr;// fully setup array to be supplied by user
    // each line for going to a page writes to that bool here. Poll for this after an actButt event
    // also supplied by user. Line to bool assignments are made in advance
    bool * pGotoPage = nullptr;// a line writes to the bool corresponding to the new page
    int currPage = 0;

    void setup( MenuPage* p_Page,  bool * p_GotoPage, int NumPages )
    {
        pPage = p_Page;
        pGotoPage = p_GotoPage;
        numPages = NumPages;
        currPage = 0;
        // start state for bools = true. Will watch for false assign (on actButt release)
        for( int n = 0; n < numPages; ++n )
            pGotoPage[n] = true;
    }

    String draw()const
    {
        return pPage[ currPage ].draw();
    }

    bool handleEvent( ArduinoEvent AE )
    {
        if( !pPage ) return false;

        // handle Quit up front
      //  if( AE.type == -1 && ( AE.ID == actButtID )
     //    && ( currPage == 0 ) &&( !pPage[0].pCurrLine->pNextLine ) )// last line has no next
     //       return false;// Quit

        // to the current page
        pPage[ currPage ].handleEvent( AE );// return value does not matter

        // goto new page? On actButt released
        if( AE.type == -1 && AE.ID == actButtID )
        {
            // was any bool assigned = false?
            for( int n = 0; n < numPages; ++n )
            {
                if( !pGotoPage[n] )// goto this page
                {
                    pGotoPage[n] = true;
                    // cleanup of currPage?
                    currPage = n;
                    // assign currLine
                    if( pPage[n].pLine ) pPage[n].pCurrLine = pPage[n].pLine;// if > 0 lines
                    else pPage[n].pCurrLine = &( pPage[n].lastLine );// if no lines. Home only
                    // update display
                    if( pPage[n].pDoUpdateDisplay ) *pPage[n].pDoUpdateDisplay = true;
                    break;
                }
            }

            // check for quit after currPage change
            if( ( currPage == 0 ) && ( pPage[0].pCurrLine == &pPage[0].lastLine ) )// last line is separate from list
                return false;// Quit
        }

        return true;// unless Quitting level
    }

    MenuMultiPage(){}
    ~MenuMultiPage()
    { 
    }
};

#endif // MENUMULTIPAGE_H
