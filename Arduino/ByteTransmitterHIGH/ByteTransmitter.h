#ifndef BYTETRANSMITTER_H
#define BYTETRANSMITTER_H

#include "Arduino.h"

class ByteTransmitter
{
    public:
    int pinBit = 0;// sender posts the bit values here
    int pinSend = 0;// sender toggles to tell receiver next bit value is on pinBit then monitors pinConfirm
    int pinConfirm = 0;// receiver toggles to confirm receipt and ready for next (while awaiting toggle on pinSend)

    // each keeps track of the pin states
    int toggleSend = HIGH;// for change detection of toggled values
    int toggleConfirm = HIGH;

    bool isSender = true;
    uint8_t byteValue = 0;// ready
    uint8_t bitCount = 0;// ready
    uint16_t byteCount = 0;// ready
    uint16_t dataSize = 0;// NOT READY assign prior to transfer to be sent or received in bytes
    uint16_t BuffSize = 0;// storage limit
    uint8_t* pBuff = nullptr;// read and write bytes
    void setup( bool is_Sender, uint8_t* p_Buff, int PinBit, int PinSend, int PinConfirm );

    // for role swap
    void setAsSender();// assign dataSize
    void setAsReceiver();

    // only the receiver monitors. Sender posts 1st bit and awaits confirm.
    // Sender does not await a prompt on pinConfirm to post 1st bit
    // 1st 2 bytes = packet size

    // receiver check once per loop for changed pinSend value then send goes all at once
    bool Monitor();// false if no toggle. true when download complete
    void readBit();

    // sender just sends. Function returns when it is all sent
    // real
 //   void SendData( uint8_t* byteSource );
    // for testing here
    void SendData();
    void writeBit();

    ByteTransmitter(){}
    ~ByteTransmitter(){}

    protected:

    private:
};

#endif // BYTETRANSMITTER_H
