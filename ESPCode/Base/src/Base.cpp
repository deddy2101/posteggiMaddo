// Base.cpp
#include "Base.h"

// packet type legend
//  0 = alive/handshake
//  1 = open gate
//  2 = tag id read from cancello

/*
print the list of the color for ansi
printf("\033[1;31mThis is red text\033[0m\n");
printf("\033[1;32mThis is green text\033[0m\n");
printf("\033[1;33mThis is yellow text\033[0m\n");
printf("\033[1;34mThis is blue text\033[0m\n");
printf("\033[1;35mThis is magenta text\033[0m\n");
printf("\033[1;36mThis is cyan text\033[0m\n");
printf("\033[1;37mThis is white text\033[0m\n");

*/
Base::Base() : radio(CE, CSN)
{
    // Initialize addresses array
    static const byte _addresses[][6] = {"00001", "00002"};
    addresses = _addresses;
    packetID = 0;
    delayTime = 5000;
    maxdelayTime = 10000;
    lostPackets = 0;
    // istance a data struct
}

void Base::begin()
{
    Serial.begin(baudRate); // init serial
    pinMode(BLUE_LED, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(BUTON, INPUT_PULLUP);
    delay(2000); // wait some time to not miss the first message
    Serial.printf("\033[1;32m[I] SPI pins: SCK = %d, MISO = %d, MOSI = %d, SS = %d\n\033[0m", SCK, MISO, MOSI, SS);
    while (!radio.begin())
    { // start the radio
        Serial.printf("\033[1;31m[E] Error starting the radio trying again\n\033[0m");
        delay(1000);
    }
    Serial.printf("\033[1;32m[I] Radio started\n\033[0m");
    radio.openWritingPipe(addresses[1]);    // 00002
    radio.openReadingPipe(1, addresses[0]); // 00001
    radio.setPALevel(RF24_PA_LOW);
    radio.startListening();
    this->handshake();
}

void Base::handshake()
{
    handshaking = true; // set the handshaking flag to true
    // the handshake function should be called in the setup function to acknoledfe thet the remote is online
    data.data[0] = HANDSHAKE_PACKET;
    // set to null the rest of the data
    for (int i = 1; i < 4; i++)
    {
        data.data[i] = 0;
    }
    data.packetType = PACKET_TYPE_HANDSHAKE;
    int i = 0;
    while (!this->sendData(data)) // send the data
    {
        Serial.printf("\033[1;32m[I] Trying to send packet try number %d\n\033[0m", i);
        delay(1000);
        i++;
        if (millis() - currentTime > maxdelayTime && blinking == false)
        {
            blinking = true;
            timer.attach_ms(500, blinkError);
        }
    }
    // now the radio should respond to the handshake with "Ciao"
    Serial.printf("\033[1;36m[I] Packet sent waiting for ack \n\033[0m");
}

bool Base::sendData(DataStruct data, bool incrementPacketID)
{
    digitalWrite(BLUE_LED, HIGH);
    // stop listening
    radio.stopListening();
    // increase the packetID
    if (incrementPacketID)
        packetID++;
    // keep track of the packetID
    data.packetID = packetID;

    // send the data
    bool result = radio.write(&data, sizeof(data));
    // check if the packet was sent
    if (!result)
    {
        Serial.printf("\033[1;31m[E] Error sending packet, destination not reachable\n\033[0m");
        lostPackets++;
        return false;
    }
    // start listening
    radio.startListening();
    digitalWrite(BLUE_LED, LOW);
    // return the result
    return result;
}

void Base::receiveData()
{
    radio.read(&data, sizeof(data));
    // get the packetID and update the packetID
    packetID = data.packetID;
    Serial.printf("\033[1;35m[I] PacketID: %d\n\033[0m", data.packetID);
    switch (data.packetType)
    {
    case PACKET_TYPE_HANDSHAKE:
        if (data.data[0] == HANDSHAKE_PACKET)
        {
            Serial.printf("\033[1;35m[I] Data: %s\n\033[0m", data.data);
            Serial.printf("\033[1;37m[I] alive recived \n\033[0m");
            // reset the timer for the handshake
            currentTime = millis();
            handshaking = false;
            if (blinking == true)
            {
                blinking = false;
                timer.detach();
                digitalWrite(LED_BUILTIN, LOW);
            }
        }
        break;
    case PACKET_TYPE_OPEN_GATE:
        // this should not be recived by the base print an error
        Serial.printf("\033[1;31m[E] Error recived open gate packet from the other station\n\033[0m");
        break;
    case PACKET_TYPE_TAG_ID:
        //signing = false;
        // this packet have to be forearded via serial to the computer with a serial.write
        // tag id recived print it
        Serial.printf("\033[1;32m[I] The dimension of data is: %d\n\033[0m", sizeof(data));
        Serial.printf("\033[1;32m[I] Tag id recived: \n\033[0m");
        for (int i = 0; i < 4; i++)
        {
            Serial.print(data.data[i], HEX);
            Serial.print(", ");
        }
        Serial.println();
        //respond with an open gate packet
        data.packetType = PACKET_TYPE_OPEN_GATE;
        data.data[0] = 0;
        data.data[1] = 0;
        data.data[2] = 0;
        data.data[3] = 0;
        sendData(data);

        break;
    default:
        break;
    }
}

void Base::loop()
{
    // run the handshake every x seconds
    if (millis() - currentTime > delayTime && !handshaking )
    {
        // print that we are starting the handshake in the serial monitor orange
        Serial.printf("\033[1;34m[W] Starting alive check\n\033[0m");
        handshake();
        // print the number of lost packets as a warning
        Serial.printf("\033[1;33m[W] Lost packets: %d\n\033[0m", lostPackets);
    }
    while (radio.available())
    {
        receiveData();
    }
    if(digitalRead(BUTON) == LOW){
        Serial.printf("\033[1;32m[I] Button pressed\n\033[0m");
        data.packetType = PACKET_TYPE_SIGN_TAG;
        data.data[0] = 0;
        data.data[1] = 0;
        data.data[2] = 0;
        data.data[3] = 0;
        sendData(data);
        delay(500);
        //signing = true;
    }
}

bool ledState = false;
void Base::blinkError()
{
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
}