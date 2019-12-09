// outputs frame to local files when we are testing
// outputs frame to strips over SPI when we aren't testing
#define TESTING 1

#include <math.h>
#include <stdio.h>
#include <stdlib.h>


// only include os libraries when we're testing
#if TESTING == 1

#include <sys/stat.h>
#include <sys/types.h>
#endif

#if TESTING == 0
#include "SAM4S4B_libs/SAM4S4B.h"
#endif

// number of cols
#define WIDTH 25
// number of rows
#define HEIGHT 24

#define PIXEL_SIZE 3

// need to redefine this sometimes because ATSAM's version of math.h doesn't
// include this constant
#ifndef M_PI
#define M_PI 3.1415
#endif

#define RED 0
#define GREEN 1
#define BLUE 2

// to simplify boolean logic
#define TRUE 1
#define FALSE 0

#define FLASH_CONSTANT 70 // 0b10101010

#define FLUSHING_PIN PIO_PA27
#define CS_PIN PIO_PA28


typedef unsigned char byte;

byte screen[HEIGHT][WIDTH][PIXEL_SIZE];


typedef struct {
  byte red;
  byte green;
  byte blue;
  byte alpha;
} color;

typedef struct {
    float x_pos;
    float y_pos;
    float x_vel;
    float y_vel;
    color col;
    int active;
} particle;

void render_frame(void);
void send_frame(void);
void clear_frame(void);
void draw_circle(double, int, int, int, color);
void draw_background(double, double, double, double, color);

#define NUM_PARTICLES 20
#define FRAME_TIME .01666
#define SPEED_FACTOR 10

particle particles[NUM_PARTICLES];

particle spawn_particle() {
    float rand_x_vel = ((float)(rand() % 20) - 10) / 5.0 * SPEED_FACTOR; // from -2 to 2
    float rand_y_vel = ((float)(rand() % 20) - 10) / 5.0 * SPEED_FACTOR; // from -2 to 2

    float rand_x_pos = rand() % WIDTH;
    float rand_y_pos = rand() % HEIGHT;

    color rand_color = (color){rand() % 256, rand() % 256, rand() % 256, 255};

    return (particle){rand_x_pos, rand_y_pos, // position
        rand_x_vel, rand_y_vel,
        rand_color, // color
        TRUE}; // active?
}

particle get_closest_particle(float x_pos, float y_pos) {
    particle closest_particle;
    float closest_particle_distance = 100; // larger than max distance

    for (int i = 0; i < NUM_PARTICLES; i++) {
        float distance = sqrt(pow(x_pos - particles[i].x_pos, 2) + pow(y_pos - particles[i].y_pos, 2));

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
            particles[s].x_pos += particles[s].x_vel * FRAME_TIME;
            particles[s].y_pos += particles[s].y_vel * FRAME_TIME;
        }

        // color pixels based on which particle is closest
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                color pixel_color = get_closest_particle(x, y).col;

                screen[y][x][RED] = pixel_color.red;
                screen[y][x][GREEN] = pixel_color.green;
                screen[y][x][BLUE] = pixel_color.blue;
            }
        }

        send_frame();

        // remove particles which have fallen off of the screen
        for (int s = 0; s < NUM_PARTICLES; s++) {
            if (particles[s].x_pos >= WIDTH || particles[s].x_pos < 0 ||
                particles[s].y_pos >= HEIGHT || particles[s].y_pos < 0)
            {
                particles[s].active = FALSE;

            }
        }
    }
}

void test_animation() {
	byte color = 0;

	while (TRUE) {
		// color pixels based on which particle is closest
		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				screen[y][x][RED] = 255;
				screen[y][x][GREEN] = 0;
				screen[y][x][BLUE] = 0;
			}
		}

		color += 1;

		send_frame();
	}
}

void shifting_background() {
    for (int i = 0; i < 1000; i++) {
        clear_frame();

        // frequency ranges from from .05 to .15
        double freq_x_1 = .05 * sin(2 * M_PI * i * .0245) + .08;
        double freq_y_1 = .05 * sin(2 * M_PI * i * .0394) + .08;

        double offset_x_1 = 1.4 * sin(2 * M_PI * i * .0494);
        double offset_y_1 = .8 * sin(2 * M_PI * i * .0394);


        double freq_x_2 = .05 * sin(2 * M_PI * i * .00245) + .08;
        double freq_y_2 = .05 * sin(2 * M_PI * i * .01345) + .08;

        double offset_x_2 = 1.2 * sin(2 * M_PI * i * .0294);
        double offset_y_2 = .4 * sin(2 * M_PI * i * .0194);


        int j = i + 77;
        int k = i + 300;
        draw_background(freq_x_1, freq_y_1, offset_x_1, offset_y_1, (color){j % 256, (2 * j) % 256, (3 * j) % 256, 255});
        draw_background(freq_x_2, freq_y_2, offset_x_2, offset_y_2, (color){k % 256, (2 * k) % 256, (3 * k) % 256, 100});
        draw_background(freq_y_2, freq_x_1 * .9, offset_y_2, offset_x_1, (color){i % 256, (2 * i) % 256, (3 * i) % 256, 70});
        send_frame();
    }
}

