#include <stdint.h>

uint32_t bytes;
extern uint8_t CS1;
extern uint8_t CLK;

int pimain(void) {
    // init
    while (1) {
        while (1) { // Read command
            while (CLK); // Set by interrupt
            CLK = 1;
            // Do stuff.
            if (bytes >= 8) { // Finished reading command
                // Get ready to Output
                break;
            }
        }
        while (1) { // Output data
            while (CLK) {
                if (CS1) break;
            } // Set by interrupt
            if (CS1) { // Set by interrupt
                CS1 = 0;
                // Get ready to input
                break; // Do this checking in a better place?
            }
            // Do stuff
        }
    }
    return 0;
}
