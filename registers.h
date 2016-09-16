#pragma once

#include <stdint.h>
// #define PI1
#define PI2

#ifdef PI1
#define PERIPHERAL_BASE 0x20000000 // Pi1

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

#define CLK 13 //defined for hedge's pi
#define CS1 19 //defined for hedge's pi
#define RST 20
#define CS2 21
#endif

#define FUNSEL(pin,func) (func << ((pin % 10) * 3))

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

#define GPPUD (*(volatile uint32_t*)(GPIO_BASE + 0x94))
#define GPPUDCLK0 (*(volatile uint32_t*)(GPIO_BASE + 0x98))

// End GPIO Registers

#define TIMER_BASE (PERIPHERAL_BASE + 0xB400)

#define ARM_TIMER_LOD (*(volatile uint32_t*)(TIMER_BASE + 0x00))
#define ARM_TIMER_VAL (*(volatile uint32_t*)(TIMER_BASE + 0x04))
#define ARM_TIMER_CTL (*(volatile uint32_t*)(TIMER_BASE + 0x08))
#define ARM_TIMER_CLI (*(volatile uint32_t*)(TIMER_BASE + 0x0C))
#define ARM_TIMER_RIS (*(volatile uint32_t*)(TIMER_BASE + 0x10))
#define ARM_TIMER_MIS (*(volatile uint32_t*)(TIMER_BASE + 0x14))
#define ARM_TIMER_RLD (*(volatile uint32_t*)(TIMER_BASE + 0x18))
#define ARM_TIMER_DIV (*(volatile uint32_t*)(TIMER_BASE + 0x1C))
#define ARM_TIMER_CNT (*(volatile uint32_t*)(TIMER_BASE + 0x20))

#define SYSTIMERCLO (*(volatile uint32_t*)0x3F003004) // TODO: figure out where this goes.

// End Timer Registers

#define AUX_BASE (PERIPHERAL_BASE + 0x00215000)

#define AUX_ENABLES (*(volatile uint32_t*)(AUX_BASE + 0x04))
#define AUX_MU_IO_REG (*(volatile uint32_t*)(AUX_BASE + 0x40))
#define AUX_MU_IER_REG (*(volatile uint32_t*)(AUX_BASE + 0x44))
#define AUX_MU_IIR_REG (*(volatile uint32_t*)(AUX_BASE + 0x48))
#define AUX_MU_LCR_REG (*(volatile uint32_t*)(AUX_BASE + 0x4C))
#define AUX_MU_MCR_REG (*(volatile uint32_t*)(AUX_BASE + 0x50))
#define AUX_MU_LSR_REG (*(volatile uint32_t*)(AUX_BASE + 0x54))
#define AUX_MU_MSR_REG (*(volatile uint32_t*)(AUX_BASE + 0x58))
#define AUX_MU_SCRATCH (*(volatile uint32_t*)(AUX_BASE + 0x5C))
#define AUX_MU_CNTL_REG (*(volatile uint32_t*)(AUX_BASE + 0x60))
#define AUX_MU_STAT_REG (*(volatile uint32_t*)(AUX_BASE + 0x64))
#define AUX_MU_BAUD_REG (*(volatile uint32_t*)(AUX_BASE + 0x68))
