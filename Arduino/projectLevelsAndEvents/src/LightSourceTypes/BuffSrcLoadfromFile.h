#ifndef BUFFSRCLOADFROMFILE_H_INCLUDED
#define BUFFSRCLOADFROMFILE_H_INCLUDED

#include "../bitArray.h"
#include "../FileParser.h"
#include "BufferSource.h"

// to type LightGrid

// caller owns and manages the LightGrid array
bool fromFile_ColorsAndData( BufferSource& BS, FileParser& fin, LightGrid*& p_LG, int& num_LG, LightGrid& TargetGrid, Light clearLt );// all frames in 1 file
bool fromFile_ColorsOnly( BufferSource& BS, FileParser& fin, LightGrid*& p_LG, int& num_LG, LightGrid& TargetGrid, Light clearLt );// 1 file per frame
bool loadColors( LightGrid& LG, const char* fileName, Light clearLt );// above calls for each frame

#endif // BUFFSRCLOADFROMFILE_H_INCLUDED
