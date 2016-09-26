/* Copyright 2016 Kitlith                                                *
 * Licensed under GPLv3 or any later version.                            *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdint.h>

#define PI_VER 2

#include "registers.h"
#include "header.h"

const uint8_t chipid[4] = {'H', 'E', 'L', 'P'};

extern void (*c_irq_handler)(void);
volatile uint8_t cs1_triggered;

uint32_t bytes;

volatile uint8_t *cmdpos;
uint8_t cmdbuf[8];

const uint8_t *output_buffer;
uint32_t buffer_size;
volatile const uint8_t *outpos;

void read_irq(void);
void write_irq(void);
void null_write_irq(void);

extern void enable_irq(void);

#define FUNSEL(pin,func) (func << ((pin % 10) * 3))

int pimain(void) {
    // Pre-init stuff.
    cs1_triggered = 0;
    cmdpos = cmdbuf;

    #if PI_VER == 1
    // Enable input on data pins, CLK, and CS1.
    GPFSEL0 &= ~(FUNSEL(D0,7) | FUNSEL(D1,7) | FUNSEL(D2,7));
    GPFSEL1 &= ~(FUNSEL(D3,7) | FUNSEL(CLK,7));
    GPFSEL2 &= ~(FUNSEL(D4,7) | FUNSEL(D5,7) | FUNSEL(D6,7)
                |FUNSEL(D7,7) | FUNSEL(CS1,7));
    #elif PI_VER == 2
    // Enable input on data pins.
    GPFSEL0 &= ~(FUNSEL(D0,7) | FUNSEL(D1,7) | FUNSEL(D2,7) | FUNSEL(D3,7)
                |FUNSEL(D4,7) | FUNSEL(D5,7) | FUNSEL(D6,7) | FUNSEL(D7,7));

    // Enable input on CLK and CS1.
    GPFSEL1 &= ~(FUNSEL(CLK,7) | FUNSEL(CS1,7));
    #endif

    // Enable interrupts on CLK and CS1;
    GPAFEN0 &= ~((1 << CLK) | (1 << CS1));
    GPAREN0 |= (1 << CLK) | (1 << CS1);
    // c_irq_handler = read_irq;
    // // Blast it, just do everything labled 'gpio_int[n]'
    // IRQ_ENABLE2 = (1 << (52 - 32)) | (1 << (51 - 32)) | (1 << (50 - 32)) | (1 << (49 - 32));
    // enable_irq();

    while (1) {
        while (cmdbuf+8 > cmdpos) { read_irq(); } // Read Command
        #if PI_VER == 1
        // Switch to output on data pins.
        GPFSEL0 |= FUNSEL(D0,1) | FUNSEL(D1,1) | FUNSEL(D2,1);
        GPFSEL1 |= FUNSEL(D3,1);
        GPFSEL2 |= FUNSEL(D4,1) | FUNSEL(D5,1) | FUNSEL(D6,1) | FUNSEL(D7,1);
        #elif PI_VER == 2
        // Switch to output on data pins.
        GPFSEL0 |= FUNSEL(D0,1) | FUNSEL(D1,1) | FUNSEL(D2,1) | FUNSEL(D3,1)
                |  FUNSEL(D4,1) | FUNSEL(D5,1) | FUNSEL(D6,1) | FUNSEL(D7,1);
        #endif
        //Switch to falling edge interrupt for clock.
        GPAREN0 &= ~(1 << CLK);
        GPAFEN0 |= 1 << CLK;
        // Switch interrupt handler.
        // c_irq_handler = write_irq;

        // Set variables for writing data
        switch (cmdbuf[0]) {
            case 0x90: // Chip ID
                outpos = output_buffer = chipid;
                buffer_size = 4;
                break;
            case 0x00:
                outpos = output_buffer = header;
                buffer_size = header_size;
                break;
            case 0x9F:
                // c_irq_handler = null_write_irq;
                GPSET0 = 0xFF << D0;
                outpos = output_buffer = 0;
                buffer_size = 1;
                break;
        }

        while (1) { // Output data
            write_irq();
            if (output_buffer + buffer_size <= outpos) {
                outpos = output_buffer;
            }
            if (cs1_triggered) {
                break;
            }
        }
        #if PI_VER == 1
        // Enable input on data pins, CLK, and CS1.
        GPFSEL0 &= ~(FUNSEL(D0,7) | FUNSEL(D1,7) | FUNSEL(D2,7));
        GPFSEL1 &= ~FUNSEL(D3,7);
        GPFSEL2 &= ~(FUNSEL(D4,7) | FUNSEL(D5,7) | FUNSEL(D6,7) | FUNSEL(D7,7));
        #elif PI_VER == 2
        // Enable input on data pins.
        GPFSEL0 &= ~(FUNSEL(D0,7) | FUNSEL(D1,7) | FUNSEL(D2,7) | FUNSEL(D3,7)
                    |FUNSEL(D4,7) | FUNSEL(D5,7) | FUNSEL(D6,7) | FUNSEL(D7,7));
        #endif
        // Switch to rising edge interrupt for clock
        GPAFEN0 &= ~(1 << CLK);
        GPAREN0 |= 1 << CLK;
        c_irq_handler = read_irq;
        // Reset variables for reading command.
        cmdpos = cmdbuf;
        cs1_triggered = 0;
    }
    return 0;
}

// These aren't handling the reset line... Or CS2...
inline void read_irq(void) {
    *cmdpos++ = GPLEV0 >> D0; // Nice and simple, right?
    GPEDS0 = 1 << CLK; // TODO: What if it's actually CS1?
}

inline void write_irq(void) {
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

inline void null_write_irq(void) {
    if (GPEDS0 & (1 << CS1)) {
        // Notify main loop.
        cs1_triggered = 1;
    }
    GPEDS0 = (1 << CLK) | (1 << CS1);
    return;
}
