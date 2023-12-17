// Cancello.h
#ifndef CANCELLO_H
#define CANCELLO_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Ticker.h>


//define pins for the radio
#define CE 12
#define CSN 13
//define packet types
#define PACKET_TYPE_HANDSHAKE 0
#define PACKET_TYPE_OPEN_GATE 1
#define PACKET_TYPE_TAG_ID 2
// define the string for handshaking
#define HANDSHAKE_STRING "Ciao"
#define GATE_RELAY_PIN 2
#define BUZZER_PIN 3

class Cancello
{
private:
    // The radio object
    RF24 radio;
    // The timer object
    Ticker timer;
    

    //Section for the radio
    const byte (*addresses)[6];
    long packetID;
    int delayTime;
    long currentTime;
    bool blinking;
    const unsigned long baudRate = 9600;
    struct DataStruct
    {
        long packetID;
        uint8_t packetType;
        char data[32];
    };

    


public:
    Cancello();   // Constructor
    void begin(); // Initialize the radio
    void loop();  // dunno
    bool sendData(DataStruct data); // Send data to the remote
    DataStruct data;                // The data struct


private:
    void handshake();               // Handshake with the remote it uses the sendData function
    void receiveData();             // Receive data from the remote
    static void blinkError();       // Blink the builtin led to signal an error
};

#endif // CANCELLO_H