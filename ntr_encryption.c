/*
    ntr_encryption.c
    Copyright (C) 2010 yellow wood goblin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

// modifyed by osilloscopion (2 Jul 2016)
// Taken from Decrypt9WIP and modified by Kitlith (3 Sep 2016)

#include "mem.h"

#include "ntr_commands.h"
#include "ntr_encryption.h"

#define BSWAP32(val) ((((val >> 24) & 0xFF)) | (((val >> 16) & 0xFF) << 8) | (((val >> 8) & 0xFF) << 16) | ((val & 0xFF) << 24))

extern uint32_t ReadDataFlags;

void NTR_CryptUp(uint32_t* pCardHash, uint32_t* aPtr)
{
    uint32_t x = aPtr[1];
    uint32_t y = aPtr[0];
    uint32_t z;

    for(int ii=0;ii<0x10;++ii)
    {
        z = pCardHash[ii] ^ x;
        x = pCardHash[0x012 + ((z >> 24) & 0xff)];
        x = pCardHash[0x112 + ((z >> 16) & 0xff)] + x;
        x = pCardHash[0x212 + ((z >> 8) & 0xff)] ^ x;
        x = pCardHash[0x312 + ((z >> 0) & 0xff)] + x;
        x = y ^ x;
        y = z;
    }
    aPtr[0] = x ^ pCardHash[0x10];
    aPtr[1] = y ^ pCardHash[0x11];
}

void NTR_CryptDown(uint32_t* pCardHash, uint32_t* aPtr)
{
    uint32_t x = aPtr[1];
    uint32_t y = aPtr[0];
    uint32_t z;

    for(int ii=0x11;ii>0x01;--ii)
    {
        z = pCardHash[ii] ^ x;
        x = pCardHash[0x012 + ((z >> 24) & 0xff)];
        x = pCardHash[0x112 + ((z >> 16) & 0xff)] + x;
        x = pCardHash[0x212 + ((z >> 8) & 0xff)] ^ x;
        x = pCardHash[0x312 + ((z >> 0) & 0xff)] + x;
        x = y ^ x;
        y = z;
    }
    aPtr[0] = x ^ pCardHash[0x01];
    aPtr[1] = y ^ pCardHash[0x00];
}

// chosen by fair dice roll.
// guaranteed to be random.
#define getRandomNumber() (4)

void NTR_InitKey1(uint8_t* aCmdData, IKEY1* pKey1, int iCardDevice)
{
    pKey1->iii = getRandomNumber() & 0x00000fff;
    pKey1->jjj = getRandomNumber() & 0x00000fff;
    pKey1->kkkkk = getRandomNumber() & 0x000fffff;
    pKey1->llll = getRandomNumber() & 0x0000ffff;
    pKey1->mmm = getRandomNumber() & 0x00000fff;
    pKey1->nnn = getRandomNumber() & 0x00000fff;

    if(iCardDevice) //DSi
      aCmdData[7]=NTRCARD_CMD_ACTIVATE_BF2;  //0x3D
    else
      aCmdData[7]=NTRCARD_CMD_ACTIVATE_BF;

    aCmdData[6] = (uint8_t)(pKey1->iii >> 4);
    aCmdData[5] = (uint8_t)((pKey1->iii << 4) | (pKey1->jjj >> 8));
    aCmdData[4] = (uint8_t)pKey1->jjj;
    aCmdData[3] = (uint8_t)(pKey1->kkkkk >> 16);
    aCmdData[2] = (uint8_t)(pKey1->kkkkk >> 8);
    aCmdData[1] = (uint8_t)pKey1->kkkkk;
    aCmdData[0] = (uint8_t)getRandomNumber();
}

void NTR_ApplyKey(uint32_t* pCardHash, int nCardHash, uint32_t* pKeyCode)
{
    uint32_t scratch[2];

    NTR_CryptUp(pCardHash, &pKeyCode[1]);
    NTR_CryptUp(pCardHash, &pKeyCode[0]);
    memset(scratch, 0, sizeof(scratch));

    for(int ii=0;ii<0x12;++ii)
    {
        pCardHash[ii] = pCardHash[ii] ^ BSWAP32(pKeyCode[ii%2]);
    }

    for(int ii=0;ii<nCardHash;ii+=2)
    {
        NTR_CryptUp(pCardHash, scratch);
        pCardHash[ii] = scratch[1];
        pCardHash[ii+1] = scratch[0];
    }
}

void NTR_InitKey(uint32_t aGameCode, uint32_t* pCardHash, int nCardHash, uint32_t* pKeyCode, int level, int iCardDevice)
{
	if(iCardDevice)
	{
		const uint8_t* BlowfishTwl = (const uint8_t*)0x01FFD3E0;
		memcpy(pCardHash, BlowfishTwl, 0x1048);
	}
	else
    {
		const uint8_t* BlowfishNtr = (const uint8_t*)0x01FFE428;
		memcpy(pCardHash, BlowfishNtr, 0x1048);
	}

    pKeyCode[0] = aGameCode;
    pKeyCode[1] = aGameCode/2;
    pKeyCode[2] = aGameCode*2;

	if (level >= 1) NTR_ApplyKey(pCardHash, nCardHash, pKeyCode);
    if (level >= 2) NTR_ApplyKey(pCardHash, nCardHash, pKeyCode);

    pKeyCode[1] = pKeyCode[1]*2;
    pKeyCode[2] = pKeyCode[2]/2;

    if (level >= 3) NTR_ApplyKey(pCardHash, nCardHash, pKeyCode);
}

void NTR_CreateEncryptedCommand(uint8_t aCommand, uint32_t* pCardHash, uint8_t* aCmdData, IKEY1* pKey1, uint32_t aBlock)
{
    uint32_t iii,jjj;
    if(aCommand!=NTRCARD_CMD_SECURE_READ) aBlock=pKey1->llll;
    if(aCommand==NTRCARD_CMD_ACTIVATE_SEC)
    {
        iii=pKey1->mmm;
        jjj=pKey1->nnn;
    }
    else
    {
        iii=pKey1->iii;
        jjj=pKey1->jjj;
    }
    aCmdData[7]=(uint8_t)(aCommand|(aBlock>>12));
    aCmdData[6]=(uint8_t)(aBlock>>4);
    aCmdData[5]=(uint8_t)((aBlock<<4)|(iii>>8));
    aCmdData[4]=(uint8_t)iii;
    aCmdData[3]=(uint8_t)(jjj>>4);
    aCmdData[2]=(uint8_t)((jjj<<4)|(pKey1->kkkkk>>16));
    aCmdData[1]=(uint8_t)(pKey1->kkkkk>>8);
    aCmdData[0]=(uint8_t)pKey1->kkkkk;

    NTR_CryptUp(pCardHash, (uint32_t*)aCmdData);

    pKey1->kkkkk+=1;
}

void NTR_DecryptSecureArea(uint32_t aGameCode, uint32_t* pCardHash, int nCardHash, uint32_t* pKeyCode, uint32_t* pSecureArea, int iCardDevice)
{
    NTR_InitKey(aGameCode, pCardHash, nCardHash, pKeyCode, 2, iCardDevice);
    NTR_CryptDown(pCardHash, pSecureArea);
    NTR_InitKey(aGameCode, pCardHash, nCardHash, pKeyCode, 3, iCardDevice);
    for(int ii=0;ii<0x200;ii+=2) NTR_CryptDown(pCardHash, pSecureArea + ii);
}


// uint32_t NTR_GetIDSafe(uint32_t flags, const uint8_t* command, uint32_t Delay)
// {
//     uint32_t data = 0;
//     Delay = ((Delay & 0x3fff) * 1000) / 0x83;
//     ioDelay(Delay);
//     cardWriteCommand(command);
//     REG_NTRCARDROMCNT = flags | NTRCARD_BLK_SIZE(7);
//
//     do
//     {
//         if (REG_NTRCARDROMCNT & NTRCARD_DATA_READY)
//         {
//             data = REG_NTRCARDFIFO;
//         }
//     }
//     while(REG_NTRCARDROMCNT & NTRCARD_BUSY);
//
//     return data;
// }

// void NTR_CmdSecure(uint32_t flags, void* buffer, uint32_t length, uint8_t* pcmd, uint32_t Delay)
// {
//     Delay = ((Delay & 0x3fff) * 1000) / 0x83;
//     ioDelay(Delay);
//     cardPolledTransfer(flags, buffer, length, pcmd);
// }

// bool NTR_Secure_Init(uint8_t* header, uint32_t CartID, int iCardDevice)
// {
// 	uint32_t iGameCode;
//     uint32_t iCardHash[0x412] = {0};
//     uint32_t iKeyCode[3] = {0};
//     uint32_t* secureArea=(uint32_t*)(header + 0x4000);
//     uint8_t cmdData[8] __attribute__((aligned(32)));
//     const uint8_t cardSeedBytes[]={0xE8,0x4D,0x5A,0xB1,0x17,0x8F,0x99,0xD5};
//     IKEY1 iKey1 ={0};
//     bool iCheapCard = (CartID & 0x80000000) != 0;
//     uint32_t cardControl13 = *((uint32_t*)&header[0x60]);
//     uint32_t cardControlBF = *((uint32_t*)&header[0x64]);
// 	uint16_t readTimeout = *((uint16_t*)&header[0x6E]); readTimeout*=8;
// 	uint8_t deviceType = header[0x13];
// 	int nCardHash = sizeof(iCardHash) / sizeof(iCardHash[0]);
//     uint32_t flagsKey1=NTRCARD_ACTIVATE|NTRCARD_nRESET|(cardControl13&(NTRCARD_WR|NTRCARD_CLK_SLOW))|((cardControlBF&(NTRCARD_CLK_SLOW|NTRCARD_DELAY1(0x1FFF)))+((cardControlBF&NTRCARD_DELAY2(0x3F))>>16));
//     uint32_t flagsSec=(cardControlBF&(NTRCARD_CLK_SLOW|NTRCARD_DELAY1(0x1FFF)|NTRCARD_DELAY2(0x3F)))|NTRCARD_ACTIVATE|NTRCARD_nRESET|NTRCARD_SEC_EN|NTRCARD_SEC_DAT;
//
//     iGameCode = *((uint32_t*)&header[0x0C]);
//     ReadDataFlags = cardControl13 & ~ NTRCARD_BLK_SIZE(7);
//     NTR_InitKey(iGameCode, iCardHash, nCardHash, iKeyCode, iCardDevice?1:2, iCardDevice);
//
//     if(!iCheapCard) flagsKey1 |= NTRCARD_SEC_LARGE;
//     //Debug("iCheapCard=%d, readTimeout=%d", iCheapCard, readTimeout);
//
// 	NTR_InitKey1(cmdData, &iKey1, iCardDevice);
//     //Debug("cmdData=%02X %02X %02X %02X %02X %02X %02X %02X ", cmdData[0], cmdData[1], cmdData[2], cmdData[3], cmdData[4], cmdData[5], cmdData[6], cmdData[7]);
//     //Debug("iKey1=%08X %08X %08X", iKey1.iii, iKey1. jjj, iKey1. kkkkk);
//     //Debug("iKey1=%08X %08X %08X", iKey1. llll, iKey1. mmm, iKey1. nnn);
//
//     NTR_CmdSecure((cardControl13 & (NTRCARD_WR | NTRCARD_nRESET | NTRCARD_CLK_SLOW)) | NTRCARD_ACTIVATE, NULL, 0, cmdData, 0);
//
//     NTR_CreateEncryptedCommand(NTRCARD_CMD_ACTIVATE_SEC, iCardHash, cmdData, &iKey1, 0);
//     //Debug("cmdData=%02X %02X %02X %02X %02X %02X %02X %02X ", cmdData[0], cmdData[1], cmdData[2], cmdData[3], cmdData[4], cmdData[5], cmdData[6], cmdData[7]);
//     if(iCheapCard)
//     {
//         NTR_CmdSecure(flagsKey1, NULL, 0, cmdData, 0);
//     }
//     NTR_CmdSecure(flagsKey1, NULL, 0, cmdData, readTimeout);
//
//     REG_NTRCARDROMCNT = 0;
//     REG_NTRCARDSEEDX_L = cardSeedBytes[deviceType & 0x07] | (iKey1.nnn << 15) | (iKey1.mmm << 27) | 0x6000;
//     REG_NTRCARDSEEDY_L = 0x879b9b05;
//     REG_NTRCARDSEEDX_H = iKey1.mmm >> 5;
//     REG_NTRCARDSEEDY_H = 0x5c;
//     REG_NTRCARDROMCNT = NTRCARD_nRESET | NTRCARD_SEC_SEED | NTRCARD_SEC_EN | NTRCARD_SEC_DAT;
//
//     flagsKey1 |= NTRCARD_SEC_EN | NTRCARD_SEC_DAT;
//     NTR_CreateEncryptedCommand(NTRCARD_CMD_SECURE_CHIPID, iCardHash, cmdData, &iKey1, 0);
//     //Debug("cmdData=%02X %02X %02X %02X %02X %02X %02X %02X ", cmdData[0], cmdData[1], cmdData[2], cmdData[3], cmdData[4], cmdData[5], cmdData[6], cmdData[7]);
//     uint32_t SecureCartID = 0;
//     if(iCheapCard)
//     {
//         NTR_CmdSecure(flagsKey1, NULL, 0, cmdData, 0);
//     }
//
//     //NTR_CmdSecure(flagsKey1, &SecureCartID, sizeof(SecureCartID), cmdData, readTimeout);
//     SecureCartID = NTR_GetIDSafe(flagsKey1, cmdData, readTimeout);
//
//     if (SecureCartID != CartID)
//     {
// 		Debug("Invalid SecureCartID. (%08X != %08X)", SecureCartID, CartID);
//         return false;
//     }
//
//     int secureAreaOffset = 0;
//     for(int secureBlockNumber=4;secureBlockNumber<8;++secureBlockNumber)
//     {
//         NTR_CreateEncryptedCommand(NTRCARD_CMD_SECURE_READ, iCardHash, cmdData, &iKey1, secureBlockNumber);
//         if (iCheapCard)
//         {
//             NTR_CmdSecure(flagsSec, NULL, 0, cmdData, 0);
//             for(int ii=8;ii>0;--ii)
//             {
//                 NTR_CmdSecure(flagsSec | NTRCARD_BLK_SIZE(1), secureArea + secureAreaOffset, 0x200, cmdData, readTimeout);
//                 secureAreaOffset += 0x200 / sizeof(uint32_t);
//             }
//         }
//         else
//         {
//             NTR_CmdSecure(flagsSec | NTRCARD_BLK_SIZE(4) | NTRCARD_SEC_LARGE, secureArea + secureAreaOffset, 0x1000, cmdData, readTimeout);
//             secureAreaOffset += 0x1000 / sizeof(uint32_t);
//         }
//     }
//
//     NTR_CreateEncryptedCommand(NTRCARD_CMD_DATA_MODE, iCardHash, cmdData, &iKey1, 0);
//     if(iCheapCard)
//     {
//         NTR_CmdSecure(flagsKey1, NULL, 0, cmdData, 0);
//     }
//     NTR_CmdSecure(flagsKey1, NULL, 0, cmdData, readTimeout);
//
//     if(!iCardDevice) //CycloDS doesn't like the dsi secure area being decrypted
//     {
// 		NTR_DecryptSecureArea(iGameCode, iCardHash, nCardHash, iKeyCode, secureArea, iCardDevice);
// 	}
//
//     //Debug("secure area %08X %08X", secureArea[0], secureArea[1]);
//     if(secureArea[0] == 0x72636e65/*'encr'*/ && secureArea[1] == 0x6a624f79/*'yObj'*/)
//     {
//         secureArea[0] = 0xe7ffdeff;
//         secureArea[1] = 0xe7ffdeff;
//     }
//     else
//     {
//         Debug("Invalid secure area.(%08X %08X)", secureArea[0], secureArea[1]);
//         //dragon quest 5 has invalid secure area. really.
// 		//return false;
//     }
//
//     return true;
// }
