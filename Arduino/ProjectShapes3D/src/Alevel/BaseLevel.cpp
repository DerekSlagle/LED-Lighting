#include "BaseLevel.h"

switchSPST* BaseLevel::pActButt = nullptr;
switchSPST* BaseLevel::pMenuButt = nullptr;
switchSPST* BaseLevel::pRotEncButt = nullptr;
int* BaseLevel::pRotEncDelta = nullptr;
SSD1306_Display* BaseLevel::pDisplay = nullptr;

void BaseLevel::setupBase( SSD1306_Display& r_Oled, switchSPST *p_ActButt,
    switchSPST *p_MenuButt, switchSPST *p_RotEncButt, int& rotEncDelta )
{
   pDisplay = &r_Oled;// There is no need to ever check this pointer. It is VALID
   pActButt = p_ActButt;
   pMenuButt = p_MenuButt;
   pRotEncButt = p_RotEncButt;
   pRotEncDelta = &rotEncDelta;

    return;
}

void BaseLevel::processInput()
{
}

void BaseLevel::update( float dt )
{
}

void BaseLevel::draw()const
{
}


void BaseLevel::updateDisplay()const
{
    String msg;
    // write
    pDisplay->clear();
    pDisplay->printAt( 0, 0, msg.c_str(), 1 );
    pDisplay->show();
}