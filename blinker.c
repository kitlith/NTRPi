#include "registers.h"

extern void dummy(uint32_t);
extern void enable_irq(void);

volatile uint32_t icount;

void c_irq_handler(void) {
    ++icount;
    if (icount & 1) {
        GPSET0 = 1 << D0;
    } else {
        GPCLR0 = 1 << D0;
    }
    ARM_TIMER_CLI = 0;
}

int pimain(void) {
    IRQ_DISABLE_BASIC = 1;

    GPFSEL0 = (GPFSEL0 & (7 << D0)) | (1 << D0);
    // GPFSEL1 &= 7 << CLK; // For future testing...

    ARM_TIMER_CTL = 0x003E0000; // Yaaay magic values...
    ARM_TIMER_LOD = 1000000 - 1;
    ARM_TIMER_RLD = 1000000 - 1;
    ARM_TIMER_DIV = 0xF9;
    ARM_TIMER_CLI = 0;
    ARM_TIMER_CTL = 0x003E00A2;

    // Cross fingers...
    IRQ_ENABLE_BASIC = 1;
    enable_irq();
    while (1) continue;

    return 0;
}
