#include <Arduino.h>
#include <CardReader.h>

CardReader cardReader;

void setup() {
Serial.begin(115200);
//wait for serial port to connect. Needed for native USB port only
while (!Serial) {
  delay(1);
}
  cardReader.begin();
  

}



//the new sector trailer is creater by KeyA, AccessBits, KeyB crate a variable called newSectorTrailer




void loop() {
  //the serial is returned as a pointer to an array of 5 bytes form readCard()
  
  byte* result = cardReader.readCard();

  //and print the result if the result is not null
  if(result != NULL){
    Serial.printf("\033[1;32m[I] The tag's serial number is: \n\033[0m");
    for(int i = 0; i < 5; i++){
      Serial.print(result[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  } 
  delay(1000);
  //stop the loop

}
