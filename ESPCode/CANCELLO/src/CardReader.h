// CardReader.h
#ifndef CARDREADER_H
#define CARDREADER_H

#include <MFRC522.h>
#include <Ticker.h>

#define SAD 38
#define RST 34
#define _AddressByteResponseSign 4
#define _ByteResultOK 0b00000000
#define _ByteResponseSignNotCorrect 0b00000001
#define _ByteResponseErrWritingSectorTrailer 0b00000010
#define _ByteResponseErrAuthCard 0b00000011
#define _ByteRespErrWrSignB1OKA 0b00000100
#define _ByteRespErrAuthB1OKA 0b00000101
#define _ByteRespErrAuthB3OKA 0b00000110
#define _ByteErrReqTag 0b00000110
#define _ByteRespErrAuthKeyNotChanged 0b00000111
#define _ByteRespRdsign 0b00001000
#define _ByteRespErrAuthB1OKB 0b00001001
#define _ByteRespErrAuthB3OKB 0b00001010

#define BUZZER_PIN 16
class CardReader
{
private:
    /* data */

    Ticker timerBuzz;
    
    MFRC522 nfc;
    byte oldkeyA[6] = {
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
    };
    byte oldkeyB[6] = {
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
    };
    byte newKeyA[6] = {0x4E, 0x2D, 0x7F, 0xD3, 0x8A, 0x1B};
    byte newKeyB[6] = {0x5F, 0xA6, 0x0C, 0x91, 0xE8, 0x3D};
    byte accessBits[4] = {0x09, 0x67, 0x8f, 0x69};
    byte sign[16] = {0x64, 0x65, 0x64, 0x64, 0x79, 0x65, 0x62, 0x65, 0x6C, 0x6C, 0x6F, 0x31, 0x32, 0x32, 0x33, 0x0}; // deddyebello1223

    // Variabile da 16 byte per contenere KeyA + Access Bits + KeyB
    byte sectorTrailer[16];
    byte status;
    byte data[MAX_LEN];
    byte serial[5];
    byte retrunVAl[5];
    int i, j, pos;
    bool* isSigning;
    void beepError();
    void beepSuccess(); 

public:
    CardReader(bool *isSigning);
    void begin();
    // readcard returns an array of 5 bytes
    byte *readCard();
    byte *signCard();
    byte *resignCard();
};

#endif // !CARDREADER_H