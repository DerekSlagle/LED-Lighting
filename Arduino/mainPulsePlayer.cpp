#include <Arduino.h>
#include <FastLED.h>
#include "PulsePlayer.h"

const int gridRows = 1, gridCols = 240;// landscape
const unsigned int numLEDs = static_cast<unsigned int>( gridRows*gridCols );
CRGB leds[numLEDs];// the lights
Light clearColor;


const int numPP = 4;
PulsePlayer PP[numPP];
void setupPP();

void setup()
{
    clearColor.setRGB(0,0,0);
    setupPP();

    FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(32);


}

uint32_t lastTime = micros();
float tElapStart = 0.0f, tStart = 0.8f;
int idxStart = 0;
void loop()
{
    uint32_t currTime = micros();
    float dtLoop = 0.000001f*( currTime - lastTime );
    lastTime = currTime;

    tElapStart += dtLoop;
    if( tElapStart > tStart )
    {
        PP[idxStart].Start();
        tElapStart = 0.0f;
        idxStart = ( 1 + idxStart )%numPP;
    }

    FastLED.clear();
    // draw
    for( int n = 0; n < numPP; ++n )
        PP[n].update( dtLoop );

	FastLED.show();
}

void setupPP()
{
    int W_Pulse = 8;
    float Speed = 10.0f;
    PP[0].init( leds[0], numLEDs, Light(200,0,0), 24, 120.0f, false );
    PP[1].init( leds[0], numLEDs, Light(0,200,0), 32, -100.0f, false );
    PP[2].init( leds[0], numLEDs, Light(0,0,200), 40, 80.0f, false );
    PP[3].init( leds[0], numLEDs, Light(100,0,100), 28, -140.0f, false );
    for( int n = 0; n < numPP; ++n )
    {
        PP[n].funcIdx = 0;
    //    PP[n].tElap = 1.0f*n;
    }
}