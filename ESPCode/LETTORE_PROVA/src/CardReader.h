//CardReader.h
#ifndef CARDREADER_H
#define CARDREADER_H

#include <MFRC522.h>

#define SAD 38
#define RST 34

class CardReader
{
private:
    /* data */
    MFRC522 nfc;
    byte oldkeyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, };
    byte oldkeyB[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, };
    byte newKeyA[6] = {0x4E, 0x2D, 0x7F, 0xD3, 0x8A, 0x1B};
    byte newKeyB[6] = {0x5F, 0xA6, 0x0C, 0x91, 0xE8, 0x3D};
    byte accessBits[4] = {0x09, 0x67, 0x8f, 0x69};
    byte sign[16] = {0x64, 0x65, 0x64, 0x64, 0x79, 0x65, 0x62, 0x65, 0x6C, 0x6C, 0x6F, 0x31, 0x32, 0x32, 0x33, 0x0}; //deddyebello1223
    // Variabile da 16 byte per contenere KeyA + Access Bits + KeyB
    byte sectorTrailer[16];  
    byte status;
    byte data[MAX_LEN];
    byte serial[5];
    int i, j, pos;
    
public:
    CardReader(/* args */);
    void begin();
    //readcard returns an array of 5 bytes
    byte* readCard();
    void signCard();
};

#endif // !CARDREADER_H