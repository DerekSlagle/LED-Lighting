#include "DataPlayer.h"

const int intSz = sizeof( int );
const unsigned int numLts = 256;
int GridRows = 8;
int GridCols = 8;
Light* GridA;//[2];//[numLts];
uint8_t imageData[1280];

Light LtArr[16];
DataPlayer DP1;

void makeImageByteReport( const DataPlayer & DP );
bool getDPdata( DataPlayer& DP );

void setup() {
//  DP1.init( GridA[0], 16, 16, imageData[0], 1028, 7 );
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  DP1.pLt0 = LtArr;

  Serial.println( "\nReady for data" );
  if( getDPdata( DP1 ) )
  {
     Serial.println( "Player load Good!" );
     makeImageByteReport( DP1 );
  }
  else Serial.println( "Player load Bad!" );

}

void loop() {
  // put your main code here, to run repeatedly:

}

void makeImageByteReport( const DataPlayer& DP )
{
   Serial.print( "numBytes = " ); Serial.println( DP.BA.ByteCapacity() );
   unsigned int chkSum = 0;
   for( unsigned int n = 0; n < DP.BA.ByteCapacity(); ++n )
    {
        if( n%10 == 0 ) Serial.print( '\n' );
    //    rptOut << (int)DP.BA.pByte[n] << '\t';
        Serial.print( (int)DP.BA.pByte[n] );
        Serial.print( '\t' );
        chkSum += (int)DP.BA.pByte[n];
    }

    Serial.print( "\n chkSum = " );
    Serial.println( chkSum );
}

