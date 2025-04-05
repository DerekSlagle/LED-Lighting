#ifndef LIGHTPLAYER4_H
#define LIGHTPLAYER4_H

#include "Light.h"
#include "../utility/bitArray.h"

struct patternData4
{
public:
    unsigned int funcIndex = 0;
    unsigned int stepPause = 1;
    unsigned int param = 0;
    void init( unsigned int fIdx, unsigned int StepPause = 1, unsigned int Param = 0 )
    { funcIndex = fIdx; stepPause = StepPause; param = Param; }
};

class LightPlayer4
{
    protected:
    Light* pLt0 = nullptr;
    int rows = 1, cols = 1;
    // dependent. For convenience in functions
    unsigned int numLts = 1;// numLts = rows*cols

    public:
    unsigned int numPatterns = 1;// size of storage arrays
    unsigned int patternIter = 0;// 0 to numPatterns. This is index into arrays
    // the 4 Lights
    Light Lt[4];

    int getRows()const{ return rows; }
    int getCols()const{ return cols; }
    unsigned int getNumLts()const{ return numLts; }

    // new
    const patternData4* pattData = nullptr;
    // newer
    uint8_t* pStateData = nullptr;// pattern #100: Length = stateDataSz/numLts
    unsigned int stateDataSz = 0;
    // newest
    bitArray BA;// for bitwise storage of above stateData
    void setStateData( uint8_t* p_StateData, unsigned int DataSz );

    unsigned int stepTimer = 0;// timer for stepIter incrementation
    unsigned int stepIter = 0;// 0 to patternLength

    // new. Find pattern length
    unsigned int getPattLength()const;

    void init( Light& r_Lt0, int Rows, int Cols, const patternData4& rPattData, unsigned int NumPatterns );
    void setLights( Light Lt0, Light Lt1, Light Lt2, Light Lt3 )
    { Lt[0] = Lt0; Lt[1] = Lt1; Lt[2] = Lt2; Lt[3] = Lt3; }

    void update();
    void updateAfter();// writes only to onLt
    uint8_t getState( unsigned int n )const;

    // patterns
    uint8_t scrollToRight( unsigned int n, unsigned int numInGroup )const;// returns state assignment
    uint8_t scrollToLeft( unsigned int n, unsigned int numInGroup )const;// returns state assignment

    uint8_t fillFromRight( unsigned int n, unsigned int LtIdx )const;
    uint8_t fillFromLeft( unsigned int n, unsigned int LtIdx )const;

    uint8_t crissCross( unsigned int n, unsigned int numPlusLts )const;
    uint8_t alternateBlink( unsigned int n, unsigned int LtPair )const;
    uint8_t checkerBlink( unsigned int n, unsigned int LtPair )const;// checker board fill

    // 2d patterns
    uint8_t scrollColToRight( unsigned int n, unsigned int LtIdx )const;
    uint8_t scrollColToLeft( unsigned int n, unsigned int LtIdx )const;
    uint8_t scrollRowToBottom( unsigned int n, unsigned int LtIdx )const;
    uint8_t scrollRowToTop( unsigned int n, unsigned int LtIdx )const;
    uint8_t cWiseRing( int n, unsigned int LtPack )const;
    uint8_t scrollBoxIn( unsigned int n, unsigned int LtIdx )const;
    uint8_t scrollBoxOut( unsigned int n, unsigned int LtIdx )const;
    // 0 = dn rt, 1 = up lt, 2 = dn lt, 3 = up lt
    uint8_t scrollDiagonal( unsigned int n, unsigned int Mode )const;



    LightPlayer4();
    ~LightPlayer4();

    private:
};

#endif // LIGHTPLAYER4_H
