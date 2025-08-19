#ifndef DATAPLAYER_INITFUNCS_H_INCLUDED
#define DATAPLAYER_INITFUNCS_H_INCLUDED

#include "DataPlayer.h"

// all return # of bytes storage required
// Must assign ByteArr and ColorArr storage 1st
unsigned int numBytes_dogAni();
unsigned int numBits_dogAni();
unsigned int numColors_dogAni();
unsigned int init_dogAniData( Light* LightGrid, DataPlayer& DP, uint8_t* ByteArr, Light* ColorArr );

unsigned int numBytes_alphaAni();
unsigned int numBits_alphaAni();
unsigned int numColors_alphaAni();
unsigned int init_alphaAniData( Light* LightGrid, DataPlayer& DP, uint8_t* ByteArr, Light* ColorArr );

unsigned int numBytes_seagullAni();
unsigned int numBits_seagullAni();
unsigned int numColors_seagullAni();
unsigned int init_seagullAniData( Light* LightGrid, DataPlayer& DP, uint8_t* ByteArr, Light* ColorArr );

unsigned int numBytes_waterAni();
unsigned int numBits_waterAni();
unsigned int numColors_waterAni();
unsigned int init_waterAniData( Light* LightGrid, DataPlayer& DP, uint8_t* ByteArr, Light* ColorArr );

unsigned int numBytes_spiderAni();
unsigned int numBits_spiderAni();
unsigned int numColors_spiderAni();
unsigned int init_spiderAniData( Light* LightGrid, DataPlayer& DP, uint8_t* ByteArr, Light* ColorArr );

#endif // DATAPLAYER_INITFUNCS_H_INCLUDED
