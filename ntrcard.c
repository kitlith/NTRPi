#include <stdint.h>

#include "ntr_commands.h"
#include "header.h"

#include "pins.h"

extern void dummy(unsigned);
extern void start_mmu ( unsigned int, unsigned int );
extern void stop_mmu ( void );
extern void invalidate_tlbs ( void );

#define MMUTABLEBASE 0x00004000

unsigned int mmu_section ( unsigned int vadd, unsigned int padd, unsigned int flags )
{
    unsigned int ra;
    unsigned int rb;
    unsigned int rc;

    ra=vadd>>20;
    rb=MMUTABLEBASE|(ra<<2);
    rc=(padd&0xFFF00000)|0xC00|flags|2;
    *(uint32_t*)rb = rc;
    return(0);
}

struct ntr_state {
    uint8_t currentRawCmd[8];
    uint8_t command;
};

static struct ntr_state state;

static uint32_t debugCount;

static inline void ntr_readcommand() {
    int iii;

    data_in();
    ++debugCount;
    if (debugCount >= 2) debug();
    for (iii = 7; iii >= 0; --iii) {
        while (!pinevent(CLK)) {;} // Wait for clock to rise.
        state.currentRawCmd[iii] = ntr_readbyte();
    }

    data_out();

    state.command = (enum ntrcard_command)(state.currentRawCmd[7] & 0xFF);
}

static inline void ntr_write_buffer(const uint8_t *data, uint32_t size) {
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

const uint8_t chipid[] = {'1', '3', '3', '7'};

int pimain(void) {
    // for(unsigned ra=0;;ra+=0x00100000) {
    //     mmu_section(ra,ra,0x0000);
    //     if(ra==0xFFF00000) break;
    // }
    // mmu_section(0x00000000,0x00000000,0x0000|8|4);
    // mmu_section(0x00100000,0x00100000,0x0000|8|4);
    // mmu_section(0x00200000,0x00200000,0x0000|8|4);
    // mmu_section(0x20000000,0x20000000,0x0000); //NOT CACHED!
    // mmu_section(0x20100000,0x20000000,0x0000); //NOT CACHED!
    // mmu_section(0x20200000,0x20200000,0x0000); //NOT CACHED!
    // start_mmu(MMUTABLEBASE,0x00000001|0x1000|0x0004);

    initpins();
    debugCount = 0;

    uint8_t debugBuffer[2];
    debugBuffer[0] = 0x45;

    const uint8_t *buffer;
    uint32_t buffer_size;

    while (1) {
        ntr_readcommand();

        switch (state.command) {
            case NTRCARD_CMD_DUMMY:
                ntr_sendbyte(0xFF);
                while (!pinevent(CS1)) {;} // ALL 0xFF!
                continue;
            case NTRCARD_CMD_HEADER_READ:
                buffer = header;
                buffer_size = header_size;
                break;
            case NTRCARD_CMD_HEADER_CHIPID:
                buffer = chipid;
                buffer_size = 0x4;
                break;
            default: // Unrecognised command!
                debugBuffer[1] = state.command;
                buffer = debugBuffer;
                buffer_size = 2; // Why not?
                break;
        }
        ntr_write_buffer(buffer, buffer_size);
    }
    return 0; // Why do I bother?
}
