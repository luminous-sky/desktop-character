#include <stdio.h>
#include <string.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "image.h"
#include "character.h"
#include "config.h"
#include "main_args.h"


// Returns whether the entire animation finished cycling
bool move(WindowDetails *windowInfo, AnimDetails *animation, unsigned long deltaTime) {
    int displayW = windowInfo -> displayW;
    int displayH = windowInfo -> displayH;

    SDL_Window *window = windowInfo -> window;
    SDL_Surface *windowSurface = windowInfo -> windowSurface;

    Character *person = animation -> person;

    // Check this is the first animation call
    bool initCall = false;

    if (animation -> hasDrawn == false) {
        animation -> hasDrawn = true;
        initCall = true;

        int curX, curY;
        if(!SDL_GetWindowPosition(window, &curX, &curY)) {
            printf("Failed to get the window's current position.\nError: %s\n", SDL_GetError());
            return true;
        }

        
        // Set directional variables
        int curOffset = SDL_rand(15) + 30;
        animation -> curFrame = 0;
        animation -> curDir = (Direction) (SDL_rand(LENGTH));

        int tempX, tempY;
        switch (animation -> curDir) {
            case UP:
                animation -> curSprite = person -> up;
                tempX = curX;
                tempY = curY - curOffset;
                break;
            case RIGHT:
                animation -> curSprite = person -> right;
                tempX = curX + curOffset;
                tempY = curY;
                break;
            case DOWN:
                animation -> curSprite = person -> down;
                tempX = curX;
                tempY = curY + curOffset;
                break;
            case LEFT:
                animation -> curSprite = person -> left;
                tempX = curX - curOffset;
                tempY = curY;
                break;
            default:
                break;
        }

        if (tempX < 0) {
            curOffset = curX;
            tempX = 0;
        }
        if (tempY < 0) {
            curOffset = curY;
            tempY = 0;
        }
        if (tempX > displayW) {
            curOffset = displayW - curX;
            tempX = displayW;
        }
        if (tempY > displayH) {
            curOffset = displayH - curY;
            tempY = displayH;
        }
        animation -> curOffset = curOffset;

        animation -> endX = tempX;
        animation -> endY = tempY;


        // Set time counters
        TimeCounter spriteCtr = animation -> spriteCtr;
        spriteCtr.curMs = 0;
        spriteCtr.maxMs = person -> msPerFrame;
        animation -> spriteCtr = spriteCtr;

        TimeCounter moveCtr = animation -> moveCtr;
        moveCtr.curMs = 0;
        moveCtr.maxMs = (person -> msPerFrame) * (person -> len) / (curOffset);
        animation -> moveCtr = moveCtr;
    }


    // Check if the next frame should be drawn
    TimeCounter spriteCtr = animation -> spriteCtr;
    spriteCtr.curMs += deltaTime;

    bool drawNext = false;
    if (spriteCtr.curMs >= spriteCtr.maxMs) {
        (animation -> curFrame) += (spriteCtr.curMs / spriteCtr.maxMs);
        (animation -> curFrame) %= (person -> len);

        spriteCtr.curMs %= spriteCtr.maxMs;
        drawNext = true;
    }
    animation -> spriteCtr = spriteCtr;

    if (drawNext || initCall) {
        SDL_Surface **sprites = animation -> curSprite;
        if (!sprites) {
            SDL_SetError("The character's sprites failed to load.\n");
            return false;
        }

        // Clear background
        SDL_ClearSurface(windowSurface, 0.0, 0.0, 0.0, 0.0);

        // Render sprite
        int curFrame = animation -> curFrame;
        if (!SDL_BlitSurface(sprites[curFrame], NULL, windowSurface, NULL)) {
            printf("Failed to blit surface.\n%s\n", SDL_GetError());
            return 2;
        }

        SDL_UpdateWindowSurface(window);
    }


    // Check if the window should be moved
    bool moveWindow = false;
    int curX, curY;
    int signX, signY;

    switch (animation -> curDir) {
        case UP:
            signX = 0;
            signY = -1;
            break;
        case RIGHT:
            signX = 1;
            signY = 0;
            break;
        case DOWN:
            signX = 0;
            signY = 1;
            break;
        case LEFT:
            signX = -1;
            signY = 0;
            break;
        default:
            break;
    }

    TimeCounter moveCtr = animation -> moveCtr;
    moveCtr.curMs += deltaTime;

    int displacement = 0;
    if (moveCtr.curMs >= moveCtr.maxMs) {
        displacement = moveCtr.curMs / moveCtr.maxMs;
        moveCtr.curMs %= moveCtr.maxMs;
        moveWindow = true;
    }

    if (moveWindow || initCall) {
        if(!SDL_GetWindowPosition(window, &curX, &curY)) {
            printf("Failed to get the window's current position.\nError: %s\n", SDL_GetError());
            return true;
        }

        curX += (signX * displacement);
        curY += (signY * displacement);
    }
    animation -> moveCtr = moveCtr;


    // Render movement once enough time has elapsed to move >1 px
    bool finished = false;
    if (moveWindow || initCall) {
        // Only set the window's position and sync it when it moves (costly operation)
        if (!SDL_SetWindowPosition(window, curX, curY)) {
            printf("Failed to set window position.\n%s\n", SDL_GetError());
        }
        SDL_SyncWindow(window);

        if (signX * (curX - (animation -> endX)) >= 0 && signY * (curY - (animation -> endY)) >= 0) {
            finished = true;
        }
    }

    return finished;
}


