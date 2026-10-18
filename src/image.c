#include <SDL3_image/SDL_image.h>
#include <stdlib.h>
#include <stdio.h>

#include "image.h"


// Loads an image into a surface with the provided SDL_PixelFormat
SDL_Surface *loadImage(char *path, SDL_PixelFormat fmt) {
    SDL_Surface *loaded = IMG_Load(path);
    if (!loaded) return NULL;
    
    SDL_Surface *formatted = SDL_ConvertSurface(loaded, fmt);
    
    SDL_DestroySurface(loaded);
    loaded = NULL;

    return formatted;
}


// Returns an array of SDL_Surfaces sliced from the original SDL_Surface
SDL_Surface **loadSpritesheet(SDL_Surface *surface, int spriteWidth, int spriteHeight, int numSprites) {
    int cols = (surface -> w) / spriteWidth;

    // Verify dimensions
    int iterRows = numSprites / cols;
    if (spriteHeight*iterRows > surface -> h) {
        SDL_SetError("Parameter 'surface' has an invalid height");
        return NULL;
    }


    // Load sprites
    SDL_Surface **sprites = malloc(numSprites * sizeof(SDL_Surface));

    SDL_Rect bounds = {};
    bounds.w = spriteWidth;
    bounds.h = spriteHeight;

    // Full rows
    for (int r = 0; r < iterRows-1; r++) {
        bounds.y = r*spriteHeight;

        for (int c = 0; c < cols; c++) {
            bounds.x = c*spriteWidth;

            SDL_Surface *sprite = SDL_CreateSurface(spriteWidth, spriteHeight, surface -> format);
            SDL_BlitSurface(surface, &bounds, sprite, NULL);

            sprites[cols*r + c] = sprite;
        }
    }
    
    // Incomplete rows
    int lastRow = iterRows - 1;
    bounds.y = lastRow * spriteHeight;
    for (int c = 0; c < numSprites % (cols+1); c++) {
        bounds.x = c*spriteWidth;

        SDL_Surface *sprite = SDL_CreateSurface(spriteWidth, spriteHeight, surface -> format);
        SDL_BlitSurface(surface, &bounds, sprite, NULL);

        sprites[cols*lastRow + c] = sprite;
    }

    return sprites;
}


// Destroys a spritesheet
void destroySprites(SDL_Surface **sprites, int numSprites) {
    for (int i = 0; i < numSprites; i++) {
        SDL_DestroySurface(sprites[i]);
    }

    free(sprites);
}
