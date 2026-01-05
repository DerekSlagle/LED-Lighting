#ifndef MENUPAGE_H
#define MENUPAGE_H

#include "ArduinoLevel.h"
#include "Arduino.h"
#include "FloatLine.h"
#include "IntegerLine.h"
#include "../FileParser.h"

class MenuPage
{
    public:    
 //   int myPageNum = 0;
    String title;
    MenuLine* pLine = nullptr;
    MenuLine* pCurrLine = nullptr;// until there are lines
    // notify need to update display. false assigned in level after draw
    bool* pDoUpdateDisplay = nullptr;
    bool ownsLines = false;
    // Quit/Home as lastLine
    MenuLine lastLine;

    // button to act on menu selection
    int actButtID = 1;// keep track of state so response
    bool actButtPressed = false;//  to other events (eg type=2 in derived)
    int menuButtID = 2;

    // recursive: deletes from tail to head (tail recursion)
    // DOES NOT delete the head node passed
    void destroyLines( MenuLine* lineIter )
    {
        if( !ownsLines ) return;
        // iterate to tail
        if( lineIter->pNextLine ) destroyLines( lineIter->pNextLine );// next node
        else return;// from tail
        // back from recursive call        
        delete lineIter->pNextLine;// delete on the way out
        lineIter->pNextLine = nullptr;// lineIter = new tail node
        return;
    }

    // Empty. No lines yet
    void setup( const char* Title, bool& doUpdateDisplay )
    {
        title = Title;
        pDoUpdateDisplay = &doUpdateDisplay;          
        if( pLine && ownsLines )// release
        {
            destroyLines( pLine );
            delete pLine;// the head node
            pLine = nullptr;
            ownsLines = false;
         //   numOptions = 1;// Quit
        }

        pLine = nullptr;
        lastLine.setupBase( "Quit" );
        pCurrLine = &lastLine;// until lines are added

        return;
    }

    // List of Lines is provided. Not owner
    void setup( const char* Title, bool& doUpdateDisplay, MenuLine& headLine )
    {
        // to recycle this instance
        if( pLine && ownsLines )// release
        {
            destroyLines( pLine );
            delete pLine;// the head node
            pLine = nullptr;
            ownsLines = false;
         //   numOptions = 1;// Quit
        }

        pLine = &headLine;
        pCurrLine = pLine;// 1st line
        ownsLines = false;
        title = Title;
        pDoUpdateDisplay = &doUpdateDisplay;
        lastLine.setupBase( "Quit" );// default

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
         //    numOptions = 2;// head + Quit
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

    void addBaseLine( const char* Label, bool* pAct = nullptr )// type MenuLine = base type
    {
        ownsLines = true;
        MenuLine* pML = new MenuLine;
        pML->setupBase( Label );
        pML->pDoAct = pAct;
        pML->pNextLine = nullptr;// cannot assign. Not allocated yet.
        addLine( *pML );        
    }

    void addIntegerLine( const char* Label, int& IntVal, int MinVal, int MaxVal, bool* pAct = nullptr )// type MenuLine = base type
    {
        ownsLines = true;
        IntegerLine* pIL = new IntegerLine;
        pIL->setupBase( Label );
        pIL->setupInt( IntVal, MinVal, MaxVal );
        pIL->pDoAct = pAct;
        pIL->pNextLine = nullptr;// just right for new tail node
        addLine( *pIL );        
    }

    void addFloatLine( const char* Label, float& FloatVal, float MinVal, float MaxVal, float InScale, bool* pAct = nullptr )// type MenuLine = base type
    {
        ownsLines = true;
        FloatLine* pFL = new FloatLine;
        pFL->setupBase( Label );
        pFL->setupFloat( FloatVal, MinVal, MaxVal );
        pFL->inScale = InScale;
        pFL->pDoAct = pAct;
        pFL->pNextLine = nullptr;// just right for new tail node
        addLine( *pFL );   
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

     //   retVal += ( pCurrLine == nullptr ) ? "\n* " : "\n  ";// OLD
        retVal += ( pCurrLine == &lastLine ) ? "\n* " : "\n  ";// NEW
        retVal += lastLine.draw();
     //   retVal += ( myPageNum == 0 ) ? "QUIT to menu" : "Home page";
        
        return retVal;
    }

    bool handleEvent( ArduinoEvent AE )
    {
        // handle Quit up front
     //   if( AE.type == -1 && (AE.ID == actButtID) && ( pCurrLine == &lastLine ) )
     //       return false;// Quit

        // no lines? Just a Quit option is ok
        if( !pLine ) return true;

        // NEW: handle menu scroll
        if( AE.ID == menuButtID )
        {
            if( AE.type == 1 )
            {
                if( pCurrLine->pNextLine ) pCurrLine = pCurrLine->pNextLine;// to next line in list
                else// next == nullptr
                {
                    if( pCurrLine == &lastLine ) pCurrLine = pLine;// to 1st line in list
                    else pCurrLine = &lastLine;// to lastLine
                }

            //    if( pCurrLine ) pCurrLine = pCurrLine->pNextLine;// to next line or Home/Quit (nullptr)
            //    else pCurrLine = pLine;// to 1st line from Home/Quit

                if( pDoUpdateDisplay ) *pDoUpdateDisplay = true;
            }
            else if( AE.type == -1 )
            {
                // nothing on release
                return true;
            }
        }

        // handle rotEncButt release?

        // handle actButtPressed
        if( AE.ID == actButtID )
        {
            if( AE.type == 1 ) actButtPressed = true;
            else if( AE.type == -1 ) actButtPressed = false;
            // pass on for now. each line has an actButtPressed member
        }

        if( pCurrLine )
        {
     //       if( pLine->handleEvent( AE ) && pDoUpdateDisplay )// OLD
            if( pCurrLine->handleEvent( AE ) && pDoUpdateDisplay )// NEW
                *pDoUpdateDisplay = true;
        }

        // handle Quit last so lastLine can write true to its bool (change page)
        if( AE.type == -1 && (AE.ID == actButtID) && ( pCurrLine == &lastLine ) )
            return false;// Quit

        return true;
    }

    MenuPage(){}
    ~MenuPage()
    { 
        if( ownsLines && pLine )
        {
            destroyLines( pLine );
            delete pLine;// the head node
            pLine = nullptr;
        }
    }
};

#endif // MENUPAGE_H
