
#include <Arduino.h>
#include <Cancello.h>
#include <CardReader.h>
bool isSigning = false;
CardReader cardReader(&isSigning);
Cancello cancello(&cardReader, &isSigning);
byte *result;

void setup()
{
  cancello.begin();
  cardReader.begin();
}

void loop()
{

  cancello.loop();
  if (isSigning)
  {
    result = cardReader.resignCard();
    // is the result is _ByteRespErrAuthB1OKA try with the other key
    if (result[_AddressByteResponseSign] == _ByteRespErrAuthB1OKA)
    {
      result = cardReader.signCard();
    }
  }
  else
  {
    result = cardReader.readCard();
  }

  // and print the result if the result is not null
  if (result[_AddressByteResponseSign] == _ByteResultOK)
  {
    Serial.printf("\033[1;32m[I] A valid tag has been read: \n\033[0m");
    // we can now send the tag id to the base
    // we need to create a data struct
    cancello.data.packetType = PACKET_TYPE_TAG_ID;

    // copy the data from the result to the data struct
    for (int i = 0; i < 4; i++)
    {
      cancello.data.data[i] = result[i];
    }

    Serial.printf("\033[1;32m[I] The tag's serial number is: \033[0m");
    for (int i = 0; i < 4; i++)
    {
      Serial.print(cancello.data.data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    // send the data
    cancello.sendData(cancello.data, true);
    // beep
    tone(BUZZER_PIN, 6000, 300);
  }
  else if (result[_AddressByteResponseSign] != _ByteResultOK && result[_AddressByteResponseSign] != _ByteErrReqTag && result[_AddressByteResponseSign != _ByteResponseSignNotCorrect])
  {
    Serial.printf("\033[1;31m[E] Error reading tag: \n\033[0m");
    // beep 3 times for error
    tone(BUZZER_PIN, 6000, 300);
    delay(200);
    noTone(BUZZER_PIN);
    delay(200);
    tone(BUZZER_PIN, 6000, 300);
    delay(200);
    noTone(BUZZER_PIN);
    delay(200);
    tone(BUZZER_PIN, 6000, 300);
  }
}
