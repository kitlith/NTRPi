#pragma once

#include <stdint.h>
#define PI1
// #define PI2

#ifdef PI1
#define PERIPHERAL_BASE 0x20000000 // Pi1

#define D0 7
#define D1 8
#define D2 9
#define D3 10
#define D4 21
#define D5 22
#define D6 23
#define D7 24

#define CLK 11
#define CS1 25
#define RST 0
#define CS2 1

#endif

#ifdef PI2
#define PERIPHERAL_BASE 0x3F000000 // Pi2

#define D0 2
#define D1 3
#define D2 4
#define D3 5
#define D4 6
#define D5 7
#define D6 8
#define D7 9

#define CLK 10
#define CS1 11
#define RST 12
#define CS2 13
#endif

#define GPIO_BASE (PERIPHERAL_BASE + 0x00200000)

#define GPFSEL0 (*(volatile uint32_t*)(GPIO_BASE + 0x00))
#define GPFSEL1 (*(volatile uint32_t*)(GPIO_BASE + 0x04))
#define GPFSEL2 (*(volatile uint32_t*)(GPIO_BASE + 0x08))

#define GPSET0  (*(volatile uint32_t*)(GPIO_BASE + 0x1C))

#define GPCLR0  (*(volatile uint32_t*)(GPIO_BASE + 0x28))

#define GPLEV0  (*(volatile uint32_t*)(GPIO_BASE + 0x34))

#define GPEDS0  (*(volatile uint32_t*)(GPIO_BASE + 0x40))

#define GPREN0  (*(volatile uint32_t*)(GPIO_BASE + 0x4C))
#define GPFEN0  (*(volatile uint32_t*)(GPIO_BASE + 0x58))

void data_in(void);
void data_out(void);
void initpins(void);
void ntr_sendbyte(const uint8_t byte);

inline uint8_t pinevent(unsigned pin) {
    if (GPEDS0 & (1 << pin)) {
        GPEDS0 = (1 << pin);
        return 1;
    }
    return 0;
}
