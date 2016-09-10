#include <stdint.h>

#include "ntr_commands.h"
#include "mem.h"
#include "payload.h"
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

void ntr_readcommand() {
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
    for(unsigned ra=0;;ra+=0x00100000) {
        mmu_section(ra,ra,0x0000);
        if(ra==0xFFF00000) break;
    }
    mmu_section(0x00000000,0x00000000,0x0000|8|4);
    mmu_section(0x00100000,0x00100000,0x0000|8|4);
    mmu_section(0x00200000,0x00200000,0x0000|8|4);
    mmu_section(0x20000000,0x20000000,0x0000); //NOT CACHED!
    mmu_section(0x20100000,0x20000000,0x0000); //NOT CACHED!
    mmu_section(0x20200000,0x20200000,0x0000); //NOT CACHED!
    start_mmu(MMUTABLEBASE,0x00000001|0x1000|0x0004);

    initpins();
    debugCount = 0;

    while (1) {
        ntr_readcommand();

        switch (state.command) {
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
            case 0xFF:
                ntr_sendbyte(0x00);
                while (!pinevent(CS1)) {;} // ALL 0x00!
                break;
            default: // Unrecognised command!
                ntr_sendbyte(state.command); // Why not?
                while (!pinevent(CS1)) {;}
                break;
        }
    }
    return 0; // Why do I bother?
}
