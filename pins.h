#pragma once

#include <stdint.h>
#include "config.h"
#include "registers.h"

#if PI_VER == 1

#define D0 7
#define D1 8
#define D2 9
#define D3 10
#define D4 22
#define D5 23
#define D6 24
#define D7 25

#define CLK 11
#define CS1 27
#define RST 14
#define CS2 15

#define LED 16

#elif PI_VER == 2

#define D0 2
#define D1 3
#define D2 4
#define D3 5
#define D4 6
#define D5 7
#define D6 8
#define D7 9

#define CLK 13 //defined for hedge's pi
#define CS1 19 //defined for hedge's pi
#define RST 20
#define CS2 21

#define LED D0

#else
#error "PI_VER needs to be defined as either 1 or 2!"

#endif

inline void data_in(void) {
    // Set pins D0 - D7 as input.
    #if PI_VER == 1
    GPFSEL0 &= ~(FUNSEL(D0, 7) | FUNSEL(D1, 7) | FUNSEL(D2, 7));
    GPFSEL1 &= ~(FUNSEL(D3, 7));
    GPFSEL2 &= ~(FUNSEL(D4, 7) | FUNSEL(D5, 7) | FUNSEL(D6, 7) | FUNSEL(D7, 7));

    GPSET0 = 1 << LED; // Turn LED off for debugging.
    #endif // PI_VER == 1

    #if PI_VER == 2
    GPFSEL0 &= ~(FUNSEL(D0, 7) | FUNSEL(D1, 7) | FUNSEL(D2, 7) | FUNSEL(D3, 7) |
                 FUNSEL(D4, 7) | FUNSEL(D5, 7) | FUNSEL(D6, 7) | FUNSEL(D7, 7));
    #endif // PI_VER == 2

    GPAREN0 |= (1 << CLK); // Enable rising edge detect on CLK
    GPAFEN0 &= (1 << CLK);
}

inline void data_out(void) {
    // Set pins 2-9 as output. (D0 - D7)
    #if PI_VER == 1
    GPFSEL0 |= FUNSEL(D0, 1) | FUNSEL(D1, 1) | FUNSEL(D2, 1);
    GPFSEL1 |= FUNSEL(D3, 1);
    GPFSEL2 |= FUNSEL(D4, 1) | FUNSEL(D5, 1) | FUNSEL(D6, 1) | FUNSEL(D7, 1);
    #endif // PI_VER == 1

    #if PI_VER == 2
    GPFSEL0 |=  FUNSEL(D0, 1) | FUNSEL(D1, 1) | FUNSEL(D2, 1) | FUNSEL(D3, 1) |
                FUNSEL(D4, 1) | FUNSEL(D5, 1) | FUNSEL(D6, 1) | FUNSEL(D7, 1);
    #endif // PI_VER == 2
    GPAFEN0 |= (1 << CLK); // Enable falling edge detect on CLK
    GPAREN0 &= (1 << CLK);
}

inline void initpins(void) {
    // Set pins CLK, CS1, RST, CS2 as input.
    #if PI_VER == 1
    GPFSEL1 &= ~(FUNSEL(CLK, 7) | FUNSEL(CS2, 7) | FUNSEL(RST, 7));
    GPFSEL2 &= ~(FUNSEL(CS1, 7));

    GPFSEL1 &= ~(FUNSEL(LED, 7)); // Set status LED as output.
    GPFSEL1 |= FUNSEL(LED, 1); // DEBUG, really.
    #endif

    #if PI_VER == 2
    GPFSEL1 &= ~(FUNSEL(CLK, 7) | FUNSEL(CS1, 7));
    GPFSEL2 &= ~(FUNSEL(RST, 7) | FUNSEL(CS2, 7));
    #endif

    GPSET0 = 1 << LED; // Clear the LED. Active low.

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

inline uint8_t pinevent(unsigned pin) {
    if (GPEDS0 & (1 << pin)) {
        GPEDS0 = (1 << pin);
        return 1;
    }
    return 0;
    // return (GPEDS0 &= (1 << pin)); // Would this work? Would it be faster?
}
