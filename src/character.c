#include <SDL3/SDL.h>
#include <stdlib.h>

#include "image.h"
#include "character.h"

// Creates a character struct with the proper sprites loaded
Character *createCharacter(char *up, char *right, char *down, char *left, SDL_PixelFormat fmt, unsigned int width, unsigned int height, unsigned int len, unsigned int msPerFrame) {
    Character *created = malloc(sizeof(Character));
    created -> width = width;
    created -> height = height;
    created -> len = len;
    created -> msPerFrame = msPerFrame;

    created -> up = loadSpritesheetFromFile(up, fmt, width, height, len);
    created -> right = loadSpritesheetFromFile(right, fmt, width, height, len);
    created -> down = loadSpritesheetFromFile(down, fmt, width, height, len);
    created -> left = loadSpritesheetFromFile(left, fmt, width, height, len);

    return created;
}


// Destroys a character struct
void destroyCharacter(Character *created) {
    if (created -> up) destroySprites(created -> up, created -> len);
    if (created -> right) destroySprites(created -> right, created -> len);
    if (created -> down) destroySprites(created -> down, created -> len);
    if (created -> left) destroySprites(created -> left, created -> len);

    free(created);
}