void static_circles() {
  for (int i = 0; i < 1000; i++) {
    int r1 = i % 37;
    int r2 = i % 29;
    int r3 = i % 47;
    int r4 = i % 23;
    int r5 = i % 17;

    color c1 = { 255, (i-100)%256, 0, 255 };
    color c2 = { 0, 255, i%256, 255 };
    color c3 = { (i+200)%256, 0, 255, 255 };
    color c4 = { 255, (i+100)%256, 0, 255 };
    color c5 = { 0, 255, (i+200)%256, 255 };

    clear_frame();

    draw_circle(r1, 200, 2, 2, c1);
    draw_circle(r1 + 1, 200, 2, 2, c1);

    draw_circle(r2, 400, 6, 12, c2);
    draw_circle(r2 + 4, 400, 6, 12, c2);

    draw_circle(r3, 400, 15, 22, c3);

    draw_circle(r4, 400, 23, 6, c4);
    draw_circle(r5, 400, 13, 17, c5);

	send_frame();
  }
}

int main() {
	#if TESTING == 0
    samInit();
    pioInit();
    spiInit(MCK_FREQ/244000, 0, 1);
    pioPinMode(CS_PIN, PIO_OUTPUT);
    pioPinMode(FLUSHING_PIN, PIO_INPUT);
	#endif

    // shifting_background(screen);
    particles_animation();
}

void draw_background(double freq_x, double freq_y, double offset_x, double offset_y, color bg_color) {
    double opacity = bg_color.alpha / 255.0;

    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {

            // from 0 to 1
            double brightness = (( sin(x * 2 * M_PI * freq_x - offset_x) * cos(y * 2 * M_PI * freq_y - offset_y) ) / 2.0) + .5;

            screen[y][x][RED] = (1-opacity) * screen[y][x][RED] + opacity *  brightness * bg_color.red;
            screen[y][x][BLUE] = (1-opacity) * screen[y][x][BLUE] + opacity * brightness * bg_color.blue;
            screen[y][x][GREEN] = (1-opacity) * screen[y][x][GREEN] + opacity * brightness * bg_color.green;
        }
    }
}

// mode = 1: opaque
// mode = 0: transparent
void draw_circle(double radius,
                 int num_points, int center_x, int center_y,
                 color circle_color)
{
  double delta_angle = 2 * M_PI / (double)num_points;

  for (int i = 0; i < num_points; i++) {
    double angle = delta_angle * i;

    int x = round(cos(angle) * radius + center_x);
    int y = round(sin(angle) * radius + center_y);

    double decimal_alpha = ((double)circle_color.alpha) / 255;

    // printf("alpha: %f, divided: %f\n", (double)circle_color.alpha, decimal_alpha);

    // if the point is in bounds...
    if (x < WIDTH && x >= 0 && y < HEIGHT && y >= 0) {
      screen[y][x][RED] = floor(circle_color.red * decimal_alpha + screen[y][x][RED] * (1-decimal_alpha));
      screen[y][x][GREEN] = floor(circle_color.green * decimal_alpha + screen[y][x][GREEN] * (1-decimal_alpha));
      screen[y][x][BLUE] = floor(circle_color.blue * decimal_alpha + screen[y][x][BLUE] * (1-decimal_alpha));
    }
  }
}

void clear_frame() {
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      screen[y][x][RED] = 0;
      screen[y][x][GREEN] = 0;
      screen[y][x][BLUE] = 0;
    }
  }
}


#if TESTING == 0
void spi_send_byte(byte data){
	while (pioDigitalRead(FLUSHING_PIN)) {}
  spiSendReceive(data);
}

void set_cs_high(){
  pioDigitalWrite(CS_PIN, PIO_HIGH);
}

void set_cs_low(){
  pioDigitalWrite(CS_PIN, PIO_LOW);
}

void flush(){
  set_cs_high();
  spi_send_byte(FLASH_CONSTANT);
  set_cs_low();
}

void send_strip(byte strip_number, byte strip_data[WIDTH][PIXEL_SIZE]){
  set_cs_high();
  spi_send_byte(strip_number); // Start by sending strip number
	spi_send_byte(0); // 0 offset
  for (int w = 0; w < WIDTH; w++) {
    for (int p = 0; p < PIXEL_SIZE; p++){
      spi_send_byte(strip_data[w][p]); // send all of the data from the strip using "burst mode"
    }
  }
  set_cs_low();
}

void send_frame(){
  for (int h = 0; h < HEIGHT; h++) {
    send_strip(h, screen[h]);
  }
  flush();
}
#endif

#if TESTING == 1
void send_frame() {
  static int frame_number = 0;
  char frame_name[50];

  sprintf(frame_name, "frames/frame_%d.txt", frame_number);

  int result = mkdir("frames", 0777); // create frames dir if it doesn't exist
  FILE* fd = fopen(frame_name, "w+");

  for (int h = 0; h < HEIGHT; h++) {
    for (int w = 0; w < WIDTH; w++) {
      fprintf(fd, "%u,%u,%u\n", screen[h][w][RED], screen[h][w][GREEN], screen[h][w][BLUE]);
    }
  }

  fclose(fd);
  frame_number += 1;
}
#endif
