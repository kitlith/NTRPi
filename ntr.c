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

int pimain(void) {
    // Pre-init stuff.
    cs1_triggered = 0;
    cmdpos = cmdbuf;
    // Enable input on data pins.
    GPFSEL0 &= ~((7 << (D0*3)) | (7 << (D1*3)) | (7 << (D2*3)) | (7 << (D3*3))
                |(7 << (D4*3)) | (7 << (D5*3)) | (7 << (D6*3)) | (7 << (D7*3)));

    // Enable input on CLK and CS1.
    GPFSEL1 &= ~((7 << ((CLK-10)*3)) | (7 << ((CS1-10)*3)));

    // Enable interrupts on CLK and CS1;
    GPAFEN0 &= ~((1 << CLK) | (1 << CS1));
    GPAREN0 |= (1 << CLK) | (1 << CS1);
    c_irq_handler = read_irq;
    IRQ_ENABLE2 = (1 << (52 - 32)); // This is a GUESS.
    enable_irq();

    while (1) {
        while (cmdbuf+8 > cmdpos) { // Read command
            ;
        }
        // Switch to output on data pins.
        GPFSEL0 |= (1 << (D0*3)) | (1 << (D1*3)) | (1 << (D2*3)) | (1 << (D3*3))
                |  (1 << (D4*3)) | (1 << (D5*3)) | (1 << (D6*3)) | (1 << (D7*3));
        //Switch to falling edge interrupt for clock.
        GPAREN0 &= ~(1 << CLK);
        GPAFEN0 |= 1 << CLK;
        // Switch interrupt handler.
        c_irq_handler = write_irq;

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
                c_irq_handler = null_write_irq;
                GPSET0 = 0xFF << D0;
                outpos = output_buffer = 0;
                buffer_size = 1;
                break;
        }

        while (1) { // Output data
            if (output_buffer + buffer_size <= outpos) {
                outpos = output_buffer;
            }
            if (cs1_triggered) {
                break;
            }
        }
        GPFSEL0 &= ~((7 << (D0*3)) | (7 << (D1*3)) | (7 << (D2*3)) | (7 << (D3*3))
                    |(7 << (D4*3)) | (7 << (D5*3)) | (7 << (D6*3)) | (7 << (D7*3)));
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
void read_irq(void) {
    *cmdpos++ = GPLEV0 >> D0; // Nice and simple, right?
    GPEDS0 = 1 << CLK; // TODO: What if it's actually CS1?
}

void write_irq(void) {
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

void null_write_irq(void) {
    if (GPEDS0 & (1 << CS1)) {
        // Notify main loop.
        cs1_triggered = 1;
    }
    GPEDS0 = (1 << CLK) | (1 << CS1);
    return;
}
