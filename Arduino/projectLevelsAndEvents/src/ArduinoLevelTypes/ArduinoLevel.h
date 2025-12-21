#ifndef ARDUINOLEVEL_H
#define ARDUINOLEVEL_H

#include "ArduinoEvent.h"

// base class for levels targeting Arduino
class ArduinoLevel
{
    public:
    // play/pause calls to update(). Allow 1 call at a time when paused
    static bool run_update, step_update;
    static ArduinoLevel* pCurrLvl;
    // static SD* in VS code for access to SD drive
    // read setup data from files?
    static bool update_stat( float dt );
    static void draw_stat();

    // regular members
    int actButtID = 1;// a 2nd may be to scroll through a menu

    // each derived defines a setup procedure
    virtual bool update( float dt ) = 0;// true if image changes on target grid
    virtual void draw()const = 0;
 //   virtual void handleEvent( ArduinoEvent& rEvent ){}// base does nothing
    virtual bool handleEvent( ArduinoEvent& rEvent )
    {
        if( rEvent.type == -1 && rEvent.ID == actButtID ) return false;// Quit
        return true;// still running
    }

    // no copy
    ArduinoLevel( const ArduinoLevel& ) = delete;
    ArduinoLevel& operator = ( const ArduinoLevel& ) = delete;

    ArduinoLevel(){}
    virtual ~ArduinoLevel(){}
};

#endif // ARDUINOLEVEL_H
