main: main.c
	gcc -o main main.c

run: main
	./main
	python3 display.py
