#ifndef MAIN_ARGS_H
#define MAIN_ARGS_H

typedef struct TimeCounter {
    unsigned int curMs;
    unsigned int maxMs;
} TimeCounter;

typedef struct WindowDetails {
    int displayW;
    int displayH;

    SDL_Window *window;
    SDL_Surface *windowSurface;
} WindowDetails;

typedef struct AnimDetails {
    Character *person;
    SDL_Surface **curSprite;

    TimeCounter pauseCtr;

    TimeCounter spriteCtr;
    unsigned int curFrame;
    
    TimeCounter moveCtr;
    Direction curDir;
    int curOffset;
    int endX, endY;

    bool hasDrawn;
} AnimDetails;

typedef struct MainLoopArgs {
    WindowDetails *window;
    AnimDetails *animation;

    unsigned long *prevTime;
    unsigned long *curTime;
} MainLoopArgs;

#endif
