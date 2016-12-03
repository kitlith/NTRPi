#pragma once
#include <stdint.h>

#ifdef EXPANDED_GPIO

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

#else

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

#endif

#if PI_VER == 1
#define PERIPHERAL_BASE 0x20000000 // Pi1
#elif PI_VER == 2
#define PERIPHERAL_BASE 0x3F000000 // Pi2
#else
#error "PI_VER needs to be defined as either 1 or 2!"
#endif

#define IRQ_BASE (PERIPHERAL_BASE + 0x0000B000)

#define IRQ_BASIC (*(volatile uint32_t*)(IRQ_BASE + 0x200))
#define IRQ_PEND1 (*(volatile uint32_t*)(IRQ_BASE + 0x204))
#define IRQ_PEND2 (*(volatile uint32_t*)(IRQ_BASE + 0x208))

#define IRQ_FIQ_CONTROL (*(volatile uint32_t*)(IRQ_BASE + 0x20C))

#define IRQ_ENABLE1 (*(volatile uint32_t*)(IRQ_BASE + 0x210))
#define IRQ_ENABLE2 (*(volatile uint32_t*)(IRQ_BASE + 0x214))
#define IRQ_ENABLE_BASIC (*(volatile uint32_t*)(IRQ_BASE + 0x218))

#define IRQ_DISABLE1 (*(volatile uint32_t*)(IRQ_BASE + 0x21C))
#define IRQ_DISABLE2 (*(volatile uint32_t*)(IRQ_BASE + 0x220))
#define IRQ_DISABLE_BASIC (*(volatile uint32_t*)(IRQ_BASE + 0x224))

// End IRQ registers

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

#define GPAREN0 (*(volatile uint32_t*)(GPIO_BASE + 0x7C))
#define GPAFEN0 (*(volatile uint32_t*)(GPIO_BASE + 0x88))

// End GPIO Registers
