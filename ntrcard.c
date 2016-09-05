#include <stdint.h>

#include "ntr_encryption.h"
#include "ntr_commands.h"
#include "mem.h"
#include "payload.h"

// #define PERIPHERAL_BASE 0x20000000 // Pi1
#define PERIPHERAL_BASE 0x3F000000 // Pi2
#define GPIO_BASE (PERIPHERAL_BASE + 0x00200000)

#define GPFSEL0 (*(volatile uint32_t*)(GPIO_BASE + 0x00))
#define GPFSEL1 (*(volatile uint32_t*)(GPIO_BASE + 0x04))

#define GPSET0  (*(volatile uint32_t*)(GPIO_BASE + 0x1C))

#define GPCLR0  (*(volatile uint32_t*)(GPIO_BASE + 0x28))

#define GPLEV0  (*(volatile uint32_t*)(GPIO_BASE + 0x34))

#define GPEDS0  (*(volatile uint32_t*)(GPIO_BASE + 0x40))

#define GPREN0  (*(volatile uint32_t*)(GPIO_BASE + 0x4C))
#define GPFEN0  (*(volatile uint32_t*)(GPIO_BASE + 0x58))

extern void dummy(unsigned);
extern void rbits64(uint64_t*);

void data_in(void) {
    // Set pins 2-9 as input. (D0 - D7)
    GPFSEL0 &= ~((7 << (2*3)) | (7 << (3*3)) | (7 << (4*3)) | (7 << (5*3)) |
                 (7 << (6*3)) | (7 << (7*3)) | (7 << (8*3)) | (7 << (9*3)));
    GPREN0 |= (1<<10); // Enable rising edge detect on pin 10. (CLK)
    GPFEN0 &= (1<<10);
}

void data_out(void) {
    // Set pins 2-9 as output. (D0 - D7)
    GPFSEL0 |=  (1 << (2*3)) | (1 << (3*3)) | (1 << (4*3)) | (1 << (5*3)) |
                (1 << (6*3)) | (1 << (7*3)) | (1 << (8*3)) | (1 << (9*3));
    GPFEN0 |= (1<<10); // Enable falling edge detect on pin 10. (CLK)
    GPREN0 &= (1<<10);
}

void initpins(void) {
    // Set pins 10 - 13 as input. (CLK, CS1, RST, CS2)
    GPFSEL1 &= ~((7 << (0 * 3)) | (7 << (1 * 3)) | (7 << (2 * 3)) | (7 << (3 * 3)));
    data_in();

    GPREN0 |= (1<<11); // Enable rising edge input for pin 11 (CS1)
}

void ntr_sendbyte(const uint8_t byte) {
    GPSET0 =   byte << 2;
    GPCLR0 = ~(byte << 2);
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
        while ( !(GPEDS0 & (1 << 10)) ) {;} // Wait for clock to rise.
        GPEDS0 = (1 << 10);
        state.currentRawCmd[iii] = (uint8_t)((GPLEV0 >> 2) & 0xFF);
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
            if (GPEDS0 & (1 << 11)) return;
        } while ( !(GPEDS0 & (1 << 10)) );
        GPEDS0 = (1 << 10);

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
                while (!(GPEDS0 & (1 << 11))) {
                    ntr_sendbyte(0xFF);
                } // ALL 0xFF!
                GPEDS0 = (1 << 11);
                break;
            case NTRCARD_CMD_HEADER_READ:
                ntr_write_buffer(header, 0x200);
                break;
            case NTRCARD_CMD_HEADER_CHIPID:
                ntr_write_buffer(chipid, 0x4);
                break;
        }
    }
    return 0; // Why do I bother?
}
