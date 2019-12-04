// outputs frame to local files when we are testing
// outputs frame to strips over SPI when we aren't testing
#define TESTING 0

#include <math.h>


// only include os libraries when we're testing
#if TESTING == 1
#include <stdio.h>
#include <stdlib.h>
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

#define NUM_PARTICLES 20
#define FRAME_TIME .01666
#define SPEED_FACTOR 10

// to simplify boolean logic
#define TRUE 1
#define FALSE 0

#define FLASH_CONSTANT 170 // 0b10101010

#define FLUSHING_PIN PIO_PB10
#define CS_PIN PIO_PB11


typedef unsigned char byte;

typedef struct {
  byte red;
  byte green;
  byte blue;
  byte alpha;
} color;

void render_frame(byte[HEIGHT][WIDTH][PIXEL_SIZE]);
void send_frame(byte[HEIGHT][WIDTH][PIXEL_SIZE]);
void clear_frame(byte[HEIGHT][WIDTH][PIXEL_SIZE]);
void draw_circle(byte[HEIGHT][WIDTH][PIXEL_SIZE], double, int, int, int, color);
void draw_background(byte[HEIGHT][WIDTH][PIXEL_SIZE], double, double, double, double, color);

void test_animation(byte screen[HEIGHT][WIDTH][PIXEL_SIZE]) {
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
		
		send_frame(screen);
	}
}

void shifting_background(byte screen[HEIGHT][WIDTH][PIXEL_SIZE]) {
  for (int i = 0; i < 1000; i++) {
    clear_frame(screen);

    // frequency ranges from from .05 to .15
    double freq_x_1 = .05 * sin(2 * M_PI * i * .0245) + .08;
    double freq_y_1 = .05 * sin(2 * M_PI * i * .0394) + .08;

    double offset_x_1 = 1.4 * sin(2 * M_PI * i * .0494);
    double offset_y_1 = .8 * sin(2 * M_PI * i * .0394);


    double freq_x_2 = .05 * sin(2 * M_PI * i * .00245) + .08;
    double freq_y_2 = .05 * sin(2 * M_PI * i * .01345) + .08;

    double offset_x_2 = 1.2 * sin(2 * M_PI * i * .0294);
    double offset_y_2 = .4 * sin(2 * M_PI * i * .0194);


    draw_background(screen, freq_x_1, freq_y_1, offset_x_1, offset_y_1, (color){255, 0, 255, 255});
    draw_background(screen, freq_x_2, freq_y_2, offset_x_2, offset_y_2, (color){0, 150, 255, 100});
    draw_background(screen, freq_y_2, freq_x_1 * .9, offset_y_2, offset_x_1, (color){0, 50, 255, 70});

		send_frame(screen);
  }
}

void static_circles(byte screen[HEIGHT][WIDTH][PIXEL_SIZE]) {
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

    clear_frame(screen);

    draw_circle(screen, r1, 200, 2, 2, c1);
    draw_circle(screen, r1 + 1, 200, 2, 2, c1);

    draw_circle(screen, r2, 400, 6, 12, c2);
    draw_circle(screen, r2 + 4, 400, 6, 12, c2);

    draw_circle(screen, r3, 400, 15, 22, c3);

    draw_circle(screen, r4, 400, 23, 6, c4);
    draw_circle(screen, r5, 400, 13, 17, c5);

		send_frame(screen);
  }
}

byte screen[HEIGHT][WIDTH][PIXEL_SIZE];

int main() {
	#if TESTING == 0
	samInit();
  pioInit();
  spiInit(MCK_FREQ/244000, 0, 1);
	pioPinMode(CS_PIN, PIO_OUTPUT);
	pioPinMode(FLUSHING_PIN, PIO_INPUT);
	#endif
	
  // shifting_background(screen);
  static_circles(screen);
}

void draw_background(byte screen[HEIGHT][WIDTH][PIXEL_SIZE], double freq_x, double freq_y, double offset_x, double offset_y, color bg_color) {
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
void draw_circle(byte screen[HEIGHT][WIDTH][PIXEL_SIZE], double radius,
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

void clear_frame(byte screen[HEIGHT][WIDTH][PIXEL_SIZE]) {
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      screen[y][x][RED] = 0;
      screen[y][x][GREEN] = 0;
      screen[y][x][BLUE] = 0;
    }
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
      spi_send_byte(strip_data[w][p]); // send all of the data from the strip using "burst mode"
    }
  }
  set_cs_low();
}

#if TESTING == 0
void send_frame(byte frame[HEIGHT][WIDTH][PIXEL_SIZE]){
  for (int h = 0; h < HEIGHT; h++) {
    send_strip(h,frame[h]);
  }
  flush();
}
#endif

#if TESTING == 1
void send_frame(byte frame[HEIGHT][WIDTH][PIXEL_SIZE]) {
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
