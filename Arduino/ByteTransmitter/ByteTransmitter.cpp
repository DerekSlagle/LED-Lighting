#include "ByteTransmitter.h"

// init function defs here
void ByteTransmitter::setup( int PinValid, int PinBit, int PinToggle, bool IsSender )
{
    pinValid = PinValid;
    pinBit = PinBit;
    pinToggle = PinToggle;
    isSender = IsSender;
    if( IsSender )
    {
        isSender = true;
        // everybody starts off HIGH
        validValue = HIGH;
        toggleValue = HIGH;
        bitCount = 8;// value between transmissions
        byteValue = 123;// to signify default still assigned

        // 1 input pin
        pinMode( pinValid, INPUT_PULLUP );// listens for signal (toggled value) from receiver
        // 2 output pins
        pinMode( pinBit, OUTPUT );// post a bit value for receiver to read
        digitalWrite( pinBit, HIGH );// initialize bit value = 0
        pinMode( pinToggle, OUTPUT );// toggle toggleValue after writing to pinBit. receiver responds
        digitalWrite( pinToggle, HIGH );// start HIGH
    }
    else// is receiver
    {
        isSender = false;
        // everybody starts off HIGH
        validValue = HIGH;
        toggleValue = HIGH;
        bitCount = 8;// value between transmissions
        byteValue = 123;// to signify default still assigned

        // 1 output pin
        pinMode( pinValid, OUTPUT );// toggle validValue to start upload and to confirm bit read
        digitalWrite( pinValid, HIGH );// start HIGH
        // 2 input pins
        pinMode( pinBit, INPUT_PULLUP );// read each bit value
        pinMode( pinToggle, INPUT_PULLUP );// after this has been toggled by sender
    }
}

void ByteTransmitter::setAsSender()// assign pinmodes, etc
{
    isSender = true;
    bitCount = 8;// value between transmissions
    byteValue = 0;    
    // 1 input pin
    pinMode( pinValid, INPUT_PULLUP );// listens for signal (toggled value) from receiver
    // 2 output pins
    pinMode( pinBit, OUTPUT );// post a bit value for receiver to read
    pinMode( pinToggle, OUTPUT );// toggle toggleValue after writing to pinBit. receiver responds
}

void ByteTransmitter::setAsReceiver()// assign pinmodes, etc
{
    isSender = false;
    bitCount = 8;// value between transmissions
    byteValue = 0;// to signify default still assigned

    // 1 output pin
    pinMode( pinValid, OUTPUT );// toggle validValue to start upload and to confirm bit read
    // 2 input pins
    pinMode( pinBit, INPUT_PULLUP );// read each bit value
    pinMode( pinToggle, INPUT_PULLUP );// after this has been toggled by sender
}

// for receiver
// receiver calls to initiate and complete an upload. Must call sender.Monitor() on other device
uint8_t ByteTransmitter::getByte()// returns byteValue. Value is written to byteValue
{
    // trigger by toggling validValue on pinValid
    if( validValue == HIGH ) validValue = LOW;
    else validValue = HIGH;
    digitalWrite( pinValid, validValue );

    // wait for 1st bit
    while(  digitalRead( pinToggle ) == toggleValue );// sender should be monitoring
    // toggle toggleValue
    if( toggleValue == HIGH ) toggleValue = LOW;
    else toggleValue = HIGH;
    // read 1st bit then notify sender
    byteValue = ( digitalRead( pinBit ) == LOW ) ? 1 : 0;// next bit in  
    bitCount = 1;
    // let sender know the bit was read
    if( validValue == HIGH ) validValue = LOW;
    else validValue = HIGH;
    digitalWrite( pinValid, validValue );

    // get the remaining 7 bits
    Monitor();// can only return true due to start above
    // reset conditions for next
  //  validValue = HIGH;
  //  toggleValue = HIGH;
  //  digitalWrite( pinValid, HIGH );// ok because sender has also reset, so HIGH is expected

    return byteValue;// should always = true
}

