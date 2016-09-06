#include "pins.h"

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
    GPSET0 = ((byte & 0x0F) << D0) | ((byte & 0xF0) << D4);
    GPCLR0 = ~(((byte & 0x0F) << D0) | ((byte & 0xF0)) << D4);
    #endif // PI1

    #ifdef PI2
    GPSET0 =   byte << 2;
    GPCLR0 = ~(byte << 2);
    #endif // PI1
}

uint8_t ntr_readbyte(void) {
    #ifdef PI1
    return ((GPLEV0 >> D0) & 0x0F) | ((GPLEV0 >> D4) & 0xF0);
    #endif // PI1

    #ifdef PI2
    return ((GPLEV0) >> D0) & 0xFF;
    #endif
}
