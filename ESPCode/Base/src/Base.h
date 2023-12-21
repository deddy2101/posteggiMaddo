// Base.h
#ifndef BASE_H
#define BASE_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Ticker.h>

#define BLUE_LED 39
class Base
{
private:
    RF24 radio;
    Ticker timer;
    const byte (*addresses)[6];
    long packetID;
    int delayTime;
    int maxdelayTime;
    long currentTime;
    bool handshaking;
    uint lostPackets;
    bool blinking;
    bool signing;
    #define CSN  13
    #define CE  12
    #define BUTON 17
    const unsigned long baudRate =115200;
    struct DataStruct
    {
        long packetID;
        uint8_t packetType;
        byte data[4];
    };
    DataStruct data;
    #define PACKET_TYPE_HANDSHAKE 0
    #define PACKET_TYPE_OPEN_GATE 1
    #define PACKET_TYPE_TAG_ID 2
    #define PACKET_TYPE_SIGN_TAG 3
    //define the byte for the packet handshaking is c in ascii written in binary
    #define HANDSHAKE_PACKET 0b1100011

public:
    Base(); // Constructor
    void begin(); // Initialize the radio 
    void loop(); //dunno

private:
    void handshake();   // Handshake with the remote it uses the sendData function
    bool sendData(DataStruct data, bool incrementPacketID=true); // Send data to the remote
    void receiveData(); // Receive data from the remote
    static void blinkError(); // Blink the builtin led to signal an error
};

#endif // BASE_H
