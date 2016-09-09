#include <stdint.h>

#include "ntr_commands.h"
#include "mem.h"
#include "payload.h"
#include "header.h"

#include "pins.h"

extern void dummy(unsigned);

struct ntr_state {
    uint8_t currentRawCmd[8];
    enum ntrcard_command command;
};

static struct ntr_state state;


void ntr_readcommand() {
    int iii;

    data_in();
    debug();
    for (iii = 7; iii >= 0; --iii) {
        while (!pinevent(CLK)) {;} // Wait for clock to rise.
        state.currentRawCmd[iii] = ntr_readbyte();
    }

    data_out();

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
                ntr_write_buffer(header, header_size);
                break;
            case NTRCARD_CMD_HEADER_CHIPID:
                ntr_write_buffer(chipid, 0x4);
                break;
        }
    }
    return 0; // Why do I bother?
}
