

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define led 12

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};

struct dataStruct {
  long packetID;
  char data[32];
};
dataStruct data;

int delayTime = 1000;
long currentTime;
void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(addresses[1]); // 00002
  radio.openReadingPipe(1, addresses[0]); // 00001
  radio.setPALevel(RF24_PA_MIN);
  data.packetID = 0;
  handshake();
  currentTime = millis();
}

void loop() {
  if (millis() - currentTime >= delayTime) {
    handshake();
  }
  while(radio.available()) {
    radio.read(&data, sizeof(data));
    Serial.printf("\033[1;32m[I] PacketID: %d\n\033[0m", data.packetID);
    Serial.printf("\033[1;32m[I] Data: %s\n\033[0m", data.data);
    //if data is equal to "Ciao" then reset the currentTime
    if (strcmp(data.data, "Ciao") == 0) {
      currentTime = millis();
    }
  }
}


void handshake() {
  radio.stopListening();
  data.packetID += 1;
  strcpy(data.data, "Ciao"); 
  while(!radio.write(&data, sizeof(data))) {
            Serial.printf("\033[1;32m[I] Trying to send packet\n\033[0m");
  }
  Serial.printf("\033[1;32m[I] Packet send and ackRecived \n\033[0m");
  radio.startListening();

}
