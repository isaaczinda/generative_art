#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>

// number of cols
#define WIDTH 25

// number of rows
#define HEIGHT 24

#define PIXEL_SIZE 3

#define RED 0
#define GREEN 1
#define BLUE 2

typedef unsigned char byte;

typedef struct {
  byte red;
  byte green;
  byte blue;
  byte alpha;
} color;

void render_frame(byte[HEIGHT][WIDTH][PIXEL_SIZE]);
void clear_frame(byte[HEIGHT][WIDTH][PIXEL_SIZE]);
void draw_circle(byte[HEIGHT][WIDTH][PIXEL_SIZE], double, int, int, int, color);

#include "archive.h"

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

    draw_circle(screen, r2, 100, 6, 12, c2);
    draw_circle(screen, r2 + 4, 100, 6, 12, c2);

    draw_circle(screen, r3, 100, 15, 22, c3);
    draw_circle(screen, r3+1, 100, 15, 22, c3);
    draw_circle(screen, r3+2, 100, 15, 22, c3);

    draw_circle(screen, r4, 100, 23, 6, c4);

    draw_circle(screen, r5, 100, 13, 17, c5);
    render_frame(screen);
  }
}

int main() {
  byte screen[HEIGHT][WIDTH][PIXEL_SIZE];

  static_circles(screen);
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

void render_frame(byte frame[HEIGHT][WIDTH][PIXEL_SIZE]) {
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
