#include <stdint.h>

#include "ntr_encryption.h"
#include "ntr_commands.h"
#include "mem.h"
#include "payload.h"

#include "pins.h"

extern void dummy(unsigned);
extern void rbits64(uint64_t*);

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
        while (!pinevent(CLK)) {;} // Wait for clock to rise.
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
            if (pinevent(CS1)) return;
        } while (!pinevent(CLK));

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
                while (!pinevent(CS1)) {;} // ALL 0xFF!
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
