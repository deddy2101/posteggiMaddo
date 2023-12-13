

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define led 12

RF24 radio(12, 13); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};

struct dataStruct
{
  long packetID;
  char data[32];
};
dataStruct data;

int delayTime = 5000;
long currentTime;

void handshake()
{
  radio.stopListening();
  data.packetID += 1;
  strcpy(data.data, "Ciao");
  int i = 0;
  while (!radio.write(&data, sizeof(data)))
  {
    Serial.printf("\033[1;32m[I] Trying to send packet try number %d\n\033[0m", i);
    delay(1000);
    i++;
  }
    //now the radio should respond to the handshake with "Ciao"
  radio.startListening();
  while (radio.available())
  {
    radio.read(&data, sizeof(data));
    Serial.printf("\033[1;32m[I] PacketID: %d\n\033[0m", data.packetID);
    Serial.printf("\033[1;32m[I] Data: %s\n\033[0m", data.data);
    //if the data is equal to "Ciao" then continue
    if (strcmp(data.data, "Ciao") == 0)
    {
      currentTime = millis();
      break;
    }
  }
  delay(1000);
  Serial.printf("\033[1;32m[I] Packet send and ackRecived \n\033[0m");
}

void setup()
{
  Serial.begin(9600);
  //start the radio and check if everything is ok
  //print the used SPI pins
  delay(2000);
    Serial.printf("\033[1;32m[I] SPI pins: SCK = %d, MISO = %d, MOSI = %d, SS = %d\n\033[0m", SCK, MISO, MOSI, SS);\

  while (!radio.begin()) {
    Serial.printf("\033[1;31m[E] Error starting the radio trying again\n\033[0m");
    delay(1000);
  }
  Serial.printf("\033[1;32m[I] Radio started\n\033[0m");
  
  radio.openWritingPipe(addresses[1]);    // 00002
  radio.openReadingPipe(1, addresses[0]); // 00001
  radio.setPALevel(RF24_PA_MIN);
  data.packetID = 0;
  handshake();
  currentTime = millis();
}

void loop()
{
  //run the handshake every 5 seconds
  if (millis() - currentTime > delayTime)
  {
    //print that we are starting the handshake in the serial monitor orange
    Serial.printf("\033[1;33m[W] Starting handshake\n\033[0m");
    handshake();
    currentTime = millis();
  }
  while (radio.available())
  {
    radio.read(&data, sizeof(data));
    Serial.printf("\033[1;32m[I] PacketID: %d\n\033[0m", data.packetID);
    Serial.printf("\033[1;32m[I] Data: %s\n\033[0m", data.data);
    // if data is equal to "Ciao" then reset the currentTime
    if (strcmp(data.data, "Ciao") == 0)
    {
      currentTime = millis();
    }
  }
}
