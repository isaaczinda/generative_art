// Because we want to work with integers and not floats, all velocities and
// posititions are measured in 1/10th pixel units


#define NUM_PARTICLES 20
#define SPEED_FACTOR 1
// units per pixel
#define UPS 128
// max velocity (in units / frame)
// note that this is not pixels / frame
#define MAX_VEL_UNITS 64

// how much velocity changes by each frame in y direction
#define GRAVITY 2

typedef struct {
    int x_pos;
    int y_pos;
    int x_vel;
    int y_vel;
    color col;
    int active;
} particle;


particle particles[NUM_PARTICLES];

/*

*/
int randrange(int min, int max) {
    return (rand() % (max-min)) + min;
}

/*

*/
particle spawn_particle() {
    int rand_x_vel = randrange(-MAX_VEL_UNITS, MAX_VEL_UNITS);
    int rand_y_vel = randrange(-MAX_VEL_UNITS, MAX_VEL_UNITS);

    int rand_x_pos = randrange(0, (WIDTH * UPS));
    int rand_y_pos = randrange(0, (WIDTH * UPS));

    color rand_color = (color){rand() % 256, rand() % 256, rand() % 256, 255};

    return (particle){rand_x_pos, rand_y_pos, // position
        rand_x_vel, rand_y_vel,
        rand_color, // color
        TRUE}; // active?
}

particle get_closest_particle(int x_pos, int y_pos) {
    particle closest_particle;
    // larger than max distance
    int closest_particle_distance = 100 * UPS*20;

    for (int i = 0; i < NUM_PARTICLES; i++) {
        int distance = (int) sqrt(pow(x_pos - particles[i].x_pos, 2) + pow(y_pos - particles[i].y_pos, 2));

        // this will make the pattern look worse, but will save LOTS of cycles
        // int distance = abs(x_pos - particles[i].x_pos) + abs(y_pos - particles[i].y_pos);

        if (distance < closest_particle_distance) {
            closest_particle_distance = distance;
            closest_particle = particles[i];
        }
    }

    return closest_particle;
}

void particles_animation() {
    // initialize all particles to
    for (int s = 0; s < NUM_PARTICLES; s++) {
        particles[s].active = FALSE;
    }

    for (int i = 0; i < 1000; i++) {
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

                screen[y][x][RED] = pixel_color.red;
                screen[y][x][GREEN] = pixel_color.green;
                screen[y][x][BLUE] = pixel_color.blue;
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
