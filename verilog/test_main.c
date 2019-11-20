#include "SAM4S4B_lab7\SAM4S4B.h"
#include <math.h>

#define WIDTH 2

// number of rows
#define HEIGHT 2

#define PIXEL_SIZE 3

#define RED 0
#define GREEN 1
#define BLUE 2

#define FLASH_CONSTANT 70 // 0b01000110

#define FLUSHING_PIN PIO_PB11
#define CS_PIN PIO_PB10

typedef unsigned char byte;

typedef struct {
  byte red;
  byte green;
  byte blue;
  byte alpha;
} color;

void send_frame(byte[HEIGHT][WIDTH][PIXEL_SIZE]);
void clear_frame(byte[HEIGHT][WIDTH][PIXEL_SIZE]);
void draw_circle(byte[HEIGHT][WIDTH][PIXEL_SIZE], double, int, int, int, color);

int main() {
	samInit();
	pioInit();
	spiInit(MCK_FREQ/244000, 0, 1);
	pioPinMode(CS_PIN, PIO_INPUT);
	pioPinMode(FLUSHING_PIN, PIO_OUTPUT);
	
	byte screen[HEIGHT][WIDTH][PIXEL_SIZE];
	send_frame(screen);
}

// mode = 1: opaque
// mode = 0: transparent
void draw_circle(byte screen[HEIGHT][WIDTH][PIXEL_SIZE], double radius,
                 int num_points, int center_x, int center_y,
                 color circle_color)
{
  double delta_angle = 2 * 3.141592 / (double)num_points;

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
	while (pioDigitalRead(FLUSHING_PIN));
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
  for (int w = 0; w < WIDTH; w++) {
    for (int p = 0; p < PIXEL_SIZE; p++){
      spi_send_byte(strip_data[w][p]); // send all of the data from the strip using "burst mode"
    }
  }
  set_cs_low();
}


void send_frame(byte frame[HEIGHT][WIDTH][PIXEL_SIZE]){
  for (int h = 0; h < HEIGHT; h++) {
    send_strip(h,frame[h]);
  }
  flush();
}
