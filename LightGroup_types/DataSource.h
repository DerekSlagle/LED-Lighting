#ifndef DATASOURCE_H
#define DATASOURCE_H

#include<fstream>// REMOVE for Arduino Usage is in: dtor
#include<fstream>// use FileParser on Arduino

#include "LightSource.h"
#include "../utility/bitArray.h"

// owns colors and color index data
class DataSource : public LightSource
{
    public:
    bitArray BA;// owns the color index data
    Light* pColor = nullptr;// owns the colors
    int* pRfcCount = nullptr;
    unsigned int numColors = 2;// 2 to 16
    unsigned int frameIter = 0, numFrames = 1;
    bool isPlaying = true;// stops animation
    // timing frame rate
    float tFrame = 0.04f, tElapFrame = 0.0f;// 25 fps
    bool update( float dt );// true when frame changes. Also updates position
    void nextFrame()
    { frameIter = ( 1 + frameIter )%numFrames; }
    void prevFrame()
    {
        if( frameIter > 0 ) --frameIter;
        else frameIter = numFrames - 1;
    }

    // allocates to BA.pByte, pColor and pRfcCount and assigns all + rows, cols, numColors
    // USER to assign: row0, col0 and tFrame
    void setDataAndColors( LightGrid& rTgt, std::ifstream& fin );

    virtual Light getLt( int r, int c )const;// PV in base

    DataSource();
    DataSource& operator = ( const DataSource& DS );// assignment
    DataSource( const DataSource& DS ){ *this = DS; }// copy ctor

    void decRfcCount();// called by oper= and dtor
    virtual ~DataSource();// frees the index and color arrays

    protected:

    private:
};

#endif // DATASOURCE_H
