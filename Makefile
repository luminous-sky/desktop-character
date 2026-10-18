CC=gcc
CFLAGS=-Wall
INCLUDES=-lSDL3 -lSDL3_image

app:
	$(CC) src/*.c $(CFLAGS) $(INCLUDES) -o build/out

clean:
	rm build/*
