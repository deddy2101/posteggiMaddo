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
    packetID = 0;
    delayTime = 5000;
}

void Cancello::begin()
{
    Serial.begin(baudRate); // init serial
    pinMode(GATE_RELAY_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
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
    radio.setPALevel(RF24_PA_MIN);
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
}

void R2D2()
{
    int beeps[] = {1933, 2156, 1863, 1505, 1816, 1933, 1729, 2291};
    int buzzVols[] = {144, 180, 216, 252, 252, 252, 252, 216, 180, 144};

    int i = 9;
    while (i >= 0)
    {
        tone(BUZZER_PIN, 1050, buzzVols[i] * 4);
        delayMicroseconds(20 * 64);
        noTone(BUZZER_PIN);
        delayMicroseconds(40 * 64);
        i--;
    }

    delay(35);

    i = 0;
    while (i < 8)
    {
        int v = 0;
        while (v < 250)
        { // 12.5 mS fade up time
            tone(BUZZER_PIN, beeps[i], v * 4);
            v += 10;
            delayMicroseconds(2 * 64);
        }
        delay(20);
        v = 250;
        while (v > 0)
        { // 12.5 mS fade down time
            tone(BUZZER_PIN, beeps[i], v * 4);
            v -= 10;
            delayMicroseconds(5 * 64);
        }
        noTone(BUZZER_PIN);
        delay(25);
        i++;
    }

    int f = 2466;
    while (f < 2825)
    {
        tone(BUZZER_PIN, f, 1023);
        f += 3;
        delay(1);
    }
    f = 2825;
    int v = 255;
    while (f > 2000)
    {
        tone(BUZZER_PIN, f, v * 4);
        f -= 6;
        v -= 1;
        delay(1);
    }
    noTone(BUZZER_PIN);
    delay(35);

    i = 10;
    while (i > 0)
    {
        tone(BUZZER_PIN, 1050, buzzVols[i] * 4);
        delayMicroseconds(20 * 64);
        tone(BUZZER_PIN, 1050, buzzVols[i] / 8 * 4);
        delayMicroseconds(40 * 64);
        i--;
    }
    noTone(BUZZER_PIN);
}
