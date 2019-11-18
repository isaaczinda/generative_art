main: main.c
	gcc -o main main.c

clean:
	rm main; rm -rf frames

run: main
	./main
	python3 display.py
