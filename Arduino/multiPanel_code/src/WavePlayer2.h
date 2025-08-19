#ifndef WAVEPLAYER2_H
#define WAVEPLAYER2_H

#include<cmath>
//#include<iostream>
#include "Light.h"

class WavePlayer2
{
    public:
    // traveling wave to right
    float tElapRt = 0.0f, periodRt = 1.0f;// period = wvLen/Spd below
    float wvLenRt = 10.0f, wvSpdRt = 20.0f;// in array index
    float* C_Rt = nullptr;// provide external storage for Fourier coefficients
    unsigned int nTermsRt = 0;// in series
    float AmpRt = 1.0f;

    // traveling wave to left
    float tElapLt = 0.0f, periodLt = 1.0f;// actually = period = wvLen/Spd
    float wvLenLt = 10.0f, wvSpdLt = 20.0f;
    float* C_Lt = nullptr;// provide external storage
    unsigned int nTermsLt = 0;
    float AmpLt = 1.0f;// AmpLt = 1.0f - AmpRt

//    Light hiLt, loLt;// interpolate
    float frHi = 0.0f, fgHi = 0.0f, fbHi = 0.0f;// store once
    float frLo = 0.0f, fgLo = 0.0f, fbLo = 0.0f;// used in update
    void setHiLight( const Light& Lt ){ frHi = (float)Lt.r; fgHi = (float)Lt.g; fbHi = (float)Lt.b; }
    void setLoLight( const Light& Lt ){ frLo = (float)Lt.r; fgLo = (float)Lt.g; fbLo = (float)Lt.b; }

    void update( float dt );
    void update_tElapOnly( float dt );
    Light getState( int n )const;
    Light getLight( int r, int c )const;

    void init( Light& r_Lt0, unsigned int GridRows, unsigned int GridCols, Light HiLt, Light LoLt );
    void setWaveData( float AmpRt, float wvLen_lt, float wvSpd_lt, float wvLen_rt, float wvSpd_rt );
    void setSeriesCoeffs( float* C_rt, unsigned int n_TermsRt, float* C_lt, unsigned int n_TermsLt );

    // set target area within Grid
    void bindToGrid( Light& r_Lt0, unsigned int GridRows, unsigned int GridCols );
    void setGridBounds( int Row0, int Col0, int GridRows, int GridCols )
    { row0 = Row0; col0 = Col0; gridRows = GridRows; gridCols = GridCols; setDrawMode(); }
    void setTargetRect( int Rows, int Cols, int Row0, int Col0 );


    WavePlayer2(){}
    ~WavePlayer2(){}

    // access
    int getRows()const{ return rows; }
    int getCols()const{ return cols; }
    int getRow0()const{ return row0; }
    int getCol0()const{ return col0; }
    unsigned int getNumLts()const{ return numLts; }
    // setters
    void setRows( int Rows ){ rows = Rows; setDrawMode(); }
    void setCols( int Cols ){ cols = Cols; setDrawMode(); }
    void setRow0( int Row0 ){ row0 = Row0; setDrawMode(); }
    void setCol0( int Col0 ){ col0 = Col0; setDrawMode(); }

    Light* pLt0 = nullptr;
    protected:
    
    int rows = 1, cols = 1;
    int row0 = 0, col0 = 0;// origin in grid
    int gridCols = 1, gridRows = 1;// bounding grid
    // dependent. For convenience in functions
    int numLts = 1;// numLts = rows*cols
    int drawMode = 3;// 1: is grid, 2: is all in grid, 3: is partly in grid
    void setDrawMode();
    void updateIsGrid();// 1
    void updateAllIn();// 2
    void updatePartlyIn();// 3

    private:
};

#endif // WAVEPLAYER2_H