// for sender
void ByteTransmitter::sendByte()// initiates and completes upload of byteValue to receiver if rcv.Monitor() is being called
{
    // trigger by sending 1st bit value
    bitCount = 1;
    int bitOut = ( ( byteValue & 1 ) == 1 ) ? LOW : HIGH;// low bit 1st
    digitalWrite( pinBit, bitOut );// 1st bit
    // then toggle toggleValue and write to pinToggle
    if( toggleValue == HIGH ) toggleValue = LOW;
    else toggleValue = HIGH;
    digitalWrite( pinToggle, toggleValue );// let receiver know 1st bit is ready

    // send the remainig 7 bits
    Monitor();// can only return true due to start above
    return;
}

void ByteTransmitter::callSender()// writes to pinValid and toggles validValue
{
    // trigger by toggling validValue on pinValid
    if( validValue == HIGH ) validValue = LOW;
    else validValue = HIGH;
    digitalWrite( pinValid, validValue );
}

// call after sender notified, if necessary.
// sender must not be calling Monitor(). sender is executing a sequence of sendByte() calls
// receiver keeps watching pinToggle
uint8_t ByteTransmitter::getNextByte()
{
    // wait for notice that 1st bit is ready
    while(  digitalRead( pinToggle ) == toggleValue );
    // toggle toggleValue
    if( toggleValue == HIGH ) toggleValue = LOW;
    else toggleValue = HIGH;    

    // read 1st bit then notify sender
    byteValue = ( digitalRead( pinBit ) == LOW ) ? 1 : 0;// next bit received
    bitCount = 1;
    // let sender know the bit was read
    if( validValue == HIGH ) validValue = LOW;
    else validValue = HIGH;
    digitalWrite( pinValid, validValue );
    Monitor();// gets the 7 more bits
    return byteValue;
}

uint16_t ByteTransmitter::getUint16( bool doStart )// if 1st data is a uint16_t
{
    if( doStart )// sender is watching validPin
    {
        // trigger by toggling validValue on pinValid
        if( validValue == HIGH ) validValue = LOW;
        else validValue = HIGH;
        digitalWrite( pinValid, validValue );
    }

    uint16_t loByte = getNextByte();
    uint16_t hiByte = getNextByte();
    return ( hiByte << 8 ) | loByte;
}

// for sender
void ByteTransmitter::sendNextByte()// when it is already going
{
  bitCount = 1;
  int bitOut = ( ( byteValue & 1 ) == 1 ) ? LOW : HIGH;// low bit 1st
  digitalWrite( pinBit, bitOut );// 1st bit
  // toggle toggleValue
  if( toggleValue == HIGH ) toggleValue = LOW;
  else toggleValue = HIGH;
  // inform receiver the bit is ready
  digitalWrite( pinToggle, toggleValue );
  Monitor();// for the remaining 7 bits
}

void ByteTransmitter::sendUint16( uint16_t val )
{
    byteValue = val%256;// low byte
    sendNextByte();
    byteValue = val/256;// high byte
    sendNextByte();
}

bool ByteTransmitter::isCalled()
{
    if( isSender )
    {
        // watching pinValid for call by receiver to begin upload
        if( validValue == digitalRead( pinValid ) )// not yet
            return false;
        // the call has arrived
        // toggle for next, which are validation for each bit sent
        if( validValue == HIGH ) validValue = LOW;
        else validValue = HIGH;
        return true;
    }
    else// is receiver
    {
        // watching pinToggle for call by sender that 1st bit is ready
        if( toggleValue == digitalRead( pinToggle ) )// not yet
            return false;
        // the call has arrived. let getByte(), etc toggle the value
        // toggle for next, which are validation for each bit sent
     //   if( toggleValue == HIGH ) toggleValue = LOW;
     //   else toggleValue = HIGH;
        return true;
    }
    return false;// jic
}

