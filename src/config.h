#ifndef CONFIG_H
#define CONFIG_H

#include "character.h"

// Basic dimension struct
typedef struct CharacterDimensions {
    unsigned int width;
    unsigned int height;
} CharacterDimensions;

Character *loadCharacter(const char *filename, SDL_PixelFormat fmt);
void saveCharacter(const char *filename, char *up, char *right, char *down, char *left, unsigned char width, unsigned char height, unsigned char len, unsigned char fps);

CharacterDimensions *loadCharacterDimensions(const char *filename);
void destroyCharacterDimensions(CharacterDimensions *ptr);

#endif
