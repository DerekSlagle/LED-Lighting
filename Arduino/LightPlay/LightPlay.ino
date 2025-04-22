#include "LightPlayer2.h"// type CRGB is included here

const unsigned int numLts = 256;
CRGB GridA[numLts];
typedef CRGB Light;

LightPlayer2 LP2;

void setup() 
{
  Light color1, color2;
  patternData pattData[4];
  pattData[0].init( 1,2,3 );
  pattData[1].init( 1,2,3 );
  pattData[2].init( 1,2,3 );
  pattData[3].init( 1,2,3 );
  LP2.init( GridA[0], 16, 16, pattData[0], 4 );
  LP2.onLt.setRGB( 0, 255, 60 );
  LP2.offLt.setRGB( 0, 60, 255 );// legal CRGB code

}

void loop() 
{
  LP2.update();
}
