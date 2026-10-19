#ifndef IMAGE_H
#define IMAGE_H

#include <SDL3/SDL.h>

SDL_Surface *loadImage(char *path, SDL_PixelFormat fmt);
SDL_Surface **loadSpritesheet(SDL_Surface *surface, int spriteWidth, int spriteHeight, int numSprites);
SDL_Surface **loadSpritesheetFromFile(char *path, SDL_PixelFormat fmt, int spriteWidth, int spriteHeight, int numSprites);
void destroySprites(SDL_Surface **sprites, int numSprites);

#endif
