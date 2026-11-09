#ifndef CONFIG_H
#define CONFIG_H

#include "character.h"

// Basic dimension struct
typedef struct CharacterDimensions {
    unsigned int width;
    unsigned int height;
    unsigned int scale;
    bool scaleAddHalf;
} CharacterDimensions;

Character *loadCharacter(const char *filename, SDL_PixelFormat fmt);
void saveCharacter(const char *filename, char *up, char *right, char *down, char *left, unsigned char width, unsigned char height, unsigned char len, unsigned char fps, unsigned char scale, bool scaleAddHalf, unsigned char launchSpeed, unsigned char gravity);

CharacterDimensions *loadCharacterDimensions(const char *filename);
void destroyCharacterDimensions(CharacterDimensions *ptr);

#endif
