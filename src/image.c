#include <SDL3_image/SDL_image.h>
#include <stdlib.h>
#include <stdio.h>

#include "image.h"


// Older versions of SDL3 do not have SDL_SCALEMODE_PIXELART
#ifndef USE_NEAREST_SCALEMODE
const SDL_ScaleMode PIXEL_SCALEMODE = SDL_SCALEMODE_PIXELART;
#else
const SDL_ScaleMode PIXEL_SCALEMODE = SDL_SCALEMODE_NEAREST;
#endif


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
SDL_Surface **loadSpritesheetScaled(SDL_Surface *surface, int spriteWidth, int spriteHeight, int numSprites, int scale, bool addHalf) {
    if (scale < 1) return NULL;
    
    int cols = (surface -> w) / spriteWidth;

    // Verify dimensions
    int iterRows = (numSprites-1) / cols;
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
    int newWidth = scale*spriteWidth;
    if (addHalf) newWidth += (spriteWidth >> 1);
    
    int newHeight = scale*spriteHeight;
    if (addHalf) newHeight += (spriteHeight >> 1);

    for (int r = 0; r < iterRows-1; r++) {
        bounds.y = r*spriteHeight;

        for (int c = 0; c < cols; c++) {
            bounds.x = c*spriteWidth;

            SDL_Surface *sprite = SDL_CreateSurface(newWidth, newHeight, surface -> format);
            if (scale == 1 && addHalf == false) {
                SDL_BlitSurface(surface, &bounds, sprite, NULL);
            } else {
                SDL_BlitSurfaceScaled(surface, &bounds, sprite, NULL, PIXEL_SCALEMODE);
            }
            
            sprites[cols*r + c] = sprite;
        }
    }
    
    // Incomplete rows
    int lastRow = iterRows;
    bounds.y = lastRow * spriteHeight;
    for (int c = 0; c < numSprites % (cols+1); c++) {
        bounds.x = c*spriteWidth;

        SDL_Surface *sprite = SDL_CreateSurface(newWidth, newHeight, surface -> format);
        if (scale == 1 && addHalf == false) {
            SDL_BlitSurface(surface, &bounds, sprite, NULL);
        } else {
            SDL_BlitSurfaceScaled(surface, &bounds, sprite, NULL, PIXEL_SCALEMODE);
        }

        sprites[cols*lastRow + c] = sprite;
    }

    return sprites;
}

SDL_Surface **loadSpritesheet(SDL_Surface *surface, int spriteWidth, int spriteHeight, int numSprites) {
    return loadSpritesheetScaled(surface, spriteWidth, spriteHeight, numSprites, 1, false);
}


// Combines loadImage() and loadSpritesheet()
SDL_Surface **loadSpritesheetFromFileScaled(char *path, SDL_PixelFormat fmt, int spriteWidth, int spriteHeight, int numSprites, int scale, bool addHalf) {
    SDL_Surface *img = loadImage(path, fmt);
    if (!img) return NULL;
    
    SDL_Surface **sprites = loadSpritesheetScaled(img, spriteWidth, spriteHeight, numSprites, scale, addHalf);
    SDL_DestroySurface(img);
    
    return sprites;
}

SDL_Surface **loadSpritesheetFromFile(char *path, SDL_PixelFormat fmt, int spriteWidth, int spriteHeight, int numSprites) {
    return loadSpritesheetFromFileScaled(path, fmt, spriteWidth, spriteHeight, numSprites, 1, false);
}


// Destroys a spritesheet
void destroySprites(SDL_Surface **sprites, int numSprites) {
    for (int i = 0; i < numSprites; i++) {
        SDL_DestroySurface(sprites[i]);
    }

    free(sprites);
}