bool getDPdata( DataPlayer& DP )
{
  
  while( Serial.available() < 4*intSz );
  int Rows = Serial.parseInt();
  int Cols = Serial.parseInt();
  int Row0 = Serial.parseInt();
  int Col0 = Serial.parseInt();

  Serial.print( "Rows = " ); Serial.println( Rows );
  Serial.print( "Cols = " ); Serial.println( Cols );
  Serial.print( "Row0 = " ); Serial.println( Row0 );
  Serial.print( "Col0 = " ); Serial.println( Col0 );
  //Serial.print( "Serial.avail = " ); Serial.println( Serial.available() );

  while( Serial.available() < 3*intSz );
  DP.stepPause = Serial.parseInt();
  int DrawOff = Serial.parseInt();
  DP.drawOff = DrawOff;
  int FadeAlong = Serial.parseInt();
  DP.fadeAlong = FadeAlong;  
  Serial.print( "stepPause = " ); Serial.println( DP.stepPause );
  Serial.print( "drawOff = " ); Serial.println( DP.drawOff );
  Serial.print( "fadeAlong = " ); Serial.println( DP.fadeAlong );
  Serial.read();
  Serial.print( "Serial.avail = " ); Serial.println( Serial.available() );

  while( Serial.available() < intSz );
  int numColors = Serial.parseInt();
  Serial.print( "numColors = " ); Serial.println( numColors );

  unsigned int rd = 0, gn = 0, bu = 0;
  for( unsigned int k = 0; k < numColors; ++k )
  {
    while( Serial.available() < 3*intSz );
    rd = Serial.parseInt();
    gn = Serial.parseInt();
    bu = Serial.parseInt();
    LtArr[k].r = rd;
    LtArr[k].g = gn;
    LtArr[k].b = bu;
    Serial.print( "color: " ); Serial.print( rd );
    Serial.print( ", " ); Serial.print( gn );
    Serial.print( ", " ); Serial.println( bu );
  }
  Serial.read();
  Serial.print( "Serial.avail = " ); Serial.println( Serial.available() );

  while( Serial.available() < 2 );
  int dataInBits = Serial.parseInt();
  Serial.print( "dataInBits = " ); Serial.println( dataInBits );
  unsigned int dataSz = 0;

  if( dataInBits != 0 )
    {
        if( numColors > 4 && numColors <= 16 )
        {
         //   std::cout << "\n initDataPlay(): 16 colors from dblBits";
            unsigned int numQuadBits;
            while( Serial.available() < 2 );
            numQuadBits = Serial.parseInt();
            Serial.print( "numQuadBits = " ); Serial.println( numQuadBits );
            if( numQuadBits%2 == 0 ) dataSz = numQuadBits/2;
            else dataSz = numQuadBits/2 + 1;
            Serial.print( "dataSz in Bytes = " ); Serial.println( dataSz );
            Serial.print( "Serial.avail = " ); Serial.println( Serial.available() );
            DP.init( LtArr[0], GridRows, GridCols, imageData[0], dataSz, numColors );
            unsigned int inVal = 0;
            DP.BA.Clear();
            unsigned int k = 0;
            for( k = 0; k < numQuadBits; ++k )
            {
          //      if( Serial.available() == 1 ) Serial.read();// flush new line
           //     while( Serial.available() < 2 );
                if( k%(Cols) == 0 )
                {
                  Serial.print( "k = ");
                  Serial.print( k );
                //  Serial.read();
                  if( Serial.available() == 1 ) Serial.read();
                  Serial.print( "   Serial.avail = " ); Serial.println( Serial.available() );
                }

                if( k%(Cols) == 0 )
                   while( Serial.available() < Cols ){ continue; }
            //    while( Serial.available() < 2 ){ continue; }
                inVal = Serial.parseInt();
                DP.BA.pushQuad( inVal );                
            }

            Serial.print( "16 Done! k = ");
            Serial.println( k );
            Serial.read();
            Serial.print( "Serial.avail = " ); Serial.println( Serial.available() );
        }
        else if( numColors > 2 )// 3 or 4
        {
         //   std::cout << "\n initDataPlay(): 4 colors from dblBits";
            unsigned int numDblBits;
            while( Serial.available() < 2 );
            numDblBits = Serial.parseInt();

            if( numDblBits%4 == 0 ) dataSz = numDblBits/4;
            else dataSz = numDblBits/4 + 1;
        //    std::cout << "\n DataVec.size() = " << DataVec.size();

            DP.init( LtArr[0], GridRows, GridCols, imageData[0], dataSz, numColors );
            Serial.print( "numDblBits = " ); Serial.println( numDblBits );
            Serial.print( "dataSz = " ); Serial.println( dataSz );

            unsigned int inVal = 0;
            DP.BA.Clear();
            unsigned int k = 0;
            for( k = 0; k < numDblBits; ++k )
            {
              if( Serial.available() == 1 ) Serial.read();// flush new line
              if( k%(Cols) == 0 ) while( Serial.available() < Cols ){ continue; }
          //    while( Serial.available() < 2 );
                inVal = Serial.parseInt();
                DP.BA.push( inVal/2 > 0 );// hi bit
                DP.BA.push( inVal%2 > 0 );// lo bit
            }
            Serial.print( "4 Done! k = ");  Serial.println( k );  Serial.read();
        }
        else if( numColors == 2 )
        {
      //      std::cout << "\n initDataPlay(): 2 colors from bits";
            unsigned int numBits;
            while( Serial.available() < 2 );
            numBits = Serial.parseInt();
            if( numBits%8 == 0 ) dataSz = numBits/8;
            else dataSz = numBits/8 + 1;
            DP.init( LtArr[0], GridRows, GridCols, imageData[0], dataSz, numColors );
            Serial.print( "numBits = " ); Serial.println( numBits );
            Serial.print( "dataSz = " ); Serial.println( dataSz );

            bool inVal = 0;
            DP.BA.Clear();
            unsigned int k = 0;
            for( k = 0; k < numBits; ++k )
            {
              if( Serial.available() == 1 ) Serial.read();// flush new line
              if( k%(Cols) == 0 ) while( Serial.available() < Cols ){ continue; }
           //   while( Serial.available() < 2 );
              inVal = Serial.parseInt();
              DP.BA.push( inVal );
            }
            Serial.print( "2 Done! k = ");  Serial.println( k );  Serial.read();
        }
     //   else// numColors bad value
     //       std::cout << "\n loadImageData() bad numColors = " << numColors;
    }
    else// data is in whole bytes which may represent 2, 4 or 16 colors
    {
   //     std::cout << "\n initDataPlay(): " << numColors << " colors from Bytes";
        unsigned int numBytes;
        while( Serial.available() < 2 );
        numBytes = Serial.parseInt();
    //    DataVec.reserve( numBytes );
        unsigned int inVal = 0;
        for( unsigned int k = 0; k < numBytes; ++k )
        {
          if( Serial.available() == 1 ) Serial.read();// flush new line
          while( Serial.available() < 2 );
          inVal = Serial.parseInt();
      //    DataVec.push_back( inVal );
        }

        DP.init( LtArr[0], GridRows, GridCols, imageData[0], numBytes, numColors );
        Serial.print( "numBytes = " ); Serial.println( numBytes );
    }

    DP.setGridBounds( Row0, Col0, GridRows, GridCols );

   // std::cout << "\n initDataPlay(): BA.bitSize() = " << DP.BA.bitSize();
    Serial.print( "\n initDataPlay(): BA.bitSize() = " );
    Serial.println( DP.BA.bitSize() );

  return DP.BA.bitSize() > 0;
}
