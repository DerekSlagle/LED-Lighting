#ifndef LIGHTPANEL_H
#define LIGHTPANEL_H

#include "Light.h"

// maps from a bounding array of Lights ( pSrc )
// to a same sized array tiled to panels
class LightPanel
{
    public:
    // location in source array
    Light *pSrc0 = nullptr;
    int gridRows = 8, gridCols = 8;// large bounding grid
    int row0 = 0, col0 = 0;// origin of source rectangle in the grid (top,left)
    int srcRows = 1, srcCols = 1;// region in the grid (height,width)
    int tgtRows = 1, tgtCols = 1;// panel dimensions
    
    // additional processing
    int type = 1;// no added process. 2 = serpentine order. Reverse odd rows.
    void reverseOddRows( bool inTarget )const;// for type = 2
    // rotate before writing to target array
    int rotIdx = 0;// 0 = no rotation, +1 = 90 cw, -1 = 90 ccw, +-2 = 180 degree
    void rotateCW()const;// rotate image 90 degrees clockwise
    void rotateCCW()const;// 90 degrees counter clockwise
    void rotate180()const;// rotate image 180 degrees

    // location in target array
    Light* pTgt0 = nullptr;// start address for panel
    // set source area and panel dimensions
    bool init( Light *p_Src0, int GridRows, int GridCols, int SrcRows, int SrcCols, int SrcRow0, int SrcCol0, int TgtRows, int TgtCols );

    void update()const;// write to target array
    void updateSideways()const;// if tgtCols == srcRows, etc

    LightPanel(){}
    ~LightPanel(){}

    protected:

    private:
};

#endif // LIGHTPANEL_H
