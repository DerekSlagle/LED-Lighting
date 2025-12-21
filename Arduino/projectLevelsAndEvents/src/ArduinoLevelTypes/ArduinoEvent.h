#ifndef ARDUINOEVENT_H
#define ARDUINOEVENT_H

// Update devices to generate events. push event into stack.
// ArduinoLevel::handleEvent( ArduinoEvent ); until stack is empty
class ArduinoEvent
{
    public:
        // button pressed = 1, released = -1, slidePot.steadyVal changed = 2
    int type = 0;// no event

    // map to devices: pushButtA = 1, pushButtB = 2, potA = 1, potB = 2, etc
    int ID = 0;// may re use values because different type

    // some types have a value. Eg slidePot::steadyVal
    int value = 0;

    ArduinoEvent(){}
    ~ArduinoEvent(){}

    protected:

    private:
};

#endif // ARDUINOEVENT_H
