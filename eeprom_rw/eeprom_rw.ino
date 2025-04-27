#include <EEPROM.h>

char mode = 'N';// read or write mode
int iter = 0;

const int intSz = sizeof( int );
const int ptrSz = sizeof( int* );

void setup()
{
  Serial.begin(9600);
  //while (!Serial) {;}; // wait for serial port to connect. Needed for native USB port only
  Serial.println( "intSz = " );
  Serial.print( intSz );
  Serial.print( "  ptrSz = " );
  Serial.println( ptrSz );

  Serial.print( "Enter mode (r/w)\n" );
}

void loop() 
{
  if(Serial.available() > 0) 
  {
    // 1st up is char mode
    mode = Serial.read();
  //  Serial.println( mode );

    if( mode == 'r' || mode == 'R')// read
    {
   //   Serial.println( " " );
      iter = 0;
      int numInts = EEPROM.get( iter, numInts );// read from eeprom 0
      Serial.print( numInts );
      Serial.print( " values read: " );

      for( int n = 0; n < numInts; ++n )
      {
        int val = 0;// read from eeprom iter
        iter += intSz;
        EEPROM.get( iter, val );
        Serial.print( val );
        Serial.print( " " );
      }
      Serial.println(" ");
    }
    else if( mode == 'w' || mode == 'W')// write
    {
      Serial.print( "Values written: " );
      int numInts =  Serial.parseInt();
      iter = 0;

      // write to eeprom 0
      EEPROM.put( iter, numInts );

      for( int n = 0; n < numInts; ++n )
      {
        int val = Serial.parseInt();
        // write to eeprom iter
        iter += intSz;
        EEPROM.put( iter, val );
        // echo
        Serial.print( val );
        Serial.print( " " );        
      }
      Serial.println(" ");
    }

    Serial.read();// burn the newline
  //  break;
  }
}
