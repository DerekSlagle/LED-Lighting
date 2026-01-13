#include "BuffSrcLoadfromFile.h"

bool fromFile_ColorsAndData( BufferSource& BS, FileParser& fin, LightGrid*& p_LG, int& num_LG, LightGrid& TargetGrid, Light clearLt )// 1 file
{
    float tFrame = 0.1f;
    fin >> tFrame;
//    fin >> BA.sizeBits;
    int Rows = 0, Cols = 0;
    fin >> Rows >> Cols;
    fin >> BS.row0 >> BS.col0;
    // index data
    bitArray BitArr;
    int SB = 0;
    fin >> SB;
    BitArr.sizeBits = SB;

    // color data
    int NumColors = 0;
    fin >> NumColors;
    Light* pColor = new Light[ NumColors ];
    int rd = 0, gn = 0, bu = 0;
    for( int n = 0; n < NumColors; ++n )
    {
        fin >> rd >> gn >> bu;
        pColor[n].setRGB(rd,gn,bu);
    }

    // index data
    int Cap = 0;
    fin >> Cap;
    BitArr.capBytes = Cap;
    BitArr.pByte = new uint8_t[ BitArr.capBytes ];
    int temp = 0;// transfer value to uint8_t
    for( unsigned int n = 0; n < BitArr.capBytes; ++n )
    {
        fin >> temp;
        BitArr.pByte[n] = temp;
    }

    // allocate the LightGrids
    unsigned int bitsPerLt = 0;
    if( NumColors > 8 && NumColors <= 16 )
        bitsPerLt = 4;
    else if( NumColors > 4 )// 5,6,7,8
        bitsPerLt = 3;
    else if( NumColors > 2 )// 3 or 4
        bitsPerLt = 2;
    else// 2 colors
        bitsPerLt = 1;

    // number of frames = number of LightGrids
    int newNum_LG = BitArr.sizeBits/( Rows*Cols*bitsPerLt );
 //   numAni_LG = BA.sizeBits/( Rows*Cols*bitsPerLt );
 //   if( numAni_LG < 10 )// proceed
    if( newNum_LG < 10 )// TEMP
    {
        // check for existing allocation
        // This function will be called to load a new animation
        if( p_LG )// shall delete
        {
            if( num_LG != 1 ) delete [] p_LG;
            else delete p_LG;
            p_LG = nullptr;
            num_LG = 0;
        }
        // allocate here. CLEANUP in destructor
        if( !p_LG )// may have saved above
        {
            num_LG = newNum_LG;
            p_LG = new LightGrid[ num_LG ];
        }
        // final check
        if( !p_LG ) return false;
        // proceed

        // CLEANUP in destructor
     //   pAni_LG = new LightGrid[ numAni_LG ];// all above to replace this one line

        unsigned int LtCount = 0;
        for( int n = 0; n < num_LG; ++n )
        {
            p_LG[n].init( Rows, Cols );// Lights allocated dynamically
            for( int k = 0; k < Rows*Cols; ++k )
            {
                uint8_t val = 0;
                if( bitsPerLt == 4 )
                    val = BitArr.getQuadBit( LtCount );
                else if( bitsPerLt == 3 )
                    val = BitArr.getTriBit( LtCount );
                else if( bitsPerLt == 2 )
                    val = BitArr.getDblBit( LtCount );
                else val = BitArr.getBit( LtCount ) ? 1 : 0;

                p_LG[n].pLt0[k] = pColor[ val ];
                ++LtCount;
            }
        }

        BS.setSource( p_LG[0], num_LG, tFrame );
        BS.setTarget( TargetGrid );
    }
    else
    {
        return false;
    }
   
    // cleanup color and index data
    delete [] pColor;
    delete [] BitArr.pByte;

    // ready
  //  BS.draw();

    return true;
}

// caller owns and manages the LightGrids
bool fromFile_ColorsOnly( BufferSource& BS, FileParser& fin, LightGrid*& p_LG, int& num_LG, LightGrid& TargetGrid, Light clearLt )// 1 file per frame
{
    float tFrame = 0.1f;
    fin >> tFrame;
    int Rows = 0, Cols = 0;
    fin >> Rows >> Cols;// correct here.
    fin >> BS.row0 >> BS.col0;

    if( p_LG )// shall delete
    {
        if( num_LG != 1 ) delete [] p_LG;
        else delete p_LG;
        p_LG = nullptr;
        num_LG = 0;
    }

    // number of frames = number of files
    fin >> num_LG;// argument write
    if( num_LG > 0 && num_LG < 20 )// proceed
    {
        if( num_LG == 1 )
            p_LG = new LightGrid;
        else
            p_LG = new LightGrid[ num_LG ];

        // for each frame
        for( int n = 0; n < num_LG; ++n )
        {
            p_LG[n].init( Rows, Cols );// Lights allocated dynamically
            String frameFname;
            fin >> frameFname;
            loadColors( p_LG[n], frameFname.c_str(), clearLt );
        }

        BS.setSource( p_LG[0], num_LG, tFrame );
        BS.setTarget( TargetGrid );
    }
    else
    {
        return false;
    }

    return true;
}

bool loadColors( LightGrid& LG, const char* fileName, Light clearLt )
{
    if( !LG.pLt0 ) return false;// no storage
    // open the file for the image data
    FileParser FP( fileName );
    
    int Cols = 1, Rows = 6;
    FP >> Rows >> Cols;
    if( Cols > LG.cols ) Cols = LG.cols;// check
    if( Rows > LG.rows ) Rows = LG.rows;// check

    return FP.readLightArray( LG.pLt0, LG.rows*LG.cols, clearLt );

    /*
    int rd, gn, bu, al;// each Light
    for( int X = 0; X < Cols; ++X )
    {
        for( int Y = 0; Y < Rows; ++Y )
        {
          FP >> rd >> gn >> bu >> al;
          if( al == 0 )
          {
            LG.pLt0[ Cols*Y + X ] = clearLt;
          }
          else LG.pLt0[ Cols*Y + X ].setRGB( rd, gn, bu );
        }
    }

    return true;
    */
}
