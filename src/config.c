#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>

#include "config.h"

#define BUF_SIZE 256


Character *loadCharacter(const char *filename, SDL_PixelFormat fmt) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        SDL_SetError("Failed to load character file");
        return NULL;
    }

    unsigned char buf[BUF_SIZE] = {0};

    // Read headers:
    // Each parameter is 1 byte read in order
    // Width, Height, Number of Frames, FPS
    const unsigned int HEADER_LEN = 4;
    if (!fread(buf, HEADER_LEN, 1, file)) {
        if (feof(file)) {
            SDL_SetError("Character file ended unexpectedly while reading headers");
        } else if (ferror(file)) {
            SDL_SetError("Error while reading character file headers");
        }

        fclose(file);
        return NULL;
    }

    
    unsigned char width = buf[0];
    unsigned char height = buf[1];
    unsigned char len = buf[2];
    unsigned char fps = buf[3];
    unsigned int msPerFrame = 1000 / fps;

    
    // Read filenames:
    // Each filename is 1 byte for the length and then the filename itself
    // Filenames are directioned, in order: up, right, down, left
    char *up = NULL;
    char *right = NULL;
    char *down = NULL;
    char *left = NULL;

    Direction dir = UP;
    fread(buf, BUF_SIZE, 1, file);
    if (ferror(file)) {
        SDL_SetError("Error while reading character filenames");
        return NULL;
    }
    
    // Iterate through all directions
    int cur = 0;
    while (dir < LENGTH) {
        // Read more as necessary
        if (cur >= BUF_SIZE) {
            if (feof(file)) break;
            fread(buf, BUF_SIZE, 1, file);
            cur = 0;
        }

        unsigned char filenameLen = buf[cur];
        cur++;

        // Get pointer to string
        char **ptr = NULL;
        switch (dir) {
            case UP:
                ptr = &up;
                break;
            case RIGHT:
                ptr = &right;
                break;
            case DOWN:
                ptr = &down;
                break;
            case LEFT:
                ptr = &left;
                break;
            default:
                break;
        }
        dir++;

        *ptr = malloc(filenameLen+1 * sizeof(char));
        (*ptr)[filenameLen] = '\0';
            
        if (BUF_SIZE-cur < filenameLen) {
            // Load partial portion, then read the rest
            int partial = BUF_SIZE-cur;
            memcpy(*ptr, buf+cur, partial);
            cur = 0;

            // Verify file can still be read
            if (ferror(file) || feof(file)) {
                SDL_SetError("Failed to finish reading character filename");
                if (up) free(up);
                if (right) free(right);
                if (down) free(down);
                if (left) free(left);
                return NULL;
            }

            fread(buf, filenameLen-partial, 1, file);
            memcpy(*ptr, buf+cur, filenameLen-partial);
        } else {
            // Read whole filename
            memcpy(*ptr, buf+cur, filenameLen);
            cur += filenameLen;
        }
    }

    // Create character
    Character *person = createCharacter(up, right, down, left, fmt, width, height, len, msPerFrame);
    free(up);
    free(right);
    free(down);
    free(left);

    return person;
}
