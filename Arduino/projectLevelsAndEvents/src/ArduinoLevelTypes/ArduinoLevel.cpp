#include "ArduinoLevel.h"

// static members
bool ArduinoLevel::run_update = true;
bool ArduinoLevel::step_update = true;
ArduinoLevel* ArduinoLevel::pCurrLvl = nullptr;

bool ArduinoLevel::update_stat( float dt )
{
    if( !pCurrLvl ) return false;

    if( run_update || step_update )
    {
        step_update = false;
        return pCurrLvl->update(dt);
    }

    return false;
}

void ArduinoLevel::draw_stat()
{
    if( !pCurrLvl ) return;
    pCurrLvl->draw();
}
