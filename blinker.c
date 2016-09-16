#include "registers.h"
#include "uart.h"
//extern void dummy(uint32_t);
extern void enable_irq(void);
extern void dummy(unsigned);

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
    uart_init();
    //debug + initial case
    hexstring(0x12345678);
    hexstring(GPFSEL0);

    // Set D0 as output. Reminder: Don't do fancy stuff.
    GPFSEL0 &= ~FUNSEL(D0, 7);
    GPFSEL0 |=  FUNSEL(D0, 1);
    GPCLR0 = 1 << D0;
    hexstring(GPFSEL0);

    // GPFSEL1 &= ~FUNSEL(CLK, 7); // For future testing...
    ARM_TIMER_CLI = 0;
    ARM_TIMER_CTL |= 1 << 5; // Timer should already be running...
    hexstring(ARM_TIMER_CTL);

    // Cross fingers...
    IRQ_ENABLE_BASIC = 1;
    enable_irq();
    while (1) {
        for (unsigned ra = 0x100000; ra > 0; --ra) dummy(ra);
        hexstring(icount);
    }

    return 0;
}
