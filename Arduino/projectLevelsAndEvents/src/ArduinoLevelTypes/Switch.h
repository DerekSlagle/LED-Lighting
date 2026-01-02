#ifndef SWITCH_H
#define SWITCH_H

#include "Arduino.h"
#include "ArduinoEvent.h"

// A single pole single throw ( on or off ) switch
class Switch
{
    public:
    int pinID = 0;
    bool isClosed = false;// open circuit by default
    bool wasClosed = false;// event detection
    // lastNow for de bounce
    bool lastNow = false;
    // event members
    int myID = 0;

    // de bounce timing
    float tWait = 0.3f;
    float tTimer = tWait;// stable value
    // event handling
    void ( *onClose )(void) = nullptr;
    void ( *onOpen )(void) = nullptr;

    void init( int eventID, int PinID, float t_Wait, bool iClosed = false )
    {
        myID = eventID;
        pinID = PinID;
        pinMode( pinID, INPUT_PULLUP );
        tWait = t_Wait; 
        tTimer = tWait;// stable
        isClosed = wasClosed = lastNow = iClosed;
    }

    Switch( int eventID, int PinID, float t_Wait, bool iClosed = false )
    { init( eventID, PinID, t_Wait, iClosed ); }
    
    Switch(){}

    ArduinoEvent update( float dt )
    {
        wasClosed = isClosed;

        bool now = ( digitalRead( pinID ) == LOW );
        if( now != lastNow ) tTimer = 0.0f;// reset timer when state has changed
        if( tTimer < tWait )
        {
            tTimer += dt;
            if( tTimer >= tWait )// Ding!           
                isClosed = now;
        }

        lastNow = now;// for next call

        // event handling. When isClosed != wasClosed
        ArduinoEvent AE;// all members = 0
        AE.ID = myID;
        if(  isClosed && !wasClosed )
        { 
            if( onClose ) onClose();// just closed
            AE.type = 1;
        }
        else if( !isClosed && wasClosed ) 
        {
            if( onOpen ) onOpen();// just opened
            AE.type = -1;
        }

        return AE;
    }

    // event polling: 0 = no event, +1 = press, -1 = release
    int pollEvent()const
    {
        if( isClosed == wasClosed ) return 0;// no change
        if( isClosed && !wasClosed ) return 1;// just closed
        if( !isClosed && wasClosed ) return -1;// just opened
        return 0;// should not get here
    }
};

#endif // SWITCH_H
