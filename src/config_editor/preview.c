#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "../config.h"
#include "../character.h"

#include "preview.h"
#include "preview_mem.h"

const char *SEMAPHORE_NAME = "preview_semaphore";
const char *BACKING_FILENAME = "/preview_share_file";

// Target time for refreshing events
int targetMs = 16;


// Returns whether the exit event was sent during the movement (original main.c code)
bool playAnimation(Character *person, Direction dir, SDL_Window *window, SDL_Surface *windowSurface, PreviewMem *previewMem) {
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
            if (sharedMemReadInt(previewMem)) return true;

            cur = SDL_GetTicks();

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) return true;
            }

            SDL_Delay(targetMs);
        }
    }

    return false;
}


// Original main.c code
void *showPreview(void *voidArgs) {
    PreviewArgs *args = (PreviewArgs *) voidArgs;
    int *retVal = args -> retVal;

    PreviewMem *previewMem = createPreviewMem(SEMAPHORE_NAME, BACKING_FILENAME, false);
    if (!previewMem) {
        *retVal = 1;
        return retVal;
    }

    Character *person = args -> person;
    if (!person) {
        *retVal = 1;
        return retVal;
    }

    // On Linux, Wayland cannot set the position of the window, so X11 must be used
    if (!strcmp(SDL_GetPlatform(), "Linux")) {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
    }

    // Initialize
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Failed to initialize SDL.\n");
        destroyCharacter(person);

        *retVal = 1;
        return retVal;
    }

    int fullWidth = (person -> scale) * (person -> width);
    if (person -> scaleAddHalf) {
        fullWidth += (person -> width) >> 1;
    }
    int fullHeight = (person -> scale) * (person -> height);
    if (person -> scaleAddHalf) {
        fullHeight += (person -> height) >> 1;
    }

    SDL_Window *window = SDL_CreateWindow("Desktop Character", fullWidth, fullHeight, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_TRANSPARENT);
    if (!window) {
        printf("Failed to create a window.\n");
        destroyCharacter(person);
        SDL_Quit();

        *retVal = 1;
        return retVal;
    }

    SDL_Surface *windowSurface = SDL_GetWindowSurface(window);
    if (!windowSurface) {
        printf("Failed to get the window's surface.\n");
        destroyCharacter(person);
        SDL_DestroyWindow(window);
        SDL_Quit();

        *retVal = 1;
        return retVal;
    }


    int numDisplays = 0;
    SDL_DisplayID *displays = SDL_GetDisplays(&numDisplays);
    if (numDisplays > 0) {
        // Pick the first display
        const SDL_DisplayMode *display = SDL_GetCurrentDisplayMode(displays[0]);
        SDL_free(displays);

        if (!display) {
            printf("Failed to get display.\nError: %s\n", SDL_GetError());
        } else {
            // Target the monitor's frame rate to reduce CPU use
            if (display -> refresh_rate > 0.0f) {
                targetMs = (int) (1000.0 / display -> refresh_rate);
            }
        }
    }


    while (true) {
        bool exiting = false;
        for (int i = 0; i < LENGTH; i++) {
            if (playAnimation(person, i, window, windowSurface, previewMem)) {
                exiting = true;
                break;
            }

            // Delay between directions
            unsigned long start = SDL_GetTicks();
            unsigned long cur = start;
            while ((cur - start) < 1000) {
                if (sharedMemReadInt(previewMem)) {
                    exiting = true;
                    break;
                }

                cur = SDL_GetTicks();

                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_EVENT_QUIT) {
                        exiting = true;
                        break;
                    }
                }

                SDL_Delay(targetMs);
            }
        }

        if (exiting) break;
    }

    // Clean up
    destroyCharacter(person);
    SDL_DestroyWindow(window);

    SDL_Quit();

    destroyPreviewMem(previewMem, false);

    *retVal = 0;
    return retVal;
}
