#ifndef PLAYERFUNCS_H_INCLUDED
#define PLAYERFUNCS_H_INCLUDED

#include<stdint.h>
#include<iostream>
#include<vector>
#include "DataPlayer.h"
#include "LightPlayer2.h"

bool initDataPlayer( DataPlayer& DP, Light& rLt0, int GridRows, int GridCols, std::vector<uint8_t>& DataVec, std::istream& is );
bool initPatternPlayer( LightPlayer2& LP, Light& rLt0, std::vector<patternData>& pattDataVec, std::istream& is );

#endif // PLAYERFUNCS_H_INCLUDED
