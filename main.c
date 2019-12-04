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

#define FLASH_CONSTANT 170 // 0b10101010
#define RENDERING 1 // outputs frame to local files when non-zero
#define SPI_ON 0 // outputs frame to strips over SPI when non-zero

#define FALSE 0
#define TRUE 1


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

typedef struct {
    float x_pos;
    float y_pos;
    float x_vel;
    float y_vel;
    color col;
    int active;
} particle;


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

void particles_animation(byte screen[HEIGHT][WIDTH][PIXEL_SIZE]) {
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

        if(RENDERING){
          render_frame(screen);
        }
        if(SPI_ON){
          send_frame(screen);
        }

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

    if(RENDERING){
      render_frame(screen);
    }
    if(SPI_ON){
      send_frame(screen);
    }
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

    if(RENDERING){
      render_frame(screen);
    }
    if(SPI_ON){
      send_frame(screen);
    }
  }
}

int main() {
  byte screen[HEIGHT][WIDTH][PIXEL_SIZE];

  // shifting_background(screen);
  particles_animation(screen);
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

// TODO: replace with spi header file
void spi_send_byte(byte data){
  printf("SPI output: %u\n", data);
}

// TODO: replace with gpio pin header file
void set_cs_high(){
  printf("CS: High\n");
}

void set_cs_low(){
  printf("CS: Low\n");
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
