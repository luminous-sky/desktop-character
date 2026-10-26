#include <stdio.h>
#include <string.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "image.h"
#include "character.h"
#include "config.h"


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


typedef struct MainLoopArgs {
    SDL_Window *window;
    SDL_Surface *windowSurface;
    Character *person;
    unsigned long *start;
    int *curPauseMs;
} MainLoopArgs;


// Returns whether an exit-related signal was sent
bool mainLoop(void *argStruct) {
    MainLoopArgs *args = (MainLoopArgs *) argStruct;

    SDL_Window *window = args -> window;
    SDL_Surface *windowSurface = args -> windowSurface;
    Character *person = args -> person;
    unsigned long *start = args -> start;
    int *curPauseMs = args -> curPauseMs;

    unsigned long curTime = SDL_GetTicks();


    // *curPauseMs < 0 only after the pause time is reset
    if (*curPauseMs < 0) {
        // Move
        Direction randDir = (Direction) (SDL_rand(LENGTH));
        int dist = SDL_rand(15) + 30;
        if (move(person, randDir, window, windowSurface, dist)) return true;

        // Setup pause between moves
        *curPauseMs = SDL_rand(500) + 1500;
        *start = SDL_GetTicks();
    } else {
        // Reset pause time once finished
        if ((curTime - *start) >= *curPauseMs) {
            *curPauseMs = -1;
        }
    }

    
    // Read events
    bool exiting = false;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            exiting = true;
            break;
        }
    }
    
    return exiting;
}


int main() {
    // On Linux, Wayland cannot set the position of the window, so X11 must be used
    if (!strcmp(SDL_GetPlatform(), "Linux")) {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
    }

    // Initialize
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Failed to initialize SDL.\n");
        return 1;
    }
    
    CharacterDimensions *spriteSize = loadCharacterDimensions("./config.charconf");

    SDL_Window *window = SDL_CreateWindow("Desktop Character", spriteSize -> width, spriteSize -> height, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_TRANSPARENT);
    destroyCharacterDimensions(spriteSize);
    spriteSize = NULL;
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
    Character *person = loadCharacter("./config.charconf", windowSurface -> format);

    // Create arg struct
    unsigned long start = 0;
    int curPauseMs = -1;
    MainLoopArgs argStruct = {
        window,
        windowSurface,
        person,
        &start,
        &curPauseMs
    };
    MainLoopArgs *argRef = &argStruct;

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, 1);
    #else
    while (true) {
        if (mainLoop(argRef)) break;
    }
    #endif

    // Clean up
    destroyCharacter(person);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
