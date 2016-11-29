#include <stdint.h>
#define PI_VER 2
#include "registers.h"

volatile uint8_t *cmdpos;
uint8_t cmdbuf[8];
volatile const uint8_t *outpos;

/* The following is the remains of the attempted use of interrupts
   to increase preformance. This approach is currently scrapped, but
   I'm keeping the code around for future reference. */

void read_irq(void);
void write_irq(void);
void null_write_irq(void);

extern void enable_irq(void);

extern void (*c_irq_handler)(void);
volatile uint8_t cs1_triggered;

// These aren't handling the reset line... Or CS2...
void read_irq(void) {
    *cmdpos++ = GPLEV0 >> D0; // Nice and simple, right?
    GPEDS0 = 1 << CLK; // TODO: What if it's actually CS1?
}

void write_irq(void) {
    if (GPEDS0 & (1 << CS1)) {
        // Notify main loop.
        cs1_triggered = 1;
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
        cs1_triggered = 1;
    }
    GPEDS0 = (1 << CLK) | (1 << CS1);
    return;
}
