// Cancello.h
#ifndef CANCELLO_H
#define CANCELLO_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Ticker.h>
#include <CardReader.h>

// define pins for the radio
#define CE 12
#define CSN 13
// define packet types
#define PACKET_TYPE_HANDSHAKE 0
#define PACKET_TYPE_OPEN_GATE 1
#define PACKET_TYPE_TAG_ID 2
#define PACKET_TYPE_SIGN_TAG 3

// define the byte for the packet handshaking is c in ascii written in binary
#define HANDSHAKE_PACKET 0b1100011
#define GATE_RELAY_PIN 2

class Cancello
{
private:
    // The radio object
    RF24 radio;
    // The timer object
    Ticker timer;
    bool* isSigning;
    // Section for the radio
    const byte (*addresses)[6];
    long packetID;
    int delayTime;
    long currentTime;
    bool blinking;
    
    const unsigned long baudRate = 115200;
    struct DataStruct
    {
        long packetID;
        uint8_t packetType;
        byte data[4];
    };
    //keep a pointer to the CardReader object
    CardReader *cardReader;

public:

    Cancello(CardReader *reader, bool* issigning);                     // Constructor takes an istance of the CardReader class
    void begin();                   // Initialize the radio
    void loop();                    // dunno
    bool sendData(DataStruct data, bool deliverSecure= false); // Send data to the remote
    DataStruct data;                // The data struct
    bool isSigningCard();           // Return true if the card is being signed

private:
    void handshake();         // Handshake with the remote it uses the sendData function
    void receiveData();       // Receive data from the remote
    static void blinkError(); // Blink the builtin led to signal an error
    void beepOpenGate();      // Beep the buzzer to signal the gate is opening  
};

#endif // CANCELLO_H