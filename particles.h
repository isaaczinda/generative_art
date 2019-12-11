/* 
particles.h

Because we want to work with integers and not floats, all velocities and
posititions are measured in 1/1024th pixel units.
*/

#include "screen.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_PARTICLES 20
#define SPEED_FACTOR 1
// units per pixel
#define UPS 1024
// max velocity (in units / frame)
// note that this is not pixels / frame
#define MAX_VEL_UNITS 50

// how much velocity changes by each frame in y direction
#define GRAVITY 10
#define FALSE 0
#define TRUE 1

typedef struct {
    int x_pos;
    int y_pos;
    int x_vel;
    int y_vel;
    color col;
    int active;
} particle;


particle particles[NUM_PARTICLES];

int randrange(int min, int max) {
    return (rand() % (max-min)) + min;
}
particle spawn_particle() {
    int rand_x_vel = randrange(-MAX_VEL_UNITS, MAX_VEL_UNITS);
    int rand_y_vel = randrange(-MAX_VEL_UNITS, MAX_VEL_UNITS);

    int rand_x_pos = randrange(0, (WIDTH * UPS));
    int rand_y_pos = randrange(0, (WIDTH * UPS));

    color rand_color = (color){(byte) (rand() % 256), (byte) (rand() % 256), (byte) (rand() % 256), 255};

    return (particle){rand_x_pos, rand_y_pos, // position
        rand_x_vel, rand_y_vel,
        rand_color, // color
        TRUE}; // active?
}

particle get_closest_particle(int x_pos, int y_pos) {
    particle closest_particle;
    // larger than max distance
    unsigned int closest_particle_distance = 4294967295; // max value of int

    for (int i = 0; i < NUM_PARTICLES; i++) {
        int distance = (x_pos - particles[i].x_pos) * (x_pos - particles[i].x_pos) +
											 (y_pos - particles[i].y_pos) * (y_pos - particles[i].y_pos);

        // this will make the pattern look worse, but will save LOTS of cycles
        // int distance = abs(x_pos - particles[i].x_pos) + abs(y_pos - particles[i].y_pos);

        if (distance < closest_particle_distance) {
            closest_particle_distance = distance;
            closest_particle = particles[i];
        }
    }

    return closest_particle;
}

float p;

void particles_animation() {
    // initialize all particles to
    for (int s = 0; s < NUM_PARTICLES; s++) {
        particles[s].active = FALSE;
    }

    while (TRUE) {
				// for (int i = 0; i < 10000; i++) { // 100000 2 fps
				//	p = sqrt(i); 
				//}
			
        for (int s = 0; s < NUM_PARTICLES; s++) {
            // add this particle if it has fallen off of the screen
            if (particles[s].active == FALSE) {
                // spawn a new particle
                particles[s] = spawn_particle();
            }

            // move particles
            particles[s].x_pos += particles[s].x_vel;
            particles[s].y_pos += particles[s].y_vel;

            particles[s].y_vel += GRAVITY;
        }

        // color pixels based on which particle is closest
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                color pixel_color = get_closest_particle(x*UPS, y*UPS).col;
                set_screen_color(y, x, pixel_color);
            }
        }

        send_frame();

        // remove particles which have fallen off of the screen
        for (int s = 0; s < NUM_PARTICLES; s++) {
            if (particles[s].x_pos >= WIDTH * UPS || particles[s].x_pos < 0 ||
                particles[s].y_pos >= HEIGHT * UPS || particles[s].y_pos < 0)
            {
                particles[s].active = FALSE;

            }
        }
    }
}
