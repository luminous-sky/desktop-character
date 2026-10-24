#ifndef CONFIG_H
#define CONFIG_H

#include "character.h"

Character *loadCharacter(const char *filename, SDL_PixelFormat fmt);
void saveCharacter(const char *filename, char *up, char *right, char *down, char *left, unsigned char width, unsigned char height, unsigned char len, unsigned char fps);

#endif
