#include <stdint.h>

#include "ntr_encryption.h"
#include "ntr_commands.h"
#include "mem.h"
#include "payload.h"

#define PI1
// #define PI2

#ifdef PI1
#define PERIPHERAL_BASE 0x20000000 // Pi1

#define D0 7
#define D1 8
#define D2 9
#define D3 10
#define D4 21
#define D5 22
#define D6 23
#define D7 24

#define CLK 11
#define CS1 25
#define RST 0
#define CS2 1

#endif

#ifdef PI2
#define PERIPHERAL_BASE 0x3F000000 // Pi2

#define D0 2
#define D1 3
#define D2 4
#define D3 5
#define D4 6
#define D5 7
#define D6 8
#define D7 9

#define CLK 10
#define CS1 11
#define RST 12
#define CS2 13
#endif

#define GPIO_BASE (PERIPHERAL_BASE + 0x00200000)

#define GPFSEL0 (*(volatile uint32_t*)(GPIO_BASE + 0x00))
#define GPFSEL1 (*(volatile uint32_t*)(GPIO_BASE + 0x04))
#define GPFSEL2 (*(volatile uint32_t*)(GPIO_BASE + 0x08))

#define GPSET0  (*(volatile uint32_t*)(GPIO_BASE + 0x1C))

#define GPCLR0  (*(volatile uint32_t*)(GPIO_BASE + 0x28))

#define GPLEV0  (*(volatile uint32_t*)(GPIO_BASE + 0x34))

#define GPEDS0  (*(volatile uint32_t*)(GPIO_BASE + 0x40))

#define GPREN0  (*(volatile uint32_t*)(GPIO_BASE + 0x4C))
#define GPFEN0  (*(volatile uint32_t*)(GPIO_BASE + 0x58))

extern void dummy(unsigned);
extern void rbits64(uint64_t*);

void data_in(void) {
    // Set pins D0 - D7 as input.
    #ifdef PI1
    GPFSEL0 &= ~((7 << (D0*3)) | 7 << (D1*3) | 7 << (D2*3));
    GPFSEL1 &= ~(7 << ((D3-10)*3));
    GPFSEL2 &= ~((7 << ((D4-20)*3)) | (7 << ((D5-20)*3)) | (7 << ((D6-20)*3)) | (7 << ((D7-20)*3)));
    #endif // PI1

    #ifdef PI2
    GPFSEL0 &= ~((7 << (D0*3)) | (7 << (D1*3)) | (7 << (D2*3)) | (7 << (D3*3)) |
                 (7 << (D4*3)) | (7 << (D5*3)) | (7 << (D6*3)) | (7 << (D7*3)));
    #endif // PI2

    GPREN0 |= (1 << CLK); // Enable rising edge detect on CLK
    GPFEN0 &= (1 << CLK);
}

void data_out(void) {
    // Set pins 2-9 as output. (D0 - D7)
    #ifdef PI1
    GPFSEL0 |= (1 << (D0*3)) | 1 << (D1*3) | 1 << (D2*3);
    GPFSEL1 |= 1 << ((D3-10)*3);
    GPFSEL2 |= (1 << ((D4-20)*3)) | (1 << ((D5-20)*3)) | (1 << ((D6-20)*3)) | (1 << ((D7-20)*3));
    #endif // PI1

    #ifdef PI2
    GPFSEL0 |=  (1 << (D0*3)) | (1 << (D1*3)) | (1 << (D2*3)) | (1 << (D3*3)) |
                (1 << (D4*3)) | (1 << (D5*3)) | (1 << (D6*3)) | (1 << (D7*3));
    #endif // PI2
    GPFEN0 |= (1 << CLK); // Enable falling edge detect on CLK
    GPREN0 &= (1 << CLK);
}

void initpins(void) {
    // Set pins CLK, CS1, RST, CS2 as input.
    #ifdef PI1
    GPFSEL0 &= ~((7 << (RST * 3)) | (7 << (CS2 * 3)));
    GPFSEL1 &= ~(7 << ((CLK - 10) * 3));
    GPFSEL2 &= ~(7 << ((CS1 - 20) * 3));
    #endif

    #ifdef PI2
    GPFSEL1 &= ~((7 << ((CLK - 10) * 3)) | (7 << ((CS1 - 10) * 3)) | (7 << ((RST-10) * 3)) | (7 << ((CS2-10) * 3)));
    #endif
    data_in();

    GPREN0 |= (1<<CS1); // Enable rising edge input for CS1
}

