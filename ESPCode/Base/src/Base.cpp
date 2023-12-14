// Base.cpp
#include "Base.h"

//packet type legend
// 0 = alive/handshake
// 1 = open gate
// 2 = tag id read from cancello



Base::Base() : radio(CE, CSN)
{
    // Initialize addresses array
    static const byte _addresses[][6] = {"00001", "00002"};
    addresses = _addresses;
    packetID = 0;
    delayTime = 5000;
    // istance a data struct
}

void Base::begin()
{
    Serial.begin(baudRate); // init serial
    delay(2000);            // wait some time to not miss the first message
    Serial.printf("\033[1;32m[I] SPI pins: SCK = %d, MISO = %d, MOSI = %d, SS = %d\n\033[0m", SCK, MISO, MOSI, SS);
    while (!radio.begin())
    { // start the radio
        Serial.printf("\033[1;31m[E] Error starting the radio trying again\n\033[0m");
        delay(1000);
    }
    Serial.printf("\033[1;32m[I] Radio started\n\033[0m");
    radio.openWritingPipe(addresses[1]);    // 00002
    radio.openReadingPipe(1, addresses[0]); // 00001
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
    this->handshake();
}

void Base::handshake()
{
    // the handshake function should be called in the setup function to acknoledfe thet the remote is online
    strcpy(data.data, "Ciao"); // copy the string "Ciao" in the data.data array
    data.packetType = PACKET_TYPE_HANDSHAKE;
    int i = 0;
    while (!this->sendData(data)) // send the data
    {
        Serial.printf("\033[1;32m[I] Trying to send packet try number %d\n\033[0m", i);
        delay(1000);
        i++;
    }
    // now the radio should respond to the handshake with "Ciao"
    Serial.printf("\033[1;32m[I] Packet send waiting for ack \n\033[0m");
    while (radio.available())
    {
        receiveData();
    }
    // ack recived
    Serial.printf("\033[1;32m[I] ackRecived \n\033[0m");
}

bool Base::sendData(DataStruct data)
{
    // stop listening
    radio.stopListening();
    // keep track of the packetID
    data.packetID = packetID;
    // increase the packetID
    packetID++;
    // send the data
    bool result = radio.write(&data, sizeof(data));
    // start listening
    radio.startListening();
    // return the result
    return result;
}

void Base::receiveData()
{
    radio.read(&data, sizeof(data));
    // get the packetID and update the packetID
    packetID = data.packetID;
    Serial.printf("\033[1;32m[I] PacketID: %d\n\033[0m", data.packetID);
    Serial.printf("\033[1;32m[I] Data: %s\n\033[0m", data.data);
    switch (data.packetType)
    {
    case PACKET_TYPE_HANDSHAKE:
        if (strcmp(data.data, HANDSHAKE_STRING) == 0)
        {
            Serial.printf("\033[1;32m[I] alive recived \n\033[0m");
            // reset the timer for the handshake
            currentTime = millis();
        }
        break;
    case PACKET_TYPE_OPEN_GATE:
        //this should not be recived by the base print an error
        Serial.printf("\033[1;31m[E] Error recived open gate packet from the other station\n\033[0m");
        break;
    case PACKET_TYPE_TAG_ID:
        //this packet have to be forearded via serial to the computer with a serial.write
        Serial.printf("\033[1;32m[I] Tag ID recived: %s\n\033[0m", data.data);
        Serial.write(data.data); 
        break;
    default:
        break;
    }
}

void Base::loop()
{
    // run the handshake every x seconds
    if (millis() - currentTime > delayTime)
    {
        // print that we are starting the handshake in the serial monitor orange
        Serial.printf("\033[1;33m[W] Starting handshake\n\033[0m");
        handshake();
    }
    while (radio.available())
    {
        receiveData();
    }
    //if we recive a serial message from the pc
    if (Serial.available())
    {
        //the message has the same structure of the data struct
        //so we can just copy the message in the data struct
        Serial.readBytes(data.data, sizeof(data.data));
        //check if the message is a OPEN_GATE message
        if (strcmp(data.data, "OPEN_GATE") == 0)
        {
            //if it is send the packet to the remote
            data.packetType = PACKET_TYPE_OPEN_GATE;
            this->sendData(data);
        }
    }
}