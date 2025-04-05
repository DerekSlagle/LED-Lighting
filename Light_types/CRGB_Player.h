#ifndef CRGB_PLAYER_H
#define CRGB_PLAYER_H

#include<stdint.h>

// remove this declaration and substitute #include"CRGB.h" or appropriate
class CRGB
{
public:
    uint8_t r = 1, g = 2, b = 3;

    // test below next?
    bool operator == ( const CRGB& Lt ) const
    {
        if( r != Lt.r || g != Lt.g || b != Lt.b ) return false;
        return true;
    }

    bool operator != ( const CRGB& Lt ) const
    { return !( *this == Lt ); }

    CRGB(){ r = 1; g = 2; b = 3; }
    CRGB( uint8_t R, uint8_t G, uint8_t B ){ r = R; g = G; b = B; }
};

// minimal test for compatible use of the "real" type CRGB
class CRGB_Player
{
    public:
    CRGB* pLt0 = nullptr;
    unsigned int numLts = 0;

    // define enough to test usage
    void init( CRGB* p_Lt0, unsigned int NumLts );
    void update( CRGB onLt, CRGB offLt )const;

    // will these be an issue? Go with default?
    CRGB_Player(){}
    ~CRGB_Player(){}

    protected:

    private:
};

#endif // CRGB_PLAYER_H
