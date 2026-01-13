#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

//#include "ArduinoLevel.h"
//#include "Arduino.h"
#include "MenuPage.h"

class PageManager
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
        pPage->updateDisplay();
        // start state for bools = true. Will watch for false assign (on actButt release)
        for( int n = 0; n < numPages; ++n )
            pGotoPage[n] = true;
    }

    String draw()const
    {
        return pPage[ currPage ].draw();
    }

    bool update()
    {
        if( !( pPage && pPage->pLine ) ) return false;
       
        // to the current page
        pPage[ currPage ].update();// return value does not matter

        // goto new page? On actButt released
        if( MenuLine::pActButt->pollEvent() == -1 )
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
                    pPage[n].pCurrLine = pPage[n].pLine;// if > 0 lines
                    // update display
                    pPage[n].updateDisplay();
                    break;
                }
            }

            // check for quit after currPage change
            if( ( currPage == 0 ) && ( pPage[0].pCurrLine->pNextLine == nullptr ) )// last line
                return false;// Quit
        }

        return true;// unless Quitting level
    }

    PageManager(){}
    ~PageManager()
    { 
    }
};

#endif // PAGEMANAGER_H
