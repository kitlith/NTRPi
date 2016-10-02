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

void data_in(void);
void data_out(void);
void initpins(void);
void ntr_sendbyte(uint8_t byte);
uint8_t ntr_readbyte(void);
void debug(void);

inline uint8_t pinevent(unsigned pin) {
    if (GPEDS0 & (1 << pin)) {
        GPEDS0 = (1 << pin);
        return 1;
    }
    return 0;
    // return (GPEDS0 &= (1 << pin)); // Would this work? Would it be faster?
}
