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

#define PI_VER 1
#define EXPANDED_GPIO

#include "registers.h"
#include "header.h"

const uint8_t chipid[4] = {'H', 'E', 'L', 'P'};

uint8_t invalid_buf[4];

uint8_t *cmdpos;
uint8_t cmdbuf[8];

const uint8_t *output_buffer;
uint32_t buffer_size;
const uint8_t *outpos;

#define FUNSEL(pin,func) (func << ((pin % 10) * 3))

int pimain(void) {
    // Pre-init stuff.
    cmdpos = cmdbuf;
    invalid_buf[1] = 0x90;
    invalid_buf[2] = 0x09;
    invalid_buf[3] = 0xFF;

    // All of these blocks rely on constant propegation.
    // If that doesn't work, this will be HORRIBLY slow.
    // TODO: encapsulate these blocks somehow, so the main path doesn't need to think about it.

    #ifdef EXPANDED_GPIO
    // Enable input on data pins.
    GPFSEL0 &= ~(FUNSEL(D0,7) | FUNSEL(D1,7) | FUNSEL(D2,7) | FUNSEL(D3,7)
                |FUNSEL(D4,7) | FUNSEL(D5,7) | FUNSEL(D6,7) | FUNSEL(D7,7));

    // Enable input on CLK and CS1.
    GPFSEL1 &= ~(FUNSEL(CLK,7) | FUNSEL(CS1,7));
    #else
    // Enable input on data pins, CLK, and CS1.
    GPFSEL0 &= ~(FUNSEL(D0,7) | FUNSEL(D1,7) | FUNSEL(D2,7));
    GPFSEL1 &= ~(FUNSEL(D3,7) | FUNSEL(CLK,7));
    GPFSEL2 &= ~(FUNSEL(D4,7) | FUNSEL(D5,7) | FUNSEL(D6,7)
                |FUNSEL(D7,7) | FUNSEL(CS1,7));
    #endif

    // Enable rising edge "interrupts" on CLK and CS1;
    GPAFEN0 &= ~((1 << CLK) | (1 << CS1));
    GPAREN0 |= (1 << CLK) | (1 << CS1);

    while (1) { // Main loop
        while (cmdbuf+8 > cmdpos) {
            while (~GPEDS0 & (1 << CLK)) { // WAIT-LOOP
                // TODO: What if it's actually CS1?
            }

            *cmdpos++ = GPLEV0 >> D0; // Nice and simple, right?
            GPEDS0 = 1 << CLK;
        }

        // Switch to output on data pins.
        #ifdef EXPANDED_GPIO
        GPFSEL0 |= FUNSEL(D0,1) | FUNSEL(D1,1) | FUNSEL(D2,1) | FUNSEL(D3,1)
                |  FUNSEL(D4,1) | FUNSEL(D5,1) | FUNSEL(D6,1) | FUNSEL(D7,1);
        #else
        GPFSEL0 |= FUNSEL(D0,1) | FUNSEL(D1,1) | FUNSEL(D2,1);
        GPFSEL1 |= FUNSEL(D3,1);
        GPFSEL2 |= FUNSEL(D4,1) | FUNSEL(D5,1) | FUNSEL(D6,1) | FUNSEL(D7,1);
        #endif

        // Switch to falling edge "interrupt" for clock.
        GPAREN0 &= ~(1 << CLK);
        GPAFEN0 |= 1 << CLK;

        // Set variables for writing data
        switch (cmdbuf[0]) {
            case 0x90: // Read Chip ID
                outpos = output_buffer = chipid;
                buffer_size = 4;
                break;
            case 0x00: // Read Header
                outpos = output_buffer = header;
                buffer_size = header_size;
                break;
            case 0x9F: // Dummy
                GPSET0 = 0xFF << D0;
                output_buffer = 0;
                break;
            default: // Help debug if something has possibly gone horribly wrong.
                invalid_buf[0] = cmdbuf[0];
                outpos = output_buffer = invalid_buf;
                buffer_size = 4;
                break;
        }

        while (1) { // Output data
            {
                // Is there actually a good point to making this a variable?
                // AKA, is this premature self-optimization?
                // If it isn't, are there other places I should do it?
                register uint32_t gpio_status;
                do { // WAIT-LOOP
                    gpio_status = GPEDS0 & ((1 << CLK) | (1 << CS1));
                } while (!gpio_status);

                if (gpio_status & (1 << CS1)) {
                    GPEDS0 = 1 << CS1;
                    break;
                }
            }

            if (output_buffer) {
                register uint8_t value = *outpos++;

                #ifdef EXPANDED_GPIO
                GPSET0 = value << D0;
                GPCLR0 = ~value << D0; // Do I need to optimize this somehow?
                #else
                #error "Non-expanded GPIO! this case is slower and more complicated. Not dealing with it yet."
                #endif

                if (output_buffer + buffer_size <= outpos) {
                    outpos = output_buffer;
                }
            }

            GPEDS0 = 1 << CLK;
        }

        // Switch to input on data pins.
        #ifdef EXPANDED_GPIO
        GPFSEL0 &= ~(FUNSEL(D0,7) | FUNSEL(D1,7) | FUNSEL(D2,7) | FUNSEL(D3,7)
                    |FUNSEL(D4,7) | FUNSEL(D5,7) | FUNSEL(D6,7) | FUNSEL(D7,7));
        #else
        GPFSEL0 &= ~(FUNSEL(D0,7) | FUNSEL(D1,7) | FUNSEL(D2,7));
        GPFSEL1 &= ~FUNSEL(D3,7);
        GPFSEL2 &= ~(FUNSEL(D4,7) | FUNSEL(D5,7) | FUNSEL(D6,7) | FUNSEL(D7,7));
        #endif
        // Switch to rising edge "interrupt" for clock
        GPAFEN0 &= ~(1 << CLK);
        GPAREN0 |= 1 << CLK;

        // Reset variables for reading command.
        cmdpos = cmdbuf;
    }
    return 0;
}
