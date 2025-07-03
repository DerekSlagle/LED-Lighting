#ifndef DATAPLAYER_H
#define DATAPLAYER_H

#include "Light.h"

//#include "C_RGB.h"
#include "../utility/bitArray.h"
// new
//#include "../Light_types/LightPalette.h"

class DataPlayer
{
    public:
    Light* pLt0 = nullptr;// the target Light grid
    int rows = 1, cols = 1;
    int row0 = 0, col0 = 0;// origin in grid
    int gridCols = 1, gridRows = 1;// bounding grid
 //   int drawMode = 3;// 1: is grid, 2: is all in grid, 3: is partly in grid
    int getDrawMode()const;// to replace drawMode member
    void updateIsGrid();// 1
    void updateAllIn();// 2
    void updatePartlyIn();// 3

 //   public:
    bool drawOff = true;// draw pLight[0] if true. Background color
    bool fadeAlong = false;// fade each frame into the next
    Light updateFade( unsigned int n )const;// called by update() when above is true

    // play pause seek functions
    bool isPlaying = true;// stops animation. Drawing continues
    void reStart(){ stepTimer = stepIter = 0; }
    void prevImage();
    void nextImage();
    void showImage( unsigned int n );// 0 to numSteps - 1

    // storage for bitArray
    uint8_t* pStateData = nullptr;
    unsigned int stateDataSz = 0;

    // array storage
    Light* pLight = nullptr;// to array of Lights defining the colors
    bitArray BA;// for bitwise storage of index to Light color for each state assignment
    unsigned int numColors = 2;// 2 to 16
    Light getState( unsigned int n )const;

    // transform image
    bool flipX = false, flipY = false;//
    void flipX_AllIn();// for drawMode = 1 or 2
    void flipY_AllIn();// for drawMode = 1 or 2

    // state management
    unsigned int stepTimer = 0, stepPause = 1;// timer for stepIter incrementation
    unsigned int stepIter = 0, numSteps = 1;

    // user to call update()
    void update();// write Light states to area in grid and takeStep()
    void updateZoomOut();
    void takeStep();// called by update()
    void updateBrigtness( float brtFactor );// brtFactor = 1.0f is normal brightness. 1.5f = 50% brighter, 0.5f = half bright
    const float* pBrtFactor = nullptr;// assign to trigger calls to above from update()
    // special usage
    void updateRainfall( int (*pGetCol)(void) );
    void updateRainfall_fade( int (*pGetCol)(void), bool doEase = false );

    // test or utility
    void showColors()const;// display in order 0 to numLts - 1
    void Clear( Light clearLt )const;
    const Light* get_pLt0()const { return pLt0; }

    void init( Light& r_Lt0, int GridRows, int GridCols, uint8_t& r_StateData, unsigned int DataSz, uint8_t NumColors );
    void bindToGrid(  Light& r_Lt0, int GridRows, int GridCols );
    void setGridBounds( int Row0, int Col0, int GridRows, int GridCols );
    void setTargetRect( int Rows, int Cols, int Row0, int Col0 );

    DataPlayer(){}
    ~DataPlayer(){}

    private:
};

#endif // DATAPLAYER_H
