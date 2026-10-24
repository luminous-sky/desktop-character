CC=gcc
CFLAGS=-Wall
CFLAGS_DEBUG=-g -fsanitize=address
CFLAGS_RELEASE=-O2
INCLUDES=-lSDL3 -lSDL3_image
INCLUDE_DIRS=-Ithirdparty
EDITOR_INCLUDES=-lraylib -lm

.PHONY: devel

devel:
	$(ARCH)$(CC) src/*.c $(CFLAGS) $(CFLAGS_DEBUG) $(INCLUDE_DIRS) $(INCLUDES) $(EXTRA_INCLUDES) -o build/out

release:
	$(ARCH)$(CC) src/*.c $(CFLAGS) $(CFLAGS_RELEASE) $(INCLUDE_DIRS) $(INCLUDES) $(EXTRA_INCLUDES) -o build/out


editor:
	$(ARCH)$(CC) src/image.c src/character.c src/config.c src/config_editor/*.c $(CFLAGS) $(CFLAGS_DEBUG) $(INCLUDE_DIRS) $(INCLUDES) $(EDITOR_INCLUDES) $(EXTRA_INCLUDES) -o build/editor

editor_release:
	$(ARCH)$(CC) src/image.c src/character.c src/config.c src/config_editor/*.c $(CFLAGS) $(CFLAGS_RELEASE) $(INCLUDE_DIRS) $(INCLUDES) $(EDITOR_INCLUDES) $(EXTRA_INCLUDES) -o build/editor


clean:
	rm build/*
