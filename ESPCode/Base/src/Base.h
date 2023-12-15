// Base.h
#ifndef BASE_H
#define BASE_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Ticker.h>

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
    #define CSN  13
    #define CE  12
    const unsigned long baudRate =9600;
    struct DataStruct
    {
        long packetID;
        uint8_t packetType;
        char data[32];
    };
    DataStruct data;
    #define PACKET_TYPE_HANDSHAKE 0
    #define PACKET_TYPE_OPEN_GATE 1
    #define PACKET_TYPE_TAG_ID 2
    //define the string for handshaking
    #define HANDSHAKE_STRING "Ciao"

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
