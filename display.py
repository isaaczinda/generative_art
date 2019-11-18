import tkinter as tk
from glob import glob
import time

HEIGHT = 24
WIDTH = 25

RED = 0
GREEN = 1
BLUE = 2


def get_frame(frame_number):
    frame = [["#ffffff" for s in range(WIDTH)] for i in range(HEIGHT)]
    filepath = f'frames/frame_{frame_number}.txt'

    with open(filepath, "r") as f:
        for pixel_index, line in enumerate(f.readlines()):
            # format the color as a hex string
            color = [int(elem) for elem in line.split(",")]
            color_string = '#%02x%02x%02x' % (color[RED], color[GREEN], color[BLUE])

            # get the row and col from the pixel index
            row = pixel_index // WIDTH
            col = pixel_index % WIDTH

            frame[row][col] = color_string

    return frame


number_frames = len(glob("frames/frame_*.txt"))

master = tk.Tk()
canvas = tk.Canvas(master, width=WIDTH*10, height=HEIGHT*10)

while True:
    for i in range(number_frames):
        frame = get_frame(i)

        for x in range(WIDTH):
            for y in range(HEIGHT):
                color = frame[y][x]
                canvas.create_rectangle(x*10, y*10, x*10 + 10, y*10 + 10, fill=color, outline = 'blue')

        canvas.pack()
        master.update()
        time.sleep(.1)
        canvas.delete("all")
