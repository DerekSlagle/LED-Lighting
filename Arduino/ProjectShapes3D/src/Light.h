#ifndef LIGHT_H
#define LIGHT_H

#include <FastLED.h>
#include<stdint.h>
typedef CRGB Light;

/*
class Light
{
    public:
    uint8_t r = 1, g = 2, b = 3;

 //   void init( uint8_t Rd, uint8_t Gn, uint8_t Bu )
    void setRGB( uint8_t Rd, uint8_t Gn, uint8_t Bu )
    { r = Rd; g = Gn; b = Bu;  }
    Light( uint8_t Rd, uint8_t Gn, uint8_t Bu ){ setRGB( Rd, Gn, Bu ); }
    Light(){}

    bool operator == ( const Light& Lt ) const
    {
        if( r != Lt.r || g != Lt.g || b != Lt.b ) return false;
        return true;
    }
    bool operator != ( const Light& Lt ) const
    {

        return !( *this == Lt );
    }
};
*/

#endif // LIGHT_H
