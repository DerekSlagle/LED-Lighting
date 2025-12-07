#include "DataSource.h"

DataSource::DataSource()
{
    //ctor
}

DataSource& DataSource::operator = ( const DataSource& DS )// assignment
{
    // todo: what if a default constructed DS is passed?
    if( this == &DS ) return *this;// in case of self assign. Should be impossible due to ctor
    // base members
    pTgt = DS.pTgt;
    row0 = 0;
    col0 = 0;// origin on target
    rows = DS.rows;// rows in draw area
    cols = DS.cols;// area drawn to
    // user may assign. No setter needed
    rShift = 0;
    cShift = 0;
    wrapRow = DS.wrapRow;
    wrapCol = DS.wrapCol;// wrap on rows x cols sub region. NOT implemented yet
    flipX = DS.flipX;
    flipY = DS.flipY;
    // draw an outline
    outLineThickness = DS.outLineThickness;
    outlineLight = DS.outlineLight;

    // derived members
    BA.pByte = DS.BA.pByte;
    BA.capBytes = DS.BA.capBytes;
    BA.sizeBits = DS.BA.sizeBits;

    pColor = DS.pColor;
    numColors = DS.numColors;
    decRfcCount();// decrement old rfcCount. Free if == 0
    // new
    pRfcCount = DS.pRfcCount;
    if( pRfcCount ) ++( *pRfcCount );// increment new rfcCount

    frameIter = 0;
    numFrames = DS.numFrames;
    isPlaying = DS.isPlaying;
    // timing frame rate
    tFrame = DS.tFrame;
    tElapFrame = 0.0f;

    return *this;
}

void DataSource::decRfcCount()// called by oper= and dtor
{
    if( pRfcCount )
    {
        --(*pRfcCount);
        std::cout << "\nDataSource dtor: rfcCount = " << *pRfcCount;
        if( *pRfcCount < 1 )// == 0 instead?
        {
            delete pRfcCount;
            pRfcCount = nullptr;
            if( pColor ) delete [] pColor;
            pColor = nullptr;
            if( BA.pByte ) delete [] BA.pByte;
            BA.pByte = nullptr;
            std::cout << " Bye!";
        }
    }
}

DataSource::~DataSource()
{
    decRfcCount();
}

bool DataSource::update( float dt )// true when frame changes
{
    if( isMoving ) updatePosition(dt);
    if( !isPlaying ) return false;
    if( numFrames == 1 ) return false;// just displaying 1 image

    tElapFrame += dt;
    if( tElapFrame >= tFrame )
    {
        tElapFrame = 0.0f;
        frameIter = ( 1 + frameIter )%numFrames;
        return true;
    }
    return false;
}

void DataSource::setDataAndColors( LightGrid& rTgt, std::ifstream& fin )// allocate to BA.pByte and pColor
{
    pTgt = &rTgt;
    pRfcCount = new int;
    *pRfcCount = 1;
    // file input
    fin >> numColors;
    pColor = new Light[ numColors ];
    for( unsigned int n = 0; n < numColors; ++n )
    {
        unsigned int rd, gn, bu;
        fin >> rd >> gn >> bu;
        pColor[n].setRGB(rd,gn,bu);
    }

    fin >> rows >> cols;
    fin >> BA.sizeBits >> BA.capBytes;
    BA.pByte = new uint8_t[ BA.capBytes ];
    int inByte = 0;
    for( unsigned int n = 0; n < BA.capBytes; ++n )
    {
        fin >> inByte;
        BA.pByte[n] = inByte;
    }

    // numFrames
    if( numColors > 8 )// 9 to 16
    {
        numFrames = BA.sizeBits/( 4*rows*cols );
    }
    else if( numColors > 4 )// 5 to 8
    {
        numFrames = BA.sizeBits/( 3*rows*cols );
    }
    else if( numColors > 2 )// 3 or 4
    {
        numFrames = BA.sizeBits/( 2*rows*cols );
    }
    else// 2 colors
    {
        numFrames = BA.sizeBits/( rows*cols );
    }
}

Light DataSource::getLt( int r, int c )const
{
    unsigned int n = rows*cols*frameIter + r*cols + c;
    uint8_t idx = 0;
    if( numColors > 8 )// 9 to 16
    {
    //    n += 4*rows*cols*frameIter;// offset to frame data
        idx = BA.getQuadBit( n );// 4 bits per index
    }
    else if( numColors > 4 )// 5 to 8
    {
     //   n += 3*rows*cols*frameIter;
        idx = BA.getTriBit( n );// 3 bits per index
    }
    else if( numColors > 2 )// 3 or 4
    {
    //    n += 2*rows*cols*frameIter;
        idx = BA.getDblBit( n );// 3 bits per index
    }
    else// 2 colors
    {
     //   n += rows*cols*frameIter;
        idx = BA.getBit( n ) ? 1 : 0;// 1 bit per index
    }

    return pColor[ idx ];
}
