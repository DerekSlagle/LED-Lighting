#include "CRGB.h"
#include "DataPlayer.h"

const unsigned int numLts = 256;
CRGB GridA[numLts];
uint8_t imageData[1028];

typedef CRGB Light;
Light LtArr[7];
DataPlayer DP;

void setup() 
{  
  DP.init( GridA[0], 16, 16, imageData[0], 1028, 7 );
  DP.pLt0 = LtArr;
}

void loop() 
{
  DP.update();
}
