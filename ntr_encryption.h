#pragma once
// Taken from Decrypt9WIP by Kitlith (3 Sep 2016)

#include <stdint.h>

typedef struct _IKEY1 {
    uint32_t iii;
    uint32_t jjj;
    uint32_t kkkkk;
    uint32_t llll;
    uint32_t mmm;
    uint32_t nnn;
} IKEY1, *PIKEY1;

void NTR_CryptUp(uint32_t* pCardHash, uint32_t* aPtr);
void NTR_CryptDown(uint32_t* pCardHash, uint32_t* aPtr);

void NTR_InitKey(uint32_t aGameCode, uint32_t *pCardHash, int nCardHash, uint32_t *pKeyCode, int level, int iCardDevice);
void NTR_InitKey1(uint8_t *aCmdData, IKEY1 *pKey1, int iCardDevice);

void NTR_CreateEncryptedCommand(uint8_t aCommand, uint32_t *pCardHash, uint8_t *aCmdData, IKEY1* pKey1, uint32_t aBlock);
void NTR_DecryptSecureArea(uint32_t aGameCode, uint32_t *pCardHash, int nCardHash, uint32_t *pKeyCode, uint32_t *pSecureArea, int iCardDevice);
