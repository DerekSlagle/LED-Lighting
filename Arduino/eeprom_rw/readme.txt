This is a simple test to write int values to program memory
and then read them back.

Either:
Write numInt given int values to program memory via
EEPROM.put( iter, val );
or
read numInt from address 0
EEPROM.get( 0, numInts );
then read and display the numInt values:

for( n = 0; n < numInts; ++n )
{
    iter += sizeof( int );
    EEPROM.get( iter, val );
    Serial.print( val );
    Serial.print( " " );
}

User enter: w 5  1 2 3 4 5 and the values are written.
User enter: r and the values are read and displayed.

Run again. Choose 'r' first. The values 1 2 3 4 5 will be read.