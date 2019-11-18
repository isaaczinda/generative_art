void moving_circles(byte screen[HEIGHT][WIDTH][PIXEL_SIZE]) {
  for (int i = 0; i < 1000; i++) {
    int r1 = i % 37;
    int r2 = i % 29;
    int r3 = i % 47;
    int r4 = i % 23;
    int r5 = i % 17;

    color c1 = { 255, 0, 0, 150 };
    color c2 = { 0, 255, 0, 150 };
    color c3 = { 255, 0, 255, 150 };
    color c4 = { 255, 255, 0, 150 };
    color c5 = { 0, 0, 255, 150 };

    int x1 = (int)(.3*i) % WIDTH;
    int y1 = (int)(.4*i) % HEIGHT;
    int x2 = (int)(.55*i) % WIDTH;
    int y2 = (int)(.4*i) % HEIGHT;
    int x3 = (int)(1.1*i) % WIDTH;
    int y3 = (int)(.1*i) % HEIGHT;
    int x4 = (int)(.8*i) % WIDTH;
    int y4 = (int)(.98*i)% HEIGHT;
    int x5 = (int)(.21*i) % WIDTH;
    int y5 = (int)(.78*i) % HEIGHT;

    clear_frame(screen);
    draw_circle(screen, r1, 50, x1, y1, c1);


    draw_circle(screen, r2, 100, x2, y2, c2);
    draw_circle(screen, r3, 100, x3, y3, c3);
    draw_circle(screen, r4, 100, x4, y4, c4);
    draw_circle(screen, r5, 100, x5, y5, c5);
    render_frame(screen);
  }
}

void simple_circle(byte screen[HEIGHT][WIDTH][PIXEL_SIZE]) {
  color c = { 255, 0, 0, 255 };

  for (int r = 0; r < 15; r++) {
    clear_frame(screen);
    draw_circle(screen, r, 20, 15, 15, c);
    render_frame(screen);
  }
}
