#include "MemoryPool.h"
#include "MemoryPool.cpp"

const unsigned int storeSz = 100;
int Store[ storeSz ];
const unsigned int numUsers = 4;
int** ppBlock[numUsers];
unsigned int* pBlockSz[numUsers];
MemoryPool<int> intPool( Store, storeSz, ppBlock, pBlockSz, numUsers );
// pool users
//PoolArray<int> PA1( &intPool, 5 );// array of 5
PoolArray<int> PA[ numUsers ];// array of numUsers arrays
// loop iter
//int k = 0;
const int intSz = sizeof(int);

void PoolReport();

void setup()
{
    Serial.begin(9600);
  //  while (!Serial) {
  //    ; // wait for serial port to connect. Needed for native USB port only
  //  }

    Serial.println("Setup");
    // Bind the arrays to the pool
    PA[0].Bind( &intPool, 15 );
    PA[1].Bind( &intPool, 10 );
    PA[2].Bind( &intPool, 5 );
    PA[3].Bind( &intPool, 12 );

    for( int n = 0; n < numUsers; ++n )
    {
        if( PA[n].pBase )// Alloc was good
        {
          Serial.println( "Good Alloc" );
            for( int j = 0; j < PA[n].Capacity; ++j )  PA[n][j] = (n+1)*(1+j);
        }
        else Serial.println( "Bad Alloc" );
    }
    
    Serial.print("Initial ");
    PoolReport();
}

void loop() 
{

  while (Serial.available() > 0) 
  {
      Serial.println( " " );
      int I = Serial.parseInt();// index to PA = 0,1,2
  //    I %= numUsers;
      Serial.print( "I = " );
      Serial.println( I );
      if( I < 0 )// Defrag
    {
      Serial.read();// burn the newline
      Serial.println( "** Defrag **" );
      intPool.DeFrag();
      PoolReport();
      break;
    }
    I %= numUsers;// keep in valid range
    // look for the next valid integer in the incoming serial stream:
    int num2follow = Serial.parseInt();
  //  if( num2follow < 0 )// Defrag
  //  {
  //    Serial.read();// burn the newline
  //    Serial.println( "** Defrag **" );
  //    intPool.DeFrag();
  //    PoolReport();
   //   break;
  //  }

    Serial.print( "num2follow = " );
    Serial.println( num2follow );// >= 1
  //  int I = Serial.parseInt();// index to PA = 0,1,2
  //  Serial.print( "I = " );
  //  Serial.println( I );
    int val = 0;
    //--num2follow;
    if( num2follow <= 0 )// free the block
    {
      // call Free() not as below
        PA[I].pBase = nullptr;
        PA[I].Capacity = 0;
    }
    else if( num2follow <= PA[I].Capacity )// reduce Capacity and copy values
    {
      PA[I].Capacity = num2follow;
   //   for( int n = 0; n < num2follow; ++n ) PA[I][n] = Serial.parseInt();
      for( int j = 0; j < PA[I].Capacity; ++j )  PA[I][j] = (I+1)*(1+j);
    }
    else// need more space
    {
        if( PA[I].pPool->Alloc( PA[I].poolIdx, num2follow ) )
        {
           Serial.println("Alloc good");
       //    for( int n = 0; n < num2follow; ++n ) PA[I][n] = Serial.parseInt();
           for( int j = 0; j < PA[I].Capacity; ++j )  PA[I][j] = (I+1)*(1+j);
        }
        else
        {
           Serial.println("Alloc Bad");// must parse input anyway
           for( int n = 0; n < num2follow; ++n ) Serial.parseInt();
        }
    }

    if (Serial.read() == '\n') // data end
    {
        Serial.print( PA[I].Capacity );
        Serial.print( " Elements:" );
        // echo
        for( int n = 0; n < PA[I].Capacity; ++n )
        {
          Serial.print( " " );
          Serial.print( PA[I][n] );
        }

        PoolReport();
    }
  }

 //   delay(10);
}

void PoolReport()
{
  Serial.print("\nPool report pBase = ");
  Serial.println( (int)intPool.pBase );

  int numHolding = 0;// # of held blocks
    unsigned int szHeld = 0;// total elements held
    for( unsigned int n = 0; n < numUsers; ++n )
    {
        if( !( ppBlock[n] && pBlockSz[n] ) ) continue;

        if( *ppBlock[n] )
        {
            ++numHolding;
            szHeld += *pBlockSz[n];
        }
    }

   // os << '\n' << numHolding << " holding " << szHeld << " of " << poolSz << ' ' << typeName;
    Serial.print( numHolding );
    Serial.print( " holding " );
    Serial.print( szHeld );
    Serial.print( " of " );
    Serial.print( storeSz );
    Serial.println(" int" );
    if( numHolding == 0 ) return;

    unsigned int nextIdx = 0;// iterate through pool
    bool isAnotherBlock = intPool.getNextBlockIdx( nullptr, nextIdx );
    if( isAnotherBlock )    
        Serial.println("Held Blocks" );    

    while( isAnotherBlock )
    {
   //     os << "\n n: " << (unsigned int)nextIdx << '\t';
   //     os << *pBlockSz[ nextIdx ] << ' ' << typeName << " from " << (int)( *ppBlock[ nextIdx ] - pBase );
        Serial.print( "n: " );
        Serial.print( nextIdx );
        Serial.print( "   " );
        Serial.print( *pBlockSz[ nextIdx ] );
        Serial.print( " int from " );
        Serial.println(  (int)( *ppBlock[ nextIdx ] - intPool.pBase ) );

        isAnotherBlock = intPool.getNextBlockIdx( *ppBlock[ nextIdx ], nextIdx );
     //   if( --DPholding == 0 ) return;
    }

    if( szHeld == 0 )
    {
  //      os << "\n Entire pool is free";
        Serial.println( "Entire pool is free" );
    }
    else if( szHeld < storeSz )// there is at least 1 free block
    {
   //     os << "\n Free Blocks";
        Serial.println( "Free Blocks" );
        isAnotherBlock =  intPool.getNextBlockIdx( nullptr, nextIdx );
        if( isAnotherBlock && *ppBlock[ nextIdx ] > intPool.pBase )// gap in front
        {
      //      os << '\n' << (int)( *ppBlock[ nextIdx ] - pBase ) << ' ' << typeName << " from 0";
            Serial.print( (int)( *ppBlock[ nextIdx ] - intPool.pBase ) );
            Serial.println( " int from 0" );
        }

        while( isAnotherBlock )
        {

            const int* pBlockLow = *ppBlock[ nextIdx ] + *pBlockSz[ nextIdx ];
            int gapSz = storeSz - (int)( pBlockLow - intPool.pBase );

            isAnotherBlock =  intPool.getNextBlockIdx( *ppBlock[ nextIdx ], nextIdx );
            if( isAnotherBlock )            
                gapSz = (int)( *ppBlock[ nextIdx ] - pBlockLow );            

            if( gapSz > 0 )// free block between
            {
          //      os << '\n' << gapSz << ' ' << typeName << " from " << (int)( pBlockLow - pBase );
                Serial.print( gapSz );
                Serial.print( " int from " );
                Serial.println( (int)( pBlockLow - intPool.pBase ) );
            }
        }
    }
}