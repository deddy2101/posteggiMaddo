// CardReader.cpp
#include "CardReader.h"

CardReader::CardReader(/* args */) : nfc(SAD, RST)
{
}

void CardReader::begin()
{
    delay(2000);

    //SPI.begin(36, 33, 35, 38); // old configuration for SPI
    // wait for serial port to connect. Needed for native USB port only
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
    if (status == MI_OK)
    {
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

        // read the first 4 blocks using the new keyA and keyB
        //  Assuming that there are only 64 blocks of memory in this chip

        // Try to authenticate each block first with the A key.
        status = nfc.authenticate(MF1_AUTHENT1A, 1, newKeyA, serial);
        if (status == MI_OK)
        {

            Serial.printf("\033[1;32m[I] The data in block 0x%x is:\n\033[0m", 1);
            // Reading block i from the tag into data.
            status = nfc.readFromTag(1, data);
            if (status == MI_OK)
            {
                // If there was no error when reading; print all the hex
                // values in the data.
                for (j = 0; j < 15; j++)
                {
                    Serial.print(data[j], HEX);
                    Serial.print(", ");
                }
                Serial.println(data[15], HEX);
                // print some carriage return
                Serial.println();
                Serial.println();
            }
            else
            {
                Serial.printf("\033[1;31m[E] Access denied at block 0x%x\n\033[0m", i);
            }
        } else {
            Serial.printf("\033[1;31m[E] Access denied at block 0x%x\n\033[0m", i);
        }

        // Stop the tag and get ready for reading a new tag.
        nfc.haltTag();
    }

    // loop through the data to check if the sign is correct
    if (data[0] != 0)
    {

        bool deddy = true;
        for (int i = 0; i < 16; i++)
        {
            if (data[i] != sign[i])
            {
                deddy = false;
            }
        }
        if (deddy)
            return serial;
        else
            Serial.printf("\033[1;31m[E] The sign is not correct\n\033[0m");
            return NULL;
    }
    else
        return NULL;
}

// this function is used to sign a card new from the factory
void CardReader::signCard()
{
    status = nfc.requestTag(MF1_REQIDL, data);
    if (status == MI_OK)
    {
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

        // ok now we have selected the tag we need to authenticate it with the old keyA (or the new one in case of a new card)
        status = nfc.authenticate(MF1_AUTHENT1A, 1, oldkeyA, serial);
        if (status == MI_OK)
        {
            // success auth write the sign in the sector 0x01
            status = nfc.writeToTag(1, sign);
            if (status == MI_OK)
            {
                // check if the sign has been written
                Serial.printf("\033[1;32m[I] The sign has been written\n\033[0m");
                // now we need to write the new sector trailer
                status = nfc.authenticate(MF1_AUTHENT1A, 3, oldkeyA, serial);
                if (status == MI_OK)
                {
                    status = nfc.writeToTag(3, sectorTrailer);
                    if (status == MI_OK)
                    {
                        Serial.printf("\033[1;32m[I] The new sector trailer has been written\n\033[0m");
                    }
                    else
                    {
                        Serial.printf("\033[1;31m[E] Error writing the new sector trailer\n\033[0m");
                    }
                }
                else
                {
                    Serial.printf("\033[1;31m[E] Error authenticating the card\n\033[0m");
                }
            }
            else
            {
                Serial.printf("\033[1;31m[E] Error writing the sign\n\033[0m");
            }
        }
        else
        {
            Serial.printf("\033[1;31m[E] Error authenticating the card Wrong Key\n\033[0m");
        }

        // read the content of the card to check if everything is ok

        // check if the sign has been written
        status = nfc.authenticate(MF1_AUTHENT1A, 1, newKeyA, serial);
        if (status == MI_OK)
        {
            // success auth write the sign in the sector 0x01
            status = nfc.readFromTag(1, data);
            if (status == MI_OK)
            {
                // check if the sign has been written
                Serial.printf("\033[1;32m[I] The sign is:\n\033[0m");
                for (j = 0; j < 15; j++)
                {
                    Serial.print(data[j], HEX);
                    Serial.print(", ");
                }
                Serial.println(data[15], HEX);
            }
            else
            {
                Serial.printf("\033[1;31m[E] Error reading the sign\n\033[0m");
            }
        }
        else
        {
            Serial.printf("\033[1;31m[E] Error authenticating the card the key is not changed\n\033[0m");
        }
    }
}