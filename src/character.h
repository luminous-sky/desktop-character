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
    unsigned int width;
    unsigned int height;
    unsigned int len;
    unsigned int msPerFrame;
    unsigned int scale;
    bool scaleAddHalf;

    // Physics details
    unsigned int launchSpeed;
    unsigned int gravity;
} Character;


// Character movement directions
typedef enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    LENGTH
} Direction;


Character *createCharacter(char *up, char *right, char *down, char *left, SDL_PixelFormat fmt, unsigned int width, unsigned int height, unsigned int len, unsigned int msPerFrame, int scale, bool scaleAddHalf, unsigned int launchSpeed, unsigned int gravity);
bool validateCharacter(Character *created);
void destroyCharacter(Character *created);

#endif
