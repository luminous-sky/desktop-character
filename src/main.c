#include <stdio.h>
#include <string.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "image.h"
#include "character.h"
#include "config.h"


// Returns whether the entire animation finished cycling
bool move(Character *person, Direction dir, SDL_Window *window, SDL_Surface *windowSurface, int moveOffset, int *endX, int *endY, unsigned int *curFrame, unsigned int *t1, unsigned int *t2) {
    unsigned int msPerFrame = person -> msPerFrame;
    unsigned int msPerPx = ((msPerFrame * (person -> len))) / moveOffset;
    
    int curX, curY;
    if(!SDL_GetWindowPosition(window, &curX, &curY)) {
        printf("Failed to get the window's current position.\nError: %s\n", SDL_GetError());
        return true;
    }

    // Get sprite direction
    SDL_Surface **sprites = NULL;
    int sign = 1;
    switch (dir) {
        case UP:
            sprites = person -> up;
            sign = -1;
            break;
        case RIGHT:
            sprites = person -> right;
            break;
        case DOWN:
            sprites = person -> down;
            break;
        case LEFT:
            sprites = person -> left;
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
    
    // Recalculate end points at start
    if (*endX < 0 || *endY < 0) {
        *endX = curX;
        *endY = curY;

        switch (dir) {
            case UP:
                *endY -= moveOffset;
                break;
            case RIGHT:
                *endX += moveOffset;
                break;
            case DOWN:
                *endY += moveOffset;
                break;
            case LEFT:
                *endX -= moveOffset;
                break;
            default:
                break;
        }

        if (*endX < 0) *endX = 0;
        if (*endX > display -> w) *endX = display -> w;
        if (*endY < 0) *endY = 0;
        if (*endY > display -> h) *endY = display -> h;
    }


    if (!sprites) {
        SDL_SetError("The character's sprites failed to load.\n");
        return false;
    }

    // Pick the next frame once msPerFrame goes over
    if (*t1 >= msPerFrame) {
        *curFrame = (*curFrame + 1) % (person -> len);
        *t1 %= msPerFrame;
    }

    // Clear background
    SDL_ClearSurface(windowSurface, 0.0, 0.0, 0.0, 0.0);

    // Render sprite
    if (!SDL_BlitSurface(sprites[*curFrame], NULL, windowSurface, NULL)) {
        printf("Failed to blit surface.\n%s\n", SDL_GetError());
        return 2;
    }

    SDL_UpdateWindowSurface(window);


    // Render movement once enough time has elapsed to move >1 px
    bool finished = false;
    if (*t2 >= msPerPx) {
        int displacement = (*t2) / msPerPx;
        *t2 %= msPerPx;

        if (sign * (*endX - curX) > 0) {
            curX += (sign * displacement);
        } else if (sign * (*endY - curY) > 0) {
            curY += (sign * displacement);
        } else {
            finished = true;
        }
    }

    if (!SDL_SetWindowPosition(window, curX, curY)) {
        printf("Failed to set window position.\n%s\n", SDL_GetError());
    }
    SDL_SyncWindow(window);

    return finished;
}


typedef struct MainLoopArgs {
    SDL_Window *window;
    SDL_Surface *windowSurface;
    Character *person;

    unsigned long *prev;
    unsigned long *start;
    unsigned int *t1;
    unsigned int *t2;
    unsigned int *curFrame;
    int *curPauseMs;

    int *endX;
    int *endY;

    Direction curMoveDir;
    int curMoveOffset;
} MainLoopArgs;


// Returns whether an exit-related signal was sent
bool mainLoop(void *argStruct) {
    MainLoopArgs *args = (MainLoopArgs *) argStruct;

    SDL_Window *window = args -> window;
    SDL_Surface *windowSurface = args -> windowSurface;
    Character *person = args -> person;

    unsigned long *prev = args -> prev;
    unsigned long *start = args -> start;
    unsigned int *t1 = args -> t1;
    unsigned int *t2 = args -> t2;
    unsigned int *curFrame = args -> curFrame;
    int *curPauseMs = args -> curPauseMs;

    int *endX = args -> endX;
    int *endY = args -> endY;
    
    Direction curMoveDir = args -> curMoveDir;
    int curMoveOffset = args -> curMoveOffset;

    unsigned long curTime = SDL_GetTicks();


    // *curPauseMs < 0 only after the pause time is reset
    if (*curPauseMs < 0) {
        // Update counters
        *t1 += (curTime - *prev);
        *t2 += (curTime - *prev);

        // Move
        if (move(person, curMoveDir, window, windowSurface, curMoveOffset, endX, endY, curFrame, t1, t2)) {
            // Movement finished, reset variables
            *curFrame = 0;
            *t1 = 0;
            *t2 = 0;

            *endX = -1;
            *endY = -1;

            args -> curMoveDir = (Direction) (SDL_rand(LENGTH));
            args -> curMoveOffset = SDL_rand(15) + 30;

            // Setup pause between moves
            *curPauseMs = SDL_rand(500) + 1500;
            *start = SDL_GetTicks();
        }
    } else {
        // Reset pause time once finished
        if ((curTime - *start) >= *curPauseMs) {
            *curPauseMs = -1;
        }
    }

    *prev = curTime;

    
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
    unsigned long prev = 0;
    unsigned long start = 0;
    unsigned int t1 = 0;
    unsigned int t2 = 0;
    unsigned int curFrame = 0;
    int endX = -1;
    int endY = -1;
    int curPauseMs = -1;
    MainLoopArgs argStruct = {
        window,
        windowSurface,
        person,
        &prev, &start,
        &t1, &t2,
        &curFrame,
        &curPauseMs,
        &endX, &endY,
        (Direction) (SDL_rand(LENGTH)),
        SDL_rand(15) + 30,
    };
    void *argRef = &argStruct;

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(mainLoop, argRef, 0, 1);
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
