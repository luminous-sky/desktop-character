#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdbool.h>

#include "image.h"
#include "character.h"

// Creates a character struct with the proper sprites loaded
Character *createCharacter(char *up, char *right, char *down, char *left, SDL_PixelFormat fmt, unsigned int width, unsigned int height, unsigned int len, unsigned int msPerFrame, int scale, bool scaleAddHalf, unsigned int launchSpeed, unsigned int gravity) {
    Character *created = malloc(sizeof(Character));
    created -> width = width;
    created -> height = height;
    created -> len = len;
    created -> msPerFrame = msPerFrame;
    created -> scale = scale;
    created -> scaleAddHalf = scaleAddHalf;
    created -> launchSpeed = launchSpeed;
    created -> gravity = gravity;

    created -> up = loadSpritesheetFromFileScaled(up, fmt, width, height, len, scale, scaleAddHalf);
    created -> right = loadSpritesheetFromFileScaled(right, fmt, width, height, len, scale, scaleAddHalf);
    created -> down = loadSpritesheetFromFileScaled(down, fmt, width, height, len, scale, scaleAddHalf);
    created -> left = loadSpritesheetFromFileScaled(left, fmt, width, height, len, scale, scaleAddHalf);

    return created;
}


// Verifies that the sprite pointers are not NULL
bool validateCharacter(Character *created) {
    if (created -> up == NULL) return false;
    if (created -> right == NULL) return false;
    if (created -> down == NULL) return false;
    if (created -> left == NULL) return false;

    return true;
}


// Destroys a character struct
void destroyCharacter(Character *created) {
    if (created -> up) destroySprites(created -> up, created -> len);
    if (created -> right) destroySprites(created -> right, created -> len);
    if (created -> down) destroySprites(created -> down, created -> len);
    if (created -> left) destroySprites(created -> left, created -> len);

    free(created);
}
