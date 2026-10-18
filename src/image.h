#ifndef IMAGE_H
#define IMAGE_H

#include <SDL3_image/SDL_image.h>

SDL_Surface *loadImage(char *path, SDL_PixelFormat fmt);
SDL_Surface **loadSpritesheet(SDL_Surface *surface, int spriteWidth, int spriteHeight, int numSprites);
void destroySprites(SDL_Surface **sprites, int numSprites);

#endif
