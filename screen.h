#include <stdio.h>

typedef unsigned char byte;
typedef struct {
  byte red;
  byte green;
  byte blue;
  byte alpha;
} color;
#define TESTING 1
// number of cols
#define WIDTH 25
// number of rows
#define HEIGHT 24
#define PIXEL_SIZE 3
#define FLASH_CONSTANT 70 // 0b10101010


#if TESTING == 1

#include <sys/stat.h>
#include <sys/types.h>
#endif

#if TESTING == 0
#include "SAM4S4B_libs/SAM4S4B.h"
#endif

#define FLUSHING_PIN PIO_PA26
#define CS_PIN PIO_PA27

byte screen[HEIGHT * WIDTH * PIXEL_SIZE];

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
#endif

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
      fprintf(fd, "%u,%u,%u\n", get_screen_red(h, w), get_screen_green(h, w), get_screen_blue(h, w));
    }
  }

  fclose(fd);
  frame_number += 1;
}
#endif
