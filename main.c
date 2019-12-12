/*
main.c
*/

// outputs frame to local files when we are testing
// outputs frame to strips over SPI when we aren't testing
#define TESTING 0

#include "particles.h"

// need to redefine this sometimes because ATSAM's version of math.h doesn't
// include this constant
#ifndef M_PI
#define M_PI 3.1415
#endif

// to simplify boolean logic
#define TRUE 1
#define FALSE 0

typedef unsigned char byte;


int main() {
	#if TESTING == 0
    samInit();
    pioInit();
    spiInit(72, 0, 1);
    pioPinMode(CS_PIN, PIO_OUTPUT);
    pioPinMode(FLUSHING_PIN, PIO_INPUT);
	#endif

    particles_animation();
}
