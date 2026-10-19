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
const Uint64 MS_PER_FRAME = 250;


// Returns whether the exit event was sent during the movement
bool move(Character *person, Direction dir, SDL_Window *window, SDL_Surface *windowSurface, int moveOffset) {
    double pxPerMs = (double) moveOffset / ((person -> msPerFrame) * (person -> len));
    
    int curX, curY;
    if(!SDL_GetWindowPosition(window, &curX, &curY)) {
        printf("Failed to get the window's current position.\nError: %s\n", SDL_GetError());
        return true;
    }

    int sign = 1;
    int endX = curX;
    int endY = curY;

    // Accumulation of pixel change
    double accumulatedX = 0;
    double accumulatedY = 0;
    
    SDL_Surface **sprites = NULL;
    switch (dir) {
        case UP:
            sprites = person -> up;
            endY -= moveOffset;
            sign = -1;
            break;
        case RIGHT:
            sprites = person -> right;
            endX += moveOffset;
            break;
        case DOWN:
            sprites = person -> down;
            endY += moveOffset;
            break;
        case LEFT:
            sprites = person -> left;
            endX -= moveOffset;
            sign = -1;
            break;
        default:
            break;
    }


    // Clamp bounds (assuming the first display is used)
    const SDL_DisplayMode *display = SDL_GetCurrentDisplayMode(1);
    if (!display) {
        printf("Failed to get display.\nError: %s\n", SDL_GetError());
        return true;
    }

    if (endX < 0) endX = 0;
    if (endX > display -> w) endX = display -> w;
    if (endY < 0) endY = 0;
    if (endY > display -> h) endY = display -> h;


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
        unsigned long prev = start;
        unsigned long cur = start;
        while ((cur - start) < person -> msPerFrame) {
            prev = cur;
            cur = SDL_GetTicks();

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    return true;
                }
            }

            // Move the window using deltatime
            if (curX != endX) {
                accumulatedX += pxPerMs * (cur - prev);

                if (accumulatedX >= 1.0) {
                    int toUse = (int) accumulatedX;
                    accumulatedX -= toUse;
                    curX += (sign * toUse);
                }
            } else if (curY != endY) {
                accumulatedY += pxPerMs * (cur - prev);

                if (accumulatedY >= 1.0) {
                    int toUse = (int) accumulatedY;
                    accumulatedY -= toUse;
                    curY += (sign * toUse);
                }
            }
            if (!SDL_SetWindowPosition(window, curX, curY)) {
                printf("Failed to set window position.\n%s\n", SDL_GetError());
            }
            SDL_SyncWindow(window);
        }
    }

    return false;
}


int main() {
    // On Linux, Wayland cannot set the position of the window, so X11 must be used
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");

    // Initialize
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Failed to initialize SDL.\n");
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Desktop Character", WIDTH, HEIGHT, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_TRANSPARENT);
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

    while (true) {
        Direction randDir = (Direction) (SDL_rand(LENGTH));
        int dist = SDL_rand(15) + 30;
        if (move(person, randDir, window, windowSurface, dist)) break;

        // Pause between moves
        bool exiting = false;
        int pauseMs = SDL_rand(500) + 1500;

        unsigned long start = SDL_GetTicks();
        unsigned long cur = start;
        while ((cur - start) < pauseMs) {
            cur = SDL_GetTicks();

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    exiting = true;
                    break;
                }
            }

            if (exiting) break;
        }
        if (exiting) break;
    }

    // Clean up
    destroyCharacter(person);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
