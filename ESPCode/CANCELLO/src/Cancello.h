//Cancello.h
#ifndef CANCELLO_H
#define CANCELLO_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


class Cancello
{
private:
    RF24 radio;
    const byte (*addresses)[6];
    long packetID;
    int delayTime;
    long currentTime;
    const uint8_t CSN = 13;
    const uint8_t CE = 12;
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
    #define GATE_RELAY_PIN 2
    #define BUZZER_PIN 3

    public:
    Cancello(); // Constructor
    void begin(); // Initialize the radio
    void loop(); //dunno

private:
    void handshake();   // Handshake with the remote it uses the sendData function
    bool sendData(DataStruct data); // Send data to the remote
    void receiveData(); // Receive data from the remote
};

#endif // CANCELLO_H