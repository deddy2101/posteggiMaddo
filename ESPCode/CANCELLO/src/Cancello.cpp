// Cancello.cpp
#include "Cancello.h"

// packet type legend
//  0 = alive/handshake
//  1 = open gate
//  2 = tag id read from cancello

Cancello::Cancello() : radio(CE, CSN)
{
    // Initialize addresses array
    static const byte _addresses[][6] = {"00001", "00002"};
    addresses = _addresses;
    currentTime = 0;
    delayTime = 5000;
    packetID = 0;
    delayTime = 5000;
}

void Cancello::begin()
{
    
    Serial.begin(baudRate); // init serial
    pinMode(GATE_RELAY_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(GATE_RELAY_PIN, LOW);
    delay(2000); // wait some time to not miss the first message
    Serial.printf("\033[1;32m[I] SPI pins: SCK = %d, MISO = %d, MOSI = %d, SS = %d\n\033[0m", SCK, MISO, MOSI, SS);
    while (!radio.begin())
    { // start the radio
        Serial.printf("\033[1;31m[E] Error starting the radio trying again\n\033[0m");
        delay(1000);
    }
    Serial.printf("\033[1;32m[I] Radio started\n\033[0m");
    radio.openWritingPipe(addresses[0]);    // 00001
    radio.openReadingPipe(1, addresses[1]); // 00002
    radio.setPALevel(RF24_PA_HIGH);
    radio.startListening();
    this->handshake();    
}

void Cancello::handshake()
{
    // we should remain here until we recive the ciao from the base
    // first we need to listen for the ciao

    while (radio.available())
    {
        receiveData();
    }
    // now we should have recived the ciao
    // now we need to send back the ciao the same packet
}

void Cancello::receiveData()
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
            // send back the data
            this->sendData(data);
            //reset the timer for alive
            currentTime = millis();
            if (blinking == true)
            {
                blinking = false;
                timer.detach();
                digitalWrite(LED_BUILTIN, HIGH);
            }
        }
        break;
    case PACKET_TYPE_OPEN_GATE:
        // this should open the gate
        Serial.printf("\033[1;32m[I] Open gate packet recived\n\033[0m");
        // pull the relay pin high for 500ms
        digitalWrite(GATE_RELAY_PIN, HIGH);
        delay(500);
        digitalWrite(GATE_RELAY_PIN, LOW);
        break;
    case PACKET_TYPE_TAG_ID:
        // this should not be recived by the base print an error
        Serial.printf("\033[1;31m[E] Error recived tag id packet from the other station\n\033[0m");
        break;
    default:
        break;
    }
}

bool Cancello::sendData(DataStruct data)
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

void Cancello::loop()
{
    while (radio.available())
    {
        receiveData();
    }
    // if the timer is expired 
    if (millis() - currentTime > delayTime && blinking == false)
    {
        blinking = true;
        timer.attach(0.5, blinkError);
    }
}

bool ledState = false;
void Cancello::blinkError(){
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
}
