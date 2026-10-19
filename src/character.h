#ifndef CHARACTER_H
#define CHARACTER_H

#include <SDL3/SDL.h>

// Structure to hold basic sprites
typedef struct Character {
    // Directional sprites
    SDL_Surface **up;
    SDL_Surface **right;
    SDL_Surface **down;
    SDL_Surface **left;

    // Animation details
    unsigned int len;
    unsigned int msPerFrame;
} Character;


// Character movement directions
typedef enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    LENGTH
} Direction;


Character *createCharacter(char *up, char *right, char *down, char *left, SDL_PixelFormat fmt, unsigned int width, unsigned int height, unsigned int len, unsigned int msPerFrame);
void destroyCharacter(Character *created);

#endif
