#ifndef LIGHTPALETTE_H
#define LIGHTPALETTE_H

#include "Light.h"

class LightPalette
{
    public:
    uint8_t* pByte = nullptr;// array to be provided on Arduino for this data
    unsigned int numBytes = 0;//
//    unsigned int numLights = 0;// numBytes = 3*numLights

    Light Lt( unsigned int n )const { return Light( pByte[3*n], pByte[3*n+1], pByte[3*n+2] ); }
    uint8_t rd( unsigned int n )const { return pByte[3*n]; }
    uint8_t gn( unsigned int n )const { return pByte[3*n+1]; }
    uint8_t bu( unsigned int n )const { return pByte[3*n+2]; }

    void init( uint8_t* p_Byte, unsigned int NumBytes )
    { pByte = p_Byte; numBytes = NumBytes; }


    LightPalette(){}
    ~LightPalette(){}

    protected:

    private:
};

#endif // LIGHTPALETTE_H
