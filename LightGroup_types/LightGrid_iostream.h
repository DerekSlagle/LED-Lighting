#ifndef LIGHTGRID_H
#define LIGHTGRID_H

#include<iostream>
#include<vector>
#include "../Light_types/Light.h"


// manager or owner of Light array
class LightGrid
{
    public:
    bool ownsLights = false;
    Light* pLt0 = nullptr;
    int rows = 1, cols = 1;
    unsigned int numLts()const { return static_cast<unsigned int>( rows*cols ); }
    // transparency is a property of the image stored here
    Light transLt;// transparency color
    bool isTransparent = false;

    // change image
    void Clear( Light clearLt )const;
    void RotateCW()const;// square Grid only
    void RotateCCW()const;// square Grid only
    void FlipX()const;
    void FlipY()const;

    Light* get_pLt( int Row, int Col )const { return pLt0 + Row*cols + Col; }

    // outside ownership assumed, though may re assign ownsLights
    void init( Light& rLt, unsigned int numLts );// 1d array
    LightGrid( Light& rLt, unsigned int numLts ) { init( rLt, numLts ); }
    void init( Light& rLt, int Rows, int Cols );// 2d array
    LightGrid( Light& rLt, int Rows, int Cols ) { init( Rows, Cols ); }
    // Lights are owned by *this
    void init( unsigned int numLts );// 1d array
    LightGrid( unsigned int numLts ) { init( numLts ); }
    void init( int Rows, int Cols );// 2d array
    LightGrid( int Rows, int Cols ) { init( Rows, Cols ); }

    // save all Light states and rows, cols to a file
    void toFile( std::ostream& os, const std::vector<Light>& LtTone )const;

    // init or create from images or text files
    // text file encoded with numTone Light states. Each state read as 'a', 'b',...,'a+k'
    // as index into array of just saved tones
    bool init( std::istream& is );// owner only
    LightGrid( std::istream& is ) { init( is ); }

    // re assign? ONLY if rows and cols are the same
    bool fromFile( std::istream& is );// rows and cols must agree. This changes state only.


    LightGrid( const LightGrid& ) = delete;
    LightGrid& operator = ( const LightGrid& ) = delete;
    // move copy
    LightGrid( LightGrid&& LG )
    {
        pLt0 = LG.pLt0;
        rows = LG.rows;
        cols = LG.cols;
        ownsLights = LG.ownsLights;
        // let go
        LG.pLt0 = nullptr;
        std::cout << "\nLightGrid(&&)";
    }

    LightGrid();
    ~LightGrid();

    protected:

    private:
};

#endif // LIGHTGRID_H
