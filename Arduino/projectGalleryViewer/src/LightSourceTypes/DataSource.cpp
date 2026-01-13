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

    if( pRfcCount != DS.pRfcCount )// team change
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
    //    std::cout << "\nDataSource dtor: rfcCount = " << *pRfcCount;
        if( *pRfcCount < 1 )// == 0 instead?
        {
            delete pRfcCount;
            pRfcCount = nullptr;
            if( pColor ) delete [] pColor;
            pColor = nullptr;
            if( BA.pByte ) delete [] BA.pByte;
            BA.pByte = nullptr;
         //   std::cout << " Bye!";
        }
    }
}

DataSource::~DataSource()
{
    decRfcCount();
}

/*
bool DataSource::update( float dt )// true when frame changes
{
    if( isMoving ) updatePosition(dt);
    if( !isPlaying ) return false;
    if( numFrames == 1 ) return false;// just displaying 1 image

    tElapFrame += dt;
    if( tElapFrame >= tFrame )
    {
        tElapFrame -= tFrame;
        frameIter = ( 1 + frameIter )%numFrames;
        return true;
    }
    return false;
}
*/

void DataSource::setDataAndColors( LightGrid& rTgt, FileParser& fin )// allocate to BA.pByte and pColor
{
    pTgt = &rTgt;
    pRfcCount = new int;
    *pRfcCount = 1;
    // file input
    fin >> rows >> cols;
    fin >> row0 >> col0;
    int BS = 0;
    fin >> BS;
    BA.sizeBits = BS;

    int NC = 0;
    fin >> NC;
    numColors = NC;// not for Json
    pColor = new Light[ numColors ];
    for( unsigned int n = 0; n < numColors; ++n )
    {
        int rd, gn, bu;
        fin >> rd >> gn >> bu;
        pColor[n].setRGB(rd,gn,bu);
    }


    int CAP = 0;
    fin >> CAP;
    BA.capBytes = CAP;
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
    if( doBlend ) return getBlendedLight(n);

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

Light DataSource::getBlendedLight( int n )const
{
    int nextFrIter = ( 1 + frameIter )%numFrames;// playForward true
//    if( !playForward )
 //   {
  //      nextFrIter = frameIter - 1;
 //       if( nextFrIter < 0 ) nextFrIter = numFrames - 1;
 //   }

    float U = tElapFrame/tFrame;
    if( U > 1.0f ) U = 1.0f;// next: 0 to 1
    float V = 1.0f - U;// current: 1 to 0

    // index to same Light last frame
    int nextN = n + rows*cols*( nextFrIter - frameIter );

    uint8_t idx = 0, nextIdx = 0;
    if( numColors > 8 )// 9 to 16
    {
        nextIdx = BA.getQuadBit( nextN );
        idx = BA.getQuadBit( n );// 4 bits per index
    }
    else if( numColors > 4 )// 5 to 8
    {
        nextIdx = BA.getTriBit( nextN );
        idx = BA.getTriBit( n );// 3 bits per index
    }
    else if( numColors > 2 )// 3 or 4
    {
        nextIdx = BA.getDblBit( nextN );
        idx = BA.getDblBit( n );// 3 bits per index
    }
    else// 2 colors
    {
        nextIdx = BA.getBit( nextN );
        idx = BA.getBit( n ) ? 1 : 0;// 1 bit per index
    }

    // blend = next*U + current*V
    float fr = ( pColor[ nextIdx ].r )*U + ( pColor[ idx ].r )*V;
    float fg = ( pColor[ nextIdx ].g )*U + ( pColor[ idx ].g )*V;
    float fb = ( pColor[ nextIdx ].b )*U + ( pColor[ idx ].b )*V;
    return Light( fr, fg, fb );
}
