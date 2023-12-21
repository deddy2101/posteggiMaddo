// Cancello.cpp
#include "Cancello.h"

// packet type legend
//  0 = alive/handshake
//  1 = open gate
//  2 = tag id read from cancello

Cancello::Cancello(CardReader *reader, bool *issigning) : radio(CE, CSN)
{
    isSigning = issigning;
    // Initialize addresses array
    static const byte _addresses[][6] = {"00001", "00002"};
    addresses = _addresses;
    currentTime = 0;
    delayTime = 5000;
    packetID = 0;
    delayTime = 5000;
    bool signing = false;
    bool blinking = false;
    // save the pointer to the CardReader object
    cardReader = reader;
}

void Cancello::begin()
{

    Serial.begin(baudRate); // init serial
    pinMode(GATE_RELAY_PIN, OUTPUT);
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
    radio.setPALevel(RF24_PA_LOW);
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
    byte *result;
    radio.read(&data, sizeof(data));
    // get the packetID and update the packetID
    packetID = data.packetID;
    //  Serial.printf("\033[1;32m[I] PacketID: %d\n\033[0m", data.packetID);
    // Serial.printf("\033[1;32m[I] Data: %s\n\033[0m", data.data);

    switch (data.packetType)
    {
    case PACKET_TYPE_HANDSHAKE:
        if (data.data[0] == HANDSHAKE_PACKET)
        {
            // Serial.printf("\033[1;32m[I] alive recived \n\n\n\033[0m");
            // send back the data
            this->sendData(data);
            // reset the timer for alive
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
        beepOpenGate();
        // pull the relay pin high for 500ms
        digitalWrite(GATE_RELAY_PIN, HIGH);
        delay(500);
        digitalWrite(GATE_RELAY_PIN, LOW);
        break;
    case PACKET_TYPE_TAG_ID:
        // this should not be recived by the base print an error
        Serial.printf("\033[1;31m[E] Error recived tag id packet from the other station\n\033[0m");
        break;
    case PACKET_TYPE_SIGN_TAG:
        // this packet should enable sign tag mode, after a tag is signed we return the tag id to the base
        Serial.printf("\033[1;32m[I] Sign tag packet recived\n\033[0m");
        // enable sign tag mode and send back the tag id
        // do the sign while the result is null
        tone(BUZZER_PIN, 800, 200);
        // set the isSigning variable to true
        *isSigning = true;

        break;

    default:
        Serial.printf("\033[1;31m[E] Error recived unknown packet type\n\033[0m");
        break;
    }
}

// send data to the remote and it automatically updates the packetID
bool Cancello::sendData(DataStruct data, bool deliverSecure)
{
    // stop listening
    radio.stopListening();
    // keep track of the packetID
    data.packetID = packetID;
    // increase the packetID
    packetID++;
    // send the data
    bool result = false;
    if (deliverSecure)
    {
        while (!result)
        {
            result = radio.write(&data, sizeof(data));
            if (!result)
            {
                Serial.printf("\033[1;31m[E] Error sending data trying again\n\033[0m");
                delay(1000);
            }
        }
    } else {
        result = radio.write(&data, sizeof(data));
    }

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
void Cancello::blinkError()
{
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
}

bool Cancello::isSigningCard()
{
    return isSigning;
}

void Cancello::beepOpenGate()
{
    //double beep opening
    tone(BUZZER_PIN, 7000, 400);
    

}