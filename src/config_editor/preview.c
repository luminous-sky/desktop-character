#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "../config.h"
#include "../character.h"

#include "preview.h"


// Returns whether the exit event was sent during the movement (original main.c code)
bool playAnimation(Character *person, Direction dir, SDL_Window *window, SDL_Surface *windowSurface) {
    SDL_Surface **sprites = NULL;
    switch (dir) {
        case UP:
            sprites = person -> up;
            break;
        case RIGHT:
            sprites = person -> right;
            break;
        case DOWN:
            sprites = person -> down;
            break;
        case LEFT:
            sprites = person -> left;
            break;
        default:
            break;
    }

    if (!sprites) {
        SDL_SetError("The character's sprites failed to load.\n");
        return false;
    }

    for (int i = 0; i < person -> len; i++) {
        // Clear background
        SDL_ClearSurface(windowSurface, 0.0, 0.0, 0.0, 0.0);

        // Render sprite
        if (!SDL_BlitSurface(sprites[i], NULL, windowSurface, NULL)) {
            printf("Failed to blit surface.\n%s\n", SDL_GetError());
            return 2;
        }

        SDL_UpdateWindowSurface(window);

        // Delay
        unsigned long start = SDL_GetTicks();
        unsigned long cur = start;
        while ((cur - start) < person -> msPerFrame) {
            cur = SDL_GetTicks();

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) return true;
            }
        }
    }

    return false;
}


// Original main.c code
int showPreview(char *up, char *right, char *down, char *left, unsigned int width, unsigned int height, unsigned int len, unsigned int msPerFrame) {
    // On Linux, Wayland cannot set the position of the window, so X11 must be used
    if (!strcmp(SDL_GetPlatform(), "Linux")) {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
    }

    // Initialize
    SDL_Window *window = SDL_CreateWindow("Desktop Character", width, height, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_TRANSPARENT);
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
    Character *person = createCharacter(up, right, down, left, windowSurface -> format, width, height, len, msPerFrame);

    while (true) {
        bool exiting = false;
        for (int i = 0; i < LENGTH; i++) {
            if (playAnimation(person, i, window, windowSurface)) {
                exiting = true;
                break;
            }

            // Delay between directions
            unsigned long start = SDL_GetTicks();
            unsigned long cur = start;
            while ((cur - start) < 1000) {
                cur = SDL_GetTicks();

                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_EVENT_QUIT) return true;
                }
            }
        }

        if (exiting) break;
    }

    // Clean up
    destroyCharacter(person);
    SDL_DestroyWindow(window);

    return 0;
}