void ntr_sendbyte(const uint8_t byte) {
    #ifdef PI1
    GPSET0 = ((byte & 0x0F) << 7) | ((byte & 0xF0) << 21);
    GPCLR0 = ~(((byte & 0x0F) << 7) | ((byte & 0xF0)) << 21);
    #endif // PI1

    #ifdef PI2
    GPSET0 =   byte << 2;
    GPCLR0 = ~(byte << 2);
    #endif // PI1
}

enum encryptionState {
    NONE,
    KEY1_ENC,
    KEY2_ENC
};

struct ntr_state {
    uint32_t pCardHash[0x412];
    uint8_t currentRawCmd[8];
    enum encryptionState encState;
    enum ntrcard_command command;
    uint64_t key2_reg[2]; // These are actually supposed to be 39 bit registers!
};

static struct ntr_state state;

void NTR_KEY2Command_Decrypt() {
    uint64_t x; x = state.key2_reg[0];
    uint64_t y; y = state.key2_reg[1];

    int iii; // Thanks for the help, DeSemuME!
    for (iii = 7; iii >= 0; --iii) {
        x = (((x >> 5) ^ (x >> 17) ^ (x >> 18) ^ (x >> 31)) & 0xFF) + (x << 8);
        y = (((y >> 5) ^ (y >> 23) ^ (y >> 18) ^ (y >> 31)) & 0xFF) + (y << 8);
        state.currentRawCmd[iii] ^= (x ^ y) & 0xFF;
    }

    state.key2_reg[0] = x;
    state.key2_reg[1] = y;
}

void rbits39(uint64_t *ptr) {
    rbits64(ptr);
    *ptr = *ptr >> 25;
}

void ntr_readcommand() {
    int iii;

    data_in();

    for (iii = 7; iii >= 0; --iii) {
        while ( !(GPEDS0 & (1 << CLK)) ) {;} // Wait for clock to rise.
        GPEDS0 = (1 << CLK);
        state.currentRawCmd[iii] = (uint8_t)((GPLEV0 >> D0) & 0xFF);
    }

    data_out();

    if (state.encState == KEY1_ENC) {
        NTR_CryptDown(state.pCardHash, (uint32_t*)state.currentRawCmd);
        state.command = (enum ntrcard_command)(state.currentRawCmd[7] & 0xF0);
        return;
    } else if (state.encState == KEY2_ENC) {
        NTR_KEY2Command_Decrypt();
    }

    state.command = (enum ntrcard_command)(state.currentRawCmd[7] & 0xFF);
}

void ntr_write_buffer(const uint8_t *data, uint32_t size) {
    const uint8_t *ptr; ptr = data;
    while (1) {
        do {
            if (GPEDS0 & (1 << CS1)) return;
        } while ( !(GPEDS0 & (1 << 10)) );
        GPEDS0 = (1 << CLK);

        ntr_sendbyte(*ptr++);
        if (ptr >= data + size) { // Is this off by one?
            ptr = data;
        }
    }
}

int pimain(void) {
    state.key2_reg[0] = 0x58C56DE0E8; // YAAAY magic.
    rbits39(state.key2_reg + 0); // TODO: Do this beforehand and use those constants.
    state.key2_reg[1] = 0x5C879B9B05;
    rbits39(state.key2_reg + 8);
    state.encState = NONE;

    memset(state.pCardHash, 0, sizeof(state.pCardHash));

    initpins();

    while (1) {
        ntr_readcommand();

        switch (state.command) {
            default:
            case NTRCARD_CMD_DUMMY:
                ntr_sendbyte(0xFF);
                while (!(GPEDS0 & (1 << CS1))) {;} // ALL 0xFF!
                GPEDS0 = (1 << CS1);
                break;
            case NTRCARD_CMD_HEADER_READ:
                ntr_write_buffer(header, 0x200);
                GPEDS0 = (1 << CS1);
                break;
            case NTRCARD_CMD_HEADER_CHIPID:
                ntr_write_buffer(chipid, 0x4);
                GPEDS0 = (1 << CS1);
                break;
        }
    }
    return 0; // Why do I bother?
}
