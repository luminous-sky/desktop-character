#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "image.h"
#include "character.h"

// Temporary variables
const unsigned int WIDTH = 32;
const unsigned int HEIGHT = 32;

// Spritesheet information
const unsigned int SPRITE_WIDTH = 32;
const unsigned int SPRITE_HEIGHT = 32;
const unsigned int FRAMES = 4;

// 4 FPS
const Uint64 MS_PER_FRAME = 1000;


int main() {
    // Initialize
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Failed to initialize SDL.\n");
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Desktop Character", WIDTH, HEIGHT, SDL_WINDOW_BORDERLESS | SDL_WINDOW_TRANSPARENT);
    if (!window) {
        printf("Failed to create a window.\n");
        return 1;
    }

    SDL_Surface *windowSurface = SDL_GetWindowSurface(window);
    if (!windowSurface) {
        printf("Failed to get the window's surface.\n");
        return 1;
    }


    // Create character
    char *up = "./example/walk_up.png";
    char *right = "./example/walk_right.png";
    char *down = "./example/walk_down.png";
    char *left = "./example/walk_left.png";
    Character *person = createCharacter(up, right, down, left, windowSurface -> format, SPRITE_WIDTH, SPRITE_HEIGHT, FRAMES, MS_PER_FRAME);
    if (!person) {
        printf("Failed to load spritesheets.\nError: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Surface **sprites = person -> down;

    Uint32 bg = SDL_MapSurfaceRGBA(windowSurface, 0xFF, 0xFF, 0xFF, 0x00);
    for (int i = 0; i < person -> len; i++) {
        // Clear background
        SDL_FillSurfaceRect(windowSurface, NULL, bg);

        // Render sprite
        if (!SDL_BlitSurface(sprites[i], NULL, windowSurface, NULL)) {
            printf("Failed to blit surface.\n%s\n", SDL_GetError());
            return 2;
        }

        SDL_UpdateWindowSurface(window);

        // Delay
        bool exiting = false;

        unsigned long start = SDL_GetTicks();
        unsigned long cur = SDL_GetTicks();
        while ((cur - start) < person -> msPerFrame) {
            cur = SDL_GetTicks();

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    exiting = true;
                    break;
                }
            }
        }

        if (exiting) break;
    }

    // Clean up
    destroyCharacter(person);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