// Returns whether an exit-related signal was sent
bool mainLoop(void *argStruct) {
    MainLoopArgs *args = (MainLoopArgs *) argStruct;

    unsigned long *prevTime = args -> prevTime;
    unsigned long *curTime = args -> curTime;

    *prevTime = *curTime;
    *curTime = SDL_GetTicks();
    unsigned long deltaTime = *curTime - *prevTime;


    // Check whether the main draw function should be called
    WindowDetails *winInfo = args -> window;

    AnimDetails *animation = args -> animation;
    bool shouldDraw = true;

    TimeCounter pauseCtr = animation -> pauseCtr;
    if (pauseCtr.maxMs > 0) {
        pauseCtr.curMs += deltaTime;

        if (pauseCtr.curMs >= pauseCtr.maxMs) {
            // Reset pause counter
            pauseCtr.curMs = 0;
            pauseCtr.maxMs = 0;
        } else {
            shouldDraw = false;
        }
    }

    if (shouldDraw) {
        if (move(winInfo, animation, deltaTime)) {
            // Reset animation
            animation -> hasDrawn = false;

            // Set pause counter
            pauseCtr.curMs = 0;
            pauseCtr.maxMs = SDL_rand(500) + 1500;
        }
    }
    animation -> pauseCtr = pauseCtr;

    
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
    

    const SDL_DisplayMode *display = SDL_GetCurrentDisplayMode(1);
    if (!display) {
        printf("Failed to get display.\nError: %s\n", SDL_GetError());
        return true;
    }

    // Target the monitor's frame rate to reduce CPU use
    int targetMs = 16;
    if (display -> refresh_rate > 0.0f) {
        targetMs = (int) (1000.0 / display -> refresh_rate);
    }

    // Create character
    Character *person = loadCharacter("./config.charconf", windowSurface -> format);


    // Create arg struct
    WindowDetails winStruct = (WindowDetails) {
        display -> w,
        display -> h,
        window, windowSurface,
    };

    AnimDetails animStruct = (AnimDetails) {
        person, person -> left,
        (TimeCounter) {0, 0},
        (TimeCounter) {0, 0}, 0,
        (TimeCounter) {0, 0}, LEFT, 0, 0, 0,
        false,
    };

    unsigned long prevTime = 0;
    unsigned long curTime = 0;
    MainLoopArgs argStruct = {
        &winStruct,
        &animStruct,
        &prevTime, &curTime,
    };
    void *argRef = &argStruct;

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(mainLoop, argRef, 0, 1);
    #else
    while (true) {
        if (mainLoop(argRef)) break;
        SDL_Delay(targetMs);
    }
    #endif

    // Clean up
    destroyCharacter(person);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
