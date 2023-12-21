// CardReader.cpp
#include "CardReader.h"

CardReader::CardReader(bool *issignong) : nfc(SAD, RST)
{
    // set 6 position of serial to 0b11111111
    memset(serial, 0xFF, sizeof(serial));
    isSigning = issignong;
}

void CardReader::begin()
{
    delay(2000);
    pinMode(BUZZER_PIN, OUTPUT);
    tone(BUZZER_PIN, 5000, 100);
    // SPI.begin(36, 33, 35, 38); // old configuration for SPI
    //  wait for serial port to connect. Needed for native USB port only
    Serial.printf("\033[1;32m[I] Looking for MFRC522\n\033[0m");
    nfc.begin();
    byte version = nfc.getFirmwareVersion();
    if (!version)
    {
        Serial.printf("\033[1;31m[E] Didn't find MFRC522 board\n\033[0m");
        while (1)
            ; // halt
    }
    Serial.printf("\033[1;32m[I] Found chip MFRC522 Firmware ver. 0x%x\n\033[0m", version);
    // Copia KeyA nei primi 6 byte di combinedData
    memcpy(sectorTrailer, newKeyA, 6);
    // Copia Access Bits nei successivi 4 byte di combinedData
    memcpy(sectorTrailer + 6, accessBits, 4);
    // Copia KeyB nei restanti 6 byte di combinedData
    memcpy(sectorTrailer + 10, newKeyB, 6);
    // Ora combinedData contiene KeyA + Access Bits + KeyB
    Serial.printf("\033[1;32m[I] The valid sector trailer is:\n\033[0m");
    for (int i = 0; i < 15; i++)
    {
        Serial.print(sectorTrailer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

byte *CardReader::readCard()
{
    // set array to null
    memset(data, 0, sizeof(data));
    memset(serial, 0, sizeof(serial));
    // Send a general request out into the aether. If there is a tag in
    // the area it will respond and the status will be MI_OK.
    status = nfc.requestTag(MF1_REQIDL, data);
    if (status != MI_OK)
    {
        nfc.haltTag();
        // Serial.printf("\033[1;31m[E] Error requesting tag\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteErrReqTag;
        return retrunVAl;
    }

    Serial.printf("\033[1;32m[I] Tag detected\n\033[0m");
    Serial.printf("\033[1;32m[I] The tag's type is: \n\033[0m");
    Serial.print(data[0], HEX);
    Serial.print(", ");
    Serial.println(data[1], HEX);

    // calculate the anti-collision value for the currently detected
    // tag and write the serial into the data array.
    status = nfc.antiCollision(data);
    memcpy(serial, data, 5);

    // Select the tag that we want to talk to. If we don't do this the
    // chip does not know which tag it should talk if there should be
    // any other tags in the area..
    nfc.selectTag(serial);

    // read the first 4 blocks using the new keyA and keyB
    //  Assuming that there are only 64 blocks of memory in this chip

    // Try to authenticate each block first with the A key.
    status = nfc.authenticate(MF1_AUTHENT1A, 1, newKeyA, serial);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error authenticating the block 1 using newKeyA\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteResponseErrAuthCard;
        return retrunVAl;
    }
    // Reading block i from the tag into data.
    status = nfc.readFromTag(1, data);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error reading the sign\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteRespRdsign;
        return retrunVAl;
    }
    nfc.haltTag();
    
    // check if the sign is correct
    bool deddy = true;
    for (int i = 0; i < 16; i++)
    {
        if (data[i] != sign[i])
        {
            deddy = false;
        }
    }
    if (deddy)
    {
        serial[_AddressByteResponseSign] = _ByteResultOK;
        tone(BUZZER_PIN, 5000, 100);
        return serial;
    }

    else
        Serial.printf("\033[1;31m[E] The sign is not correct\n\033[0m");
    retrunVAl[_AddressByteResponseSign] = _ByteResponseSignNotCorrect;
    // play a beep sound
    beepError();
    
    return retrunVAl;
}

// retunr a pointer to the serial number of the card in the 5th index of the array there is the result of the operation
byte *CardReader::signCard()
{
    status = nfc.requestTag(MF1_REQIDL, data);
    if (status != MI_OK)
    {
        nfc.haltTag();
        // Serial.printf("\033[1;31m[E] Error requesting tag\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteErrReqTag;
        return retrunVAl;
    }

    Serial.printf("\033[1;32m[I] Tag detected\n\033[0m");
    Serial.printf("\033[1;32m[I] The tag's type is: \n\033[0m");
    Serial.print(data[0], HEX);
    Serial.print(", ");
    Serial.println(data[1], HEX);
    // calculate the anti-collision value for the currently detected
    // tag and write the serial into the data array.
    status = nfc.antiCollision(data);
    memcpy(serial, data, 5);

    Serial.printf("\033[1;32m[I] The serial nb of the tag is:\n\033[0m");
    for (i = 0; i < 3; i++)
    {
        Serial.print(serial[i], HEX);
        Serial.print(", ");
    }
    Serial.println(serial[3], HEX);

    // Select the tag that we want to talk to. If we don't do this the
    // chip does not know which tag it should talk if there should be
    // any other tags in the area..
    nfc.selectTag(serial);

    // ok now we have selected the tag we need to authenticate it with the old keyA
    status = nfc.authenticate(MF1_AUTHENT1A, 1, oldkeyA, serial);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error authenticating the block 1 using oldKeyA\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteRespErrAuthB1OKA;
        return retrunVAl;
    }
    // sign the card if the authentication is ok (oldkey is default factory key)
    status = nfc.writeToTag(1, sign);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error writing the new sign to block 1\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteRespErrWrSignB1OKA;
        return retrunVAl;
    }
    // now authenticate block 3 with the old keyA
    status = nfc.authenticate(MF1_AUTHENT1A, 3, oldkeyA, serial);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error authenticating the block 3 using oldKeyA\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteRespErrAuthB3OKA;
        return retrunVAl;
    }
    // write the sector trailer with the new keyA and keyB
    status = nfc.writeToTag(3, sectorTrailer);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error writing the sector trailer\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteResponseErrWritingSectorTrailer;
        return retrunVAl;
    }
    // if sector trailer is written correctly authenticate the card with the new keyA

    status = nfc.authenticate(MF1_AUTHENT1A, 1, newKeyA, serial);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error authenticating the card the key is not changed\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteRespErrAuthKeyNotChanged;
        return retrunVAl;
    }
    // success auth write the sign in the sector 0x01
    status = nfc.readFromTag(1, data);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error reading the sign\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteRespRdsign;
        return NULL;
    }
    nfc.haltTag();

    // check if the sign is correct
    bool deddy = true;
    for (int i = 0; i < 16; i++)
    {
        if (data[i] != sign[i])
        {
            deddy = false;
        }
    }
    if (deddy)
    {
        serial[_AddressByteResponseSign] = _ByteResultOK;
        //set the isSigning variable to false
        *isSigning = false;
        beepSuccess();
        return serial;
    }

    else
        Serial.printf("\033[1;31m[E] The sign is not correct\n\033[0m");
    retrunVAl[_AddressByteResponseSign] = _ByteResponseSignNotCorrect;

    return retrunVAl;
}

