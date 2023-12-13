#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Arduino.h>
#define button 4

RF24 radio(12, 13); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};

struct dataStruct {
  long packetID;
  char data[32];
};
dataStruct data;
int delayTime = 1000;
long currentTime;


void handshake() {
  radio.startListening();
  while(radio.available()) {
    radio.read(&data, sizeof(data));
    Serial.printf("\033[1;33m[W] PacketID: %d\n\033[0m", data.packetID);
    Serial.printf("\033[1;33m[W] Data: %s\n\033[0m", data.data);
    //if the data is equal to "Ciao" then send back "Ciao"
    if (strcmp(data.data, "Ciao") == 0) {
      strcpy(data.data, "Ciao");
      radio.stopListening();
      int i = 0;
      while(!radio.write(&data, sizeof(data))) {
        Serial.printf("\033[1;33m[W] Trying to send packet try number %d\n\033[0m", i);
        i++;
        delay(500);
      }
      Serial.printf("\033[1;33m[W] Packet send and ackRecived \n\033[0m");
      radio.startListening();
    }
  }
}



void setup() {
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
  radio.begin();
  radio.openWritingPipe(addresses[0]); // 00001
  radio.openReadingPipe(1, addresses[1]); // 00002
  radio.setPALevel(RF24_PA_MIN);
  handshake();
  currentTime = millis();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  delay(1000);
  while(radio.available()) {
    radio.read(&data, sizeof(data));
    if (strcmp(data.data, "Ciao") == 0) { //the packet is a handshake
      strcpy(data.data, "Ciao");
      radio.stopListening();
      int i = 0;
      while(!radio.write(&data, sizeof(data))) {
        Serial.printf("\033[1;33m[W] Trying to send packet try number %d\n\033[0m", i);
        i++;
        delay(500);
      }
      Serial.printf("\033[1;33m[W] Packet send and ackRecived \n\033[0m");
      radio.startListening();
    }
    Serial.printf("\033[1;33m[W] PacketID: %d\n\033[0m", data.packetID);
    Serial.printf("\033[1;33m[W] Data: %s\n\033[0m", data.data);
    //if data is equal to "Ciao" then reset the currentTime
  }
}


