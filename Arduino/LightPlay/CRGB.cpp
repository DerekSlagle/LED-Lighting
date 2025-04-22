#include "CRGB.h"

int CRGB::BS = 10;

void CRGB::doThis()
{
    CRGB Lt;
    Lt.r = 2;
    Lt.g = 7;
    Lt.b = 12;
    return;
}

void CRGB::doThat()
{
    return;
}

float CRGB::doAnotherThing( float& rF, unsigned int p )
{
    rF = 3.14f;
    return rF*p;
}
