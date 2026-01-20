#ifndef MENUPAGE_H
#define MENUPAGE_H

#include "Arduino.h"
#include "MenuLine.h"
//#include "FloatLine.h"
//#include "IntegerLine.h"

class MenuPage
{
    public:    
 //   int myPageNum = 0;
    String title;
    MenuLine* pLine = nullptr;
    MenuLine* pCurrLine = nullptr;// until there are lines
    // notify need to update display. false assigned in level after draw
    void updateDisplay()const
    {
        if( !MenuLine::pDisplay ) return;// crash avoidance
        String str = title;
        MenuLine* iter = pLine;
        while( iter )
        {
            str += ( iter == pCurrLine ) ? "\n* " : "\n  ";
            str += iter->draw();
            iter = iter->pNextLine;
        }

        // write
        MenuLine::pDisplay->clear();
        MenuLine::pDisplay->printAt( 0, 0, str.c_str(), 1 );
        MenuLine::pDisplay->show();
    }

    // Empty. No lines yet
    void setup( const char* Title )
    {
        title = Title;  
        
        pLine = nullptr;
        pCurrLine = nullptr;// until lines are added
        return;
    }

    // List of Lines is provided. Not owner
    void setup( const char* Title, MenuLine& headLine )
    {       

        pLine = &headLine;
        pCurrLine = pLine;// 1st line
        title = Title;
        updateDisplay();      

        return;
    }

    void addLine( MenuLine& ML )
    {
        // assign as tail node
        if( !pLine )// is also head node
        {
             ML.pNextLine = nullptr;// assured
             pLine = &ML;
             pCurrLine = pLine;
             return;// done
        }
        // iterate to tail node      
        MenuLine* LineIter = pLine;        
        while( LineIter )
        {
            if( !LineIter->pNextLine )
            {
                ML.pNextLine = nullptr;// assured
                LineIter->pNextLine = &ML;// new tail node
              //  ++numOptions;
                break;                    
            }

            LineIter = LineIter->pNextLine;
        }
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
        
        return retVal;
    }

    bool update()
    {
        if( !pLine ) return false;
        bool retVal = false;

        // NEW: handle menu scroll
        if( MenuLine::pMenuButt->pollEvent() == 1 )
        {
            if( pCurrLine->pNextLine ) pCurrLine = pCurrLine->pNextLine;// to next line in list
            else pCurrLine = pLine;// to 1st line in list
        //    updateDisplay();
            retVal = true;
        }
        else if( MenuLine::pMenuButt->pollEvent() == -1 )
        {
            // nothing on release
        }
        

        // handle rotEncButt release?
        // handle actButtPressed ?

        if( pCurrLine )
        {
            if( pCurrLine->update() )// NEW
             { retVal = true; }
        }

        if( retVal ) updateDisplay();
        return retVal;
    }

    MenuPage(){}
    ~MenuPage() {}
};

#endif // MENUPAGE_H