byte *CardReader::resignCard()
{
    status = nfc.requestTag(MF1_REQIDL, data);
    if (status != MI_OK)
    {
        nfc.haltTag();
        // Serial.printf("\033[1;31m[E] Error requesting tag\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteErrReqTag;
        return retrunVAl;
    }

    Serial.printf("\033[1;32m[I] Tag detected\n\033[0m");
    Serial.printf("\033[1;32m[I] The tag's type is: \n\033[0m");
    Serial.print(data[0], HEX);
    Serial.print(", ");
    Serial.println(data[1], HEX);
    // calculate the anti-collision value for the currently detected
    // tag and write the serial into the data array.
    status = nfc.antiCollision(data);
    memcpy(serial, data, 5);

    Serial.printf("\033[1;32m[I] The serial nb of the tag is:\n\033[0m");
    for (i = 0; i < 3; i++)
    {
        Serial.print(serial[i], HEX);
        Serial.print(", ");
    }
    Serial.println(serial[3], HEX);

    // Select the tag that we want to talk to. If we don't do this the
    // chip does not know which tag it should talk if there should be
    // any other tags in the area..
    nfc.selectTag(serial);

    // ok now we have selected the tag we need to authenticate it with the old keyA
    status = nfc.authenticate(MF1_AUTHENT1B, 1, newKeyB, serial);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error authenticating the block 1 using newKeyB\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteRespErrAuthB1OKA;
        return retrunVAl;
    }
    // sign the card if the authentication is ok (oldkey is default factory key)
    status = nfc.writeToTag(1, sign);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error writing the new sign to block 1\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteRespErrWrSignB1OKA;
        return retrunVAl;
    }
    // now authenticate block 3 with the old keyA
    status = nfc.authenticate(MF1_AUTHENT1B, 3, newKeyB, serial);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error authenticating the block 3 using newKeyB\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteRespErrAuthB3OKA;
        return retrunVAl;
    }
    // write the sector trailer with the new keyA and keyB
    status = nfc.writeToTag(3, sectorTrailer);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error writing the sector trailer\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteResponseErrWritingSectorTrailer;
        return retrunVAl;
    }
    // if sector trailer is written correctly authenticate the card with the new keyA

    status = nfc.authenticate(MF1_AUTHENT1A, 1, newKeyA, serial);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error authenticating the card the key is not changed\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteRespErrAuthKeyNotChanged;
        return retrunVAl;
    }
    // success auth write the sign in the sector 0x01
    status = nfc.readFromTag(1, data);
    if (status != MI_OK)
    {
        nfc.haltTag();
        Serial.printf("\033[1;31m[E] Error reading the sign\n\033[0m");
        retrunVAl[_AddressByteResponseSign] = _ByteRespRdsign;
        return NULL;
    }
    nfc.haltTag();

    // check if the sign is correct
    bool deddy = true;
    for (int i = 0; i < 16; i++)
    {
        if (data[i] != sign[i])
        {
            deddy = false;
        }
    }
    if (deddy)
    {
        serial[_AddressByteResponseSign] = _ByteResultOK;
        //set the isSigning variable to false
        *isSigning = false;
        beepSuccess();
        return serial;
    }

    else
        Serial.printf("\033[1;31m[E] The sign is not correct\n\033[0m");
    retrunVAl[_AddressByteResponseSign] = _ByteResponseSignNotCorrect;


    return retrunVAl;
}

void CardReader::beepError()
{
    //make a double beep
    tone(BUZZER_PIN, 3000, 100);
    delay(100);
    noTone(BUZZER_PIN);
    delay(100);
    tone(BUZZER_PIN, 3000, 100);
    delay(100);

}
void CardReader::beepSuccess()
{
    //make a double beep
    tone(BUZZER_PIN, 3000, 100);
    delay(100);
    noTone(BUZZER_PIN);
    delay(100);
    tone(BUZZER_PIN, 5000, 100);
    delay(100);
}