// Monitor() is called by respondent
// If called by sender only: receiver must initiate by calling getByte()
// If called by receiver only: sender must initiate by calling sendByte()
bool ByteTransmitter::Monitor()// returns true when a byte transmission completes so byteValue may be changed or used
{
    if( isSender )
    {
        if( bitCount == 8 )// waiting for prompt from receiver
        {
            int validNow = digitalRead( pinValid );
            if( validNow != validValue )
            {
                validValue = validNow;// update for next time
                // start the upload
                bitCount = 1;
                int bitOut = ( ( byteValue & 1 ) == 1 ) ? LOW : HIGH;// low bit 1st
                digitalWrite( pinBit, bitOut );// 1st bit
                // toggle toggleValue
                if( toggleValue == HIGH ) toggleValue = LOW;
                else toggleValue = HIGH;
                // inform receiver the bit is ready
                digitalWrite( pinToggle, toggleValue );                
            }
            else// no call to post 1st bit yet
                return false;
        }

        // follow through on 1st bit sent by above or by sendByte()
        while( bitCount < 8 )// send the remaining 7 bits
        {
            // wait for verification that receiver has read the posted bit before writing the next
            int validNow = digitalRead( pinValid );
            if( validNow != validValue )
            {
                validValue = validNow;// update for next time
            
                // consume byteValue
                byteValue = byteValue >> 1;
                bool BitVal = ( byteValue & 1 ) == 1;
                // preserve byteValue
                //  bool BitVal = ( ( byte2send >> bitsSent ) & 1 ) == 1;

                digitalWrite( pinBit, BitVal ? LOW : HIGH );// 2nd bit
                // toggle toggleValue
                if( toggleValue == HIGH ) toggleValue = LOW;
                else toggleValue = HIGH;
                digitalWrite( pinToggle, toggleValue );// sb HIGH 
                ++bitCount;
                if( bitCount == 8 )
                {
                    // get last bit validation
                    while( digitalRead( pinValid ) == validValue );                    
                 //   validValue = !validValue;// not a bool
                 // update for next time
                    if( validValue == HIGH ) validValue = LOW;
                    else validValue = HIGH;
                    // reset conditions for next
                 //   validValue = HIGH;
                 //   toggleValue = HIGH;
                 //   digitalWrite( pinToggle, HIGH );// can not do this immediately after

                    return true;// Done! byteValue is ready                   
                }
             // stay in the while loop. Do not break;// not done yet
            }

          //  break;// no verification yet
        }// end while bitCount < 8      
    }
    else// is receiver
    {
        if( bitCount == 8 )// waiting for prompt from sender
        {
            // watch for change in state of pinToggle
            int inRcvNow = digitalRead( pinToggle );
            if( inRcvNow != toggleValue )// It is ON! All 8 bits will be sent
            {
                // toggle
                toggleValue = inRcvNow;
                // read 1st bit then notify sender
                byteValue = ( digitalRead( pinBit ) == LOW ) ? 1 : 0;// next bit received
                bitCount = 1;
                // let sender know the bit was read
                if( validValue == HIGH ) validValue = LOW;
                else validValue = HIGH;
                digitalWrite( pinValid, validValue );
            }
            else// maybe next call
                return false;// no byte uploaded
        }

        // follow through on 1st bit read by above or by getByte()
        while( bitCount < 8 )
        {
            // watch for change in state of pinToggle
            int inRcvNow = digitalRead( pinToggle );
            if( inRcvNow != toggleValue )
            {
                // toggle
                toggleValue = inRcvNow;
                
                uint8_t bitIn = ( digitalRead( pinBit ) == LOW ) ? 1 : 0;// next bit in
             //   byteValue += bitIn << bitCount;
                byteValue = byteValue | ( bitIn << bitCount );// cheaper than above?
                ++bitCount;
                // let sender know the bit was read
                if( validValue == HIGH ) validValue = LOW;
                else validValue = HIGH;
                digitalWrite( pinValid, validValue );
                
                // report byteIn
                if( bitCount == 8 )
                {                    
                    return true;// done
                }  
            }
        }// end while bitCount < 8

        return false;// this should never be reached
    }

    return false;// this should never be reached
}