#include <stdint.h>

uint32_t bytes;
extern uint8_t CS1;

int pimain(void) {
    // init
    while (1) {
        while (1) { // Read command
            // Do stuff.
            if (bytes >= 8) { // Finished reading command
                // Get ready to Output
                break;
            }
        }
        while (1) { // Output data
            if (CS1) { // Set by interrupt
                // Get ready to input
                break;
            }
            // Do stuff
        }
    }
    return 0;
}
