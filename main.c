// outputs frame to local files when we are testing
// outputs frame to strips over SPI when we aren't testing
#define TESTING 0

#include <math.h>
#include <stdio.h>
#include <stdlib.h>


// only include os libraries when we're testing
#if TESTING == 1

#include <sys/stat.h>
#include <sys/types.h>
#endif

#if TESTING == 0
#include "SAM4S4B_libs\SAM4S4B.h"
#endif

// number of cols
#define WIDTH 25
// number of rows
#define HEIGHT 24

#define PIXEL_SIZE 3

#define M_PI 3.1415

#define RED 0
#define GREEN 1
#define BLUE 2

// to simplify boolean logic
#define TRUE 1
#define FALSE 0

#define FLASH_CONSTANT 70 // 0b10101010

#define FLUSHING_PIN PIO_PA26
#define CS_PIN PIO_PA27


typedef unsigned char byte;

byte screen[HEIGHT * WIDTH * PIXEL_SIZE];
float g_brightness;

byte zero_buffer[HEIGHT * WIDTH * PIXEL_SIZE / 2] = {
	0,0,0, 0,0,0, 0,0,0, 0,0,0,      0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
	0,0,0, 0,0,0, 0,0,0, 0,0,0,      0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
	0,0,0, 0,0,0, 0,0,0, 0,0,0,      0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
	0,0,0, 0,0,0, 0,0,0, 0,0,0,      0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
	0,0,0, 0,0,0, 0,0,0, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 100,100,100, 0,0,0,
	0,0,0, 0,0,0, 0,0,0, 100,100,100, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
	0,0,0, 0,0,0, 0,0,0, 100,100,100, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
	0,0,0, 0,0,0, 0,0,0, 100,100,100, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
	0,0,0, 0,0,0, 0,0,0, 100,100,100, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
	0,0,0, 0,0,0, 0,0,0, 100,100,100, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
	0,0,0, 0,0,0, 0,0,0, 0,0,0,      0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 
	0,0,0, 0,0,0, 0,0,0, 0,0,0,      0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0};
	
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
void send_zeros();

#define NUM_PARTICLES 20
#define FRAME_TIME .01666
#define SPEED_FACTOR 10

particle particles[NUM_PARTICLES];

void set_screen_color(unsigned char h, unsigned char w, color pixel_color){
		screen[(h * WIDTH * PIXEL_SIZE) + (w * PIXEL_SIZE)] = pixel_color.red;
		screen[(h * WIDTH * PIXEL_SIZE) + (w * PIXEL_SIZE) + 1] = pixel_color.green;
		screen[(h * WIDTH * PIXEL_SIZE) + (w * PIXEL_SIZE) + 2] = pixel_color.blue;	
}

void set_screen(unsigned char h, unsigned char w, unsigned char red, unsigned char green, unsigned char blue){
		screen[(h * WIDTH * PIXEL_SIZE) + (w * PIXEL_SIZE)] = red;
		screen[(h * WIDTH * PIXEL_SIZE) + (w * PIXEL_SIZE) + 1] = green;
		screen[(h * WIDTH * PIXEL_SIZE) + (w * PIXEL_SIZE) + 2] = blue;
}

byte get_screen_red(unsigned char h, unsigned char w){
	return screen[(h * WIDTH * PIXEL_SIZE) + (w * PIXEL_SIZE)];
}

byte get_screen_green(unsigned char h, unsigned char w){
	return screen[(h * WIDTH * PIXEL_SIZE) + (w * PIXEL_SIZE) + 1];
}

byte get_screen_blue(unsigned char h, unsigned char w){
	return screen[(h * WIDTH * PIXEL_SIZE) + (w * PIXEL_SIZE) + 2];
}

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

								set_screen(y,x, pixel_color.red, pixel_color.green, pixel_color.blue);
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
						set_screen(y, x, 255, 0, 0);
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

void speed_test() {
	for(int x = 0; x < 1000; x++){
		send_zeros();
	}
}

int main() {
	#if TESTING == 0
	samInit();
  pioInit();
  spiInit(35, 0, 1);
	pioPinMode(CS_PIN, PIO_OUTPUT);
	pioPinMode(FLUSHING_PIN, PIO_INPUT);
	#endif
	
	g_brightness = .1;

	//speed_test();
	static_circles();
  //shifting_background();
  //particles_animation();
}

void draw_background(double freq_x, double freq_y, double offset_x, double offset_y, color bg_color) {
    double opacity = bg_color.alpha / 255.0;

    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {

            // from 0 to 1
            double brightness = (( sin(x * 2 * M_PI * freq_x - offset_x) * cos(y * 2 * M_PI * freq_y - offset_y) ) / 2.0) + .5;
						set_screen(y, x, (1-opacity) * get_screen_red(y,x) + opacity *  brightness * bg_color.red,
					(1-opacity) * get_screen_green(y,x) + opacity * brightness * bg_color.green,
					(1-opacity) * get_screen_blue(y,x) + opacity * brightness * bg_color.blue);
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
			set_screen(y, x, floor(circle_color.red * decimal_alpha + get_screen_red(y,x) * (1-decimal_alpha)),
									floor(circle_color.green * decimal_alpha + get_screen_green(y,x) * (1-decimal_alpha)),
									floor(circle_color.blue * decimal_alpha + get_screen_blue(y,x) * (1-decimal_alpha)));
    }
  }
}

void clear_frame() {
  for (int x = 0; x < WIDTH * HEIGHT * PIXEL_SIZE; x++) {
		screen[x] = 0;
  }
}

void spi_send_byte(byte data){
	while (pioDigitalRead(FLUSHING_PIN)) {}
  spiSendReceive(data);
}

// TODO: replace with gpio pin header file
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
      spi_send_byte((char) strip_data[w][p]); // send all of the data from the strip using "burst mode"
    }
  }
  set_cs_low();
}

void send_zeros(){
  for (int h = 0; h < HEIGHT; h++) {
      set_cs_high();
			spi_send_byte(h); // Start by sending strip number
			spi_send_byte(0); // 0 offset
			unsigned char adjusted_h = h % (HEIGHT / 2);
			for (int w = 0; w < WIDTH; w++) {
				for (int p = 0; p < PIXEL_SIZE; p++){
					unsigned int offset = (adjusted_h * WIDTH * PIXEL_SIZE) + (w * PIXEL_SIZE) + p;
					spi_send_byte((char) zero_buffer[offset]); // send all of the data from the strip using "burst mode"
				}
			}
			set_cs_low();
  }
  flush();
}

#if TESTING == 0
void send_frame(){
  for (int h = 0; h < HEIGHT; h++) {
      set_cs_high();
			spi_send_byte(h); // Start by sending strip number
			spi_send_byte(0); // 0 offset
			for (int w = 0; w < WIDTH; w++) {
				for (int p = 0; p < PIXEL_SIZE; p++){
					spi_send_byte((char) screen[(h * WIDTH * PIXEL_SIZE) + (w * PIXEL_SIZE) + p] * g_brightness); // send all of the data from the strip using "burst mode"
				}
			}
			set_cs_low();
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
      fprintf(fd, "%u,%u,%u\n", frame[h][w][RED], frame[h][w][GREEN], frame[h][w][BLUE]);
    }
  }

  fclose(fd);
  frame_number += 1;
}
#endif
