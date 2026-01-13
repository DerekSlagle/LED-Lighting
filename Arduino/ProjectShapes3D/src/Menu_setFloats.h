#ifndef MENU_SETFLOATS_H
#define MENU_SETFLOATS_H

#include "switchSPST.h"
#include "slidePot.h"

// use a potentiometer to adjust numVals float values
// use push button to cycle which value
class Menu_setFloats
{
    public:
    slidePot thePot;// 
    int editMode = 0;// 0 = display values only. 1, 2, 3, ..., numVals = set float value
    // a button to set edit mode
    switchSPST editModeButt;// sensor type open/close state reversed ( normally closed )
    char ButtType = 'O';// assumed to be normally open. Some are normally closed

    String Title;
    String StrOut;// instance for our message
    bool doShow = false;// true if editMode or value is changed
    bool* pDoShow = nullptr;// will write above if assigned

    // for the float values
    unsigned int numVals = 0;// float values to adjust
    // use new to allocate numVals elements to each below
    String* valLabel = nullptr;// "label: "
    float** pVal = nullptr;// a pointer for writing for each
    float* minVal = nullptr;
    float* maxVal = nullptr;
    float* potFactor = nullptr;// pot sensitivity

    void Alloc( unsigned int NumVals, const char* title )
    {
        Title = title;
        if( NumVals < 2 ) return;
        numVals = NumVals;
        // arrays
        valLabel = new String[numVals];
        pVal = new float*[numVals];
        minVal = new float[numVals];
        maxVal = new float[numVals];
        potFactor = new float[numVals];
    }
    
    Menu_setFloats( unsigned int NumVals, const char* title ){ Alloc( NumVals, title ); }
    Menu_setFloats(){}
    ~Menu_setFloats()
    {
        // arrays
        if( valLabel ) delete[] valLabel;
        if( pVal ) delete[] pVal;
        if( minVal ) delete[] minVal;
        if( maxVal ) delete[] maxVal;
        if( potFactor ) delete[] potFactor;        
    }

    Menu_setFloats( const Menu_setFloats& ) = delete;// no copy
    Menu_setFloats& operator = ( const Menu_setFloats& ) = delete;// no assign

    // for each value
    void initVal( unsigned int N, const char* ValLabel, float& Value, float MinVal, float MaxVal, float PotFactor )
    {
        valLabel[N] = ValLabel;// insert "\n* " or "\n  " before
        pVal[N] = &Value;
        minVal[N] = MinVal;
        maxVal[N] = MaxVal;
        potFactor[N] =  PotFactor;
    }

    void makeMsg()// form *pStrOut
    {
     //   if( !pStrOut ) return;// bad pointer

            // 1st line
            StrOut = Title;
            if( editMode == 1 ) StrOut += "\n* " + valLabel[0] + ": ";
            else StrOut += "\n  " + valLabel[0] + ": ";
            StrOut += *pVal[0];

            // editMode == 2, 3, ...
            for( unsigned int n = 1; n < numVals; ++n )
            {
                if( editMode == n + 1 ) StrOut += "\n* " + valLabel[n] + ": ";
                else StrOut += "\n  " + valLabel[n] + ": ";
                StrOut += *pVal[n];
            } 
    }

    void update( float dt )// editModeButt has a de bounce time
    {
        if( !pVal ) return;// crash avoidance
     //   doShow = false;

        // using push button to set editMode
        editModeButt.update(dt);        
        int ButtEvent = editModeButt.pollEvent();
        if( ButtType == 'C' ) ButtEvent *= -1;// invert if normally closed
        if( ButtEvent == 1 )// press event
        {
            ++editMode;// cycle editMode
            editMode %= 1 + numVals;// range: 0 to numVals
            doShow = true;// move asterisk to next line
            if( pDoShow ) *pDoShow = true;
        }

        if( editMode > 0 )// && pushButtPP.isClosed )// 1, 2, 3 or 4
        {
            int potDiff = thePot.updateChange();// may re center pot while button is pressed
            bool ButtOpen = !editModeButt.isClosed;
            if( ButtType == 'C' ) ButtOpen = !ButtOpen;// invert if normally closed

            if( potDiff != 0 && ButtOpen )// button has been released
            {   
                doShow = true;
                if( pDoShow ) *pDoShow = true;
                unsigned int i = editMode - 1;
                float& val = *pVal[i];// for convenience
                val += (float)potDiff*potFactor[i];
                if( val < minVal[i] ) val = minVal[i];
                else if( val > maxVal[i] ) val = maxVal[i];
            }
        }

        if( doShow )
        {
            makeMsg();   
        }
    }
};

#endif // MENU_SETFLOATS_H