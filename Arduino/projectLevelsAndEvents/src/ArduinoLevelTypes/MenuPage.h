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
    int myPageNum = 0;
    String title;
    int numOptions = 0;
    MenuLine* pLine = nullptr;
    int menuIter = 0;

    // button to act on menu selection
    int actButtID = 1;// keep track of state so response
    bool actButtPressed = false;//  to other events (eg type=2 in derived)
    int menuButtID = 2;

    // recursive: deletes from tail to head (tail recursion)
    // DOES NOT delete the head node passed
    void destroyLines( MenuLine* lineIter )
    {
        // iterate to tail
        if( lineIter->pNextLine ) destroyLines( lineIter->pNextLine );// next node
        else return;
        // back from recursive call
        // delete on the way out
        delete lineIter->pNextLine;
        lineIter->pNextLine = nullptr;// safe off
        return;
    }

    void setup( const char* Title, int NumLines )
    {
        title = Title;
        numOptions = NumLines + 1;
        if( pLine )// release
        {
            destroyLines( pLine );
            delete pLine;// the head node
            pLine = nullptr;
        }

        pLine = nullptr;// extra sure

        return;
    }

    void addLine( MenuLine& ML )
    {
        // assign as tail node
        if( !pLine )// is also head node
        {
             ML.pNextLine = nullptr;// assured
             pLine = &ML;
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
                break;                    
            }

            LineIter = LineIter->pNextLine;
        }
    }

    void addBaseLine( const char* Label, bool* pAct = nullptr )// type MenuLine = base type
    {
        MenuLine* pML = new MenuLine;
        pML->setupBase( menuIter, Label );
        pML->pDoAct = pAct;
        pML->pNextLine = nullptr;// cannot assign. Not allocated yet.
        addLine( *pML );        
    }

    void addIntegerLine( const char* Label, int& IntVal, int MinVal, int MaxVal, bool* pAct = nullptr )// type MenuLine = base type
    {
        IntegerLine* pIL = new IntegerLine;
        pIL->setupBase( menuIter, Label );
        pIL->setupInt( IntVal, MinVal, MaxVal );
        pIL->pDoAct = pAct;
        pIL->pNextLine = nullptr;// just right for new tail node
        addLine( *pIL );        
    }

    void addFloatLine( const char* Label, float& FloatVal, float MinVal, float MaxVal, float InScale, bool* pAct = nullptr )// type MenuLine = base type
    {
        FloatLine* pFL = new FloatLine;
        pFL->setupBase( menuIter, Label );
        pFL->setupFloat( FloatVal, MinVal, MaxVal );
        pFL->inScale = InScale;
        pFL->pDoAct = pAct;
        pFL->pNextLine = nullptr;// just right for new tail node
        addLine( *pFL );   
    }

    String draw()const
    {
        String retVal = title;
        if( pLine ) retVal += pLine->draw();

        retVal += ( menuIter == numOptions - 1 ) ? "\n *" : "\n  ";
        retVal += ( myPageNum == 0 ) ? "QUIT to menu" : "Home page";
        
        return retVal;
    }

    bool handleEvent( ArduinoEvent AE )
    {
        // handle Quit up front
        if( AE.type == -1 && (AE.ID == actButtID) && (myPageNum == 0) &&  (menuIter == numOptions - 1) )
            return false;// Quit

        // handle menu scroll
        if( AE.ID == menuButtID )
        {
            if( AE.type == 1 )
            menuIter = ( 1 + menuIter )%numOptions;
            // nothing on release
            return true;
        }

        // handle rotEncButt release?

        // handle actButtPressed
        if( AE.ID == actButtID )
        {
            if( AE.type == 1 ) actButtPressed = true;
            else if( AE.type == -1 ) actButtPressed = false;
            // pass on for now. each line has an actButtPressed member
        }

        if( pLine ) return pLine->handleEvent( AE );
        return true;
    }

    MenuPage(){}
    ~MenuPage()
    { 
        destroyLines( pLine );
        delete pLine;// the head node
        pLine = nullptr;
    }
};

#endif // MENUPAGE_H
