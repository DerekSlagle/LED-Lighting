#ifndef CRGB_H
#define CRGB_H

#include<stdint.h>

class CRGB
{
    public:
    static int BS;

        // used by players
    uint8_t r = 1, g = 2, b = 3;

    // test below next?
    bool operator == ( const CRGB& Lt ) const
    {
        if( r != Lt.r || g != Lt.g || b != Lt.b ) return false;
        return true;
    }

    bool operator != ( const CRGB& Lt ) const
    { return !( *this == Lt ); }

    CRGB(): r(1), g(2), b(3) {}
    CRGB( uint8_t R, uint8_t G, uint8_t B ){ r = R; g = G; b = B; }
    ~CRGB(){}
    
    // to serve as init()
    void setRGB( uint8_t R, uint8_t G, uint8_t B ){ r = R; g = G; b = B; }

    // extra not used by players
  //  uint8_t X = 3;

    void doThis();
    void doThat();
    float doAnotherThing( float& rF, unsigned int p );
};

#endif // CRGB_H
