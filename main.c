// outputs frame to local files when we are testing
// outputs frame to strips over SPI when we aren't testing


#include "particles.h"


// only include os libraries when we're testing


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



typedef unsigned char byte;


void render_frame(void);
void clear_frame(void);
void draw_circle(double, int, int, int, color);
void draw_background(double, double, double, double, color);
void send_zeros();


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


