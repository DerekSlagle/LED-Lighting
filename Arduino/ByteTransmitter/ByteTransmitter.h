#ifndef BYTETRANSMITTER_H
#define BYTETRANSMITTER_H

#include "Arduino.h"

class ByteTransmitter
{
    public:
    int pinValid = 7;// sender listens for signal ( receiver toggles ) to begin upload
    // and to verify that it read the posted bit value on pinBit (declared below)
    int validValue = HIGH;// toggled by receiver. Compared by sender to new value

    // bit value posted by sender ( pinMode = OUTPUT ) then read by receiver ( pinMode = INPUT_PULLUP )
    int pinBit = 8;// INPUT_PULLUP so LOW = 1, HIGH = 0

    //** */ sender reports next bit value has been written to pinBit
    // by toggling the value on pinToggle
    int toggleValue = HIGH;// this to toggle value from bit to bit
    // sender writes ( pinMode = OUTPUT ) toggleValue (to signal bit ready) and receiver reads ( pinMode = INPUT_PULLUP )
    int pinToggle = 9;// sender also initiates by writing LOW. receiver will get it if rcv.Monitor() is being called
    int bitCount = 8;// left at 8 between byte transmissions
    uint8_t byteValue = 0;// to send or receive 8 bits    
    bool isSender = false;// configure as 1 or the other = receiver

    // configure 1 each way
    // init functions here
    void setup( int PinValid, int pinBit, int pinToggle, bool IsSender );
    void setAsSender();// assign pinmodes, etc
    void setAsReceiver();// assign pinmodes, etc

    // for receiver
    // receiver calls to initiate and complete an upload. Must call sender.Monitor() on other device
    uint8_t getByte();// returns true if successful. Value is written to byteValue. calls sender

    // after started by getByte() or a call to callSender()
    void callSender();// writes to pinValid and toggles validValue
    uint8_t getNextByte();// as receiver does not call sender 
    uint16_t getUint16( bool doStart = false );// if 1st data is a uint16_t

    // for sender
    void sendByte();// sends byteValue
    void sendByte( uint8_t val ){ byteValue = val; sendByte(); }
    void sendNextByte();// when it is already going
    void sendNextByte( uint8_t val ){ byteValue = val; sendNextByte(); }
    void sendUint16( uint16_t val );

    // Monitor() is called by respondent
    // If called by sender only: receiver must initiate by calling getByte() or callSender()
    // If called by receiver only: sender must initiate by calling sendByte()
    bool Monitor();// returns true when a byte transmission completes so byteValue may be changed or used

    // use isCalled() to wait for call.
    // as sender: Just answers call. Then sendByte() posts the 1st bit
    // as receiver: Does not process 1st byte call getFirstByte() which starts with 1st bit read
    bool isCalled();
};

#endif// BYTETRANSMITTER