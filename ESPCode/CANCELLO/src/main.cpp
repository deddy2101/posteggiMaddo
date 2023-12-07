#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define button 4

RF24 radio(7, 8); // CE, CSN
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
    Serial.printf("\033[1;32m[I] PacketID: %d\n\033[0m", data.packetID);
    Serial.printf("\033[1;32m[I] Data: %s\n\033[0m", data.data);
    //if the data is equal to "Ciao" then send back "Ciao"
    if (strcmp(data.data, "Ciao") == 0) {
      data.packetID += 1;
      strcpy(data.data, "Ciao");
      radio.stopListening();
      while(!radio.write(&data, sizeof(data))) {
        Serial.printf("\033[1;32m[I] Trying to send packet\n\033[0m");
      }
      Serial.printf("\033[1;32m[I] Packet send and ackRecived \n\033[0m");
      radio.startListening();
    }
  }
}



void setup() {
  pinMode(button, INPUT);
  radio.begin();
  radio.openWritingPipe(addresses[0]); // 00001
  radio.openReadingPipe(1, addresses[1]); // 00002
  radio.setPALevel(RF24_PA_MIN);
  handshake();
  currentTime = millis();
}

void loop() {
  delay(1000);
  while(radio.available()) {
    radio.read(&data, sizeof(data));
    Serial.printf("\033[1;32m[I] PacketID: %d\n\033[0m", data.packetID);
    Serial.printf("\033[1;32m[I] Data: %s\n\033[0m", data.data);
    //if data is equal to "Ciao" then reset the currentTime
  }
}


