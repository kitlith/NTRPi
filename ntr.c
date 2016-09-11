#include <stdint.h>

#define INPUT_STATE 0
#define OUTPUT_STATE 1

uint8_t state;

int pimain(void) {
    // init
    state = INPUT_STATE;
    while(1) {
        if (state) {
            // Do stuff.
            state = OUTPUT_STATE;
        } else {
            // Do stuff.
            state = INPUT_STATE;
        }
    }
}
