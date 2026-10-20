CC=gcc
CFLAGS=-Wall
CFLAGS_DEBUG=-g -fsanitize=address
CFLAGS_RELEASE=-O2
INCLUDES=-lSDL3 -lSDL3_image

.PHONY: devel

devel:
	$(ARCH)$(CC) src/*.c $(CFLAGS) $(CFLAGS_DEBUG) $(INCLUDES) $(EXTRA_INCLUDES) -o build/out

release:
	$(ARCH)$(CC) src/*.c $(CFLAGS) $(CFLAGS_RELEASE) $(INCLUDES) $(EXTRA_INCLUDES) -o build/out

clean:
	rm build/*
