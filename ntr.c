#include <stdint.h>

#include "registers.h"
#include "header.h"

const uint8_t chipid[4] = {'H', 'E', 'L', 'P'};

extern void (*c_irq_handler)(void);
volatile uint8_t cs1_triggered;

uint32_t bytes;

volatile uint8_t *cmdpos;
uint8_t cmdbuf[8];

const uint8_t *output_buffer;
uint32_t buffer_size;
volatile const uint8_t *outpos;

void read_irq(void);
void write_irq(void);
void null_write_irq(void);

int pimain(void) {
    cmdpos = cmdbuf;
    GPFSEL0 &= ~((7 << (D0*3)) | (7 << (D1*3)) | (7 << (D2*3)) | (7 << (D3*3))
                |(7 << (D4*3)) | (7 << (D5*3)) | (7 << (D6*3)) | (7 << (D7*3)));
    c_irq_handler = read_irq;
    // Finish init, setup IRQ and such.
    cs1_triggered = 0;
    while (1) {
        while (cmdbuf+8 > cmdpos) { // Read command
            ;
        }
        GPFSEL0 |= (1 << (D0*3)) | (1 << (D1*3)) | (1 << (D2*3)) | (1 << (D3*3))
                |  (1 << (D4*3)) | (1 << (D5*3)) | (1 << (D6*3)) | (1 << (D7*3));
        c_irq_handler = write_irq;

        switch (cmdbuf[0]) {
            case 0x90: // Chip ID
                outpos = output_buffer = chipid;
                buffer_size = 4;
                break;
            case 0x00:
                outpos = output_buffer = header;
                buffer_size = header_size;
                break;
            case 0x9F:
                c_irq_handler = null_write_irq;
                GPSET0 = 0xFF << D0;
                outpos = output_buffer = 0;
                buffer_size = 1;
                break;
        }

        while (1) { // Output data
            if (output_buffer + buffer_size <= outpos) {
                outpos = output_buffer;
            }
            if (cs1_triggered) {
                break;
            }
        }
        GPFSEL0 &= ~((7 << (D0*3)) | (7 << (D1*3)) | (7 << (D2*3)) | (7 << (D3*3))
                    |(7 << (D4*3)) | (7 << (D5*3)) | (7 << (D6*3)) | (7 << (D7*3)));
        c_irq_handler = read_irq;
        cmdpos = cmdbuf;
        cs1_triggered = 0;
    }
    return 0;
}

// These aren't handling the reset line... Or CS2...
void read_irq(void) {
    *cmdpos++ = GPLEV0 >> D0; // Nice and simple, right?
    GPEDS0 = 1 << CLK;
}

void write_irq(void) {
    if (GPEDS0 & (1 << CS1)) {
        // Notify main loop.
        GPEDS0 = 1 << CS1;
        return;
    }
    uint8_t value = *outpos++;
    GPSET0 = value << D0;
    GPCLR0 = ~value << D0; // Do I need to optimize this?
    GPEDS0 = 1 << CLK;
}

void null_write_irq(void) {
    if (GPEDS0 & (1 << CS1)) {
        // Notify main loop.
    }
    GPEDS0 = (1 << CLK) | (1 << CS1);
    return;
}
