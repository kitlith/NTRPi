#include "config.h"
#include "pins.h"
#include "registers.h"

inline void data_in(void) {
    // Set pins D0 - D7 as input.
    #if PI_VER == 1
    GPFSEL0 &= ~((7 << (D0*3)) | 7 << (D1*3) | 7 << (D2*3));
    GPFSEL1 &= ~(7 << ((D3-10)*3));
    GPFSEL2 &= ~((7 << ((D4-20)*3)) | (7 << ((D5-20)*3)) | (7 << ((D6-20)*3)) | (7 << ((D7-20)*3)));

    GPFSEL1 &= ~(7 << (3 * (LED - 10))); // Set status LED as output.
    GPFSEL1 |= 1 << (3 * (LED - 10)); // DEBUG, really.

    GPSET0 = 1 << LED; // Turn LED off for debugging.
    #endif // PI_VER == 1

    #if PI_VER == 2
    GPFSEL0 &= ~((7 << (D0*3)) | (7 << (D1*3)) | (7 << (D2*3)) | (7 << (D3*3)) |
                 (7 << (D4*3)) | (7 << (D5*3)) | (7 << (D6*3)) | (7 << (D7*3)));
    #endif // PI_VER == 2

    GPAREN0 |= (1 << CLK); // Enable rising edge detect on CLK
    GPAFEN0 &= (1 << CLK);
}

inline void data_out(void) {
    // Set pins 2-9 as output. (D0 - D7)
    #if PI_VER == 1
    GPFSEL0 |= (1 << (D0*3)) | 1 << (D1*3) | 1 << (D2*3);
    GPFSEL1 |= 1 << ((D3-10)*3);
    GPFSEL2 |= (1 << ((D4-20)*3)) | (1 << ((D5-20)*3)) | (1 << ((D6-20)*3)) | (1 << ((D7-20)*3));
    #endif // PI_VER == 1

    #if PI_VER == 2
    GPFSEL0 |=  (1 << (D0*3)) | (1 << (D1*3)) | (1 << (D2*3)) | (1 << (D3*3)) |
                (1 << (D4*3)) | (1 << (D5*3)) | (1 << (D6*3)) | (1 << (D7*3));
    #endif // PI_VER == 2
    GPAFEN0 |= (1 << CLK); // Enable falling edge detect on CLK
    GPAREN0 &= (1 << CLK);
}

inline void initpins(void) {
    // Set pins CLK, CS1, RST, CS2 as input.
    #if PI_VER == 1
    GPFSEL1 &= ~((7 << ((CLK - 10) * 3)) | (7 << ((CS2 - 10) * 3)) |
                 (7 << ((RST - 10) * 3)));
    GPFSEL2 &= ~(7 << ((CS1 - 20) * 3));
    #endif

    #if PI_VER == 2
    GPFSEL1 &= ~((7 << ((CLK - 10) * 3)) | (7 << ((CS1 - 10) * 3)));
    GPFSEL2 &= ~((7 << ((RST - 20) * 3)) | (7 << ((CS2 - 20) * 3)));
    #endif
    data_in();

    GPAREN0 |= (1<<CS1); // Enable rising edge input for CS1
}

inline void ntr_sendbyte(uint8_t byte) {
    #if PI_VER == 1
    GPSET0 = ((byte & 0x0F) << D0) | ((byte & 0xF0) << (D4 - 4));
    byte = ~byte;
    GPCLR0 = ((byte & 0x0F) << D0) | ((byte & 0xF0) << (D4 - 4));
    #endif // PI_VER == 1

    #if PI_VER == 2
    GPSET0 = byte << 2;
    GPCLR0 = ~byte << 2;
    #endif // PI_VER == 2
}

inline uint8_t ntr_readbyte(void) {
    #if PI_VER == 1
    return ((GPLEV0 >> D0) & 0x0F) | ((GPLEV0 >> (D4-4)) & 0xF0);
    #endif // PI_VER == 1

    #if PI_VER == 2
    return ((GPLEV0) >> D0) & 0xFF;
    #endif
}

inline void debug(void) {
    GPCLR0 = 1 << LED; // Turn status LED on. This is active low, yes.
}
