#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "../config.h"

// Inclusive max filename length
#define MAX_FILENAME_LEN 255
#define FILE_FILTER "DIRS*;.png;.jpg;.jpeg;.bmp"


// Returns whether the exit event was sent during the movement (from main.c)
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


const int WIDTH = 640;
const int HEIGHT = 480;
const int TARGET_FPS = 10;

// Draws Raylib text centered at a point
void drawCenteredText(char *text, int centerX, int centerY, int size, Color color) {
    int textWidth = MeasureText(text, size);
    DrawText(text, centerX - textWidth / 2, centerY - size / 2, size, color);
}


// Draws Raylib text wrapped to a certain number of characters
int drawTextWrapped(char *text, int maxRowChars, int x, int y, int deltaY, int size, Color color) {
    int textLen = strlen(text);
    int lastRow = textLen / maxRowChars;

    // Copy complete rows
    for (int i = 0; i < lastRow; i++) {
        char partial[maxRowChars+1];
        partial[maxRowChars] = '\0';
        memcpy(partial, text + (i*maxRowChars), maxRowChars);

        DrawText(partial, x, y + (i*deltaY), size, color);
    }

    // Copy last row
    char last[maxRowChars+1];
    
    int numRemaining = textLen % maxRowChars;
    last[numRemaining] = '\0';
    memcpy(last, text + (lastRow*maxRowChars), numRemaining);

    DrawText(last, x, y + (lastRow*deltaY), size, color);

    // Return how many rows were used
    return lastRow+1;
}


// Draws Raylib text except wrapped to slashes over characters
int drawTextWrappedAtSlash(char *text, int maxRowChars, int x, int y, int deltaY, int size, Color color) {
    int rowCount = 0;

    // Copy complete rows
    int prev = 0;
    int lastCur = 0;
    int cur = lastCur;
    while (text[cur]) {
        // Find last slash
        if (text[cur] == '/' || text[cur] == '\\') lastCur = cur+1;

        // Write row
        if (cur-prev >= maxRowChars) {
            int len = lastCur-prev;

            char partial[len+1];
            partial[len] = '\0';
            memcpy(partial, text + prev, len);

            DrawText(partial, x, y + (rowCount*deltaY), size, color);

            prev = lastCur;
            rowCount++;
        }

        cur++;
    }

    // Write last row if needed
    if (cur > prev) {
        int len = cur-prev;

        char partial[len+1];
        partial[len] = '\0';
        memcpy(partial, text + prev, len);

        DrawText(partial, x, y + (rowCount*deltaY), size, color);
        rowCount++;
    }

    // Return how many rows were used
    return rowCount+1;
}


// Draws the main window text
const int INIT_Y = 88;
const int LINE_SPACE = 24;

char dir[MAX_FILENAME_LEN];
unsigned int dirLen = 0;
FilePathList files = {};


char **getFilenames(FilePathList files, unsigned int baseDirLen) {
    char **fixed = malloc(files.count * sizeof(char *));
    for (int i = 0; i < files.count; i++) {
        // Create a new string for each file
        int filenameLen = strlen(files.paths[i]);
        int newLen = filenameLen - baseDirLen;

        char *filename = malloc((newLen+1) * sizeof(char));
        filename[newLen] = '\0';
        memcpy(filename, files.paths[i]+baseDirLen, newLen);

        fixed[i] = filename;
    }

    return fixed;
}


bool dirsUpdated = false;
char **curFilenames = NULL;
int numFiles = 0;

int scrollIndex = 0;
int active = -1;
int focus = -1;
void drawMain() {
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;

    drawCenteredText("Desktop Character Editor", centerX, 48, 32, BLACK);

    int numRows = drawTextWrappedAtSlash(dir, 35, 90, INIT_Y, LINE_SPACE, 24, BLACK);
    
    if (!curFilenames) {
        // Get first filenames
        curFilenames = getFilenames(files, dirLen);
        numFiles = files.count;
    } else if (dirsUpdated) {
        // Update filenames
        dirsUpdated = false;

        for (int i = 0; i < numFiles; i++) {
            free(curFilenames[i]);
        }
        free(curFilenames);

        curFilenames = getFilenames(files, dirLen);
        numFiles = files.count;
    }
    
    Rectangle bounds = {35, 90 + numRows*LINE_SPACE, WIDTH - 70, HEIGHT-INIT_Y - (numRows+1)*LINE_SPACE};
    GuiSetStyle(LISTVIEW, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiListViewEx(bounds, curFilenames, numFiles, &scrollIndex, &active, &focus);
}


int main() {
    // Create main Raylib window
    InitWindow(WIDTH, HEIGHT, "Desktop Character Editor");
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL);

    // Main loop
    strcpy(dir, GetWorkingDirectory());
    dirLen = strlen(dir);
    
    files = LoadDirectoryFilesEx(dir, FILE_FILTER, false);

    bool moveUpDir = false;
    while (!WindowShouldClose()) {
        // Handle back button directory changes
        if (moveUpDir) {
            UnloadDirectoryFiles(files);

            strcpy(dir, GetPrevDirectoryPath(dir));
            dirLen = strlen(dir);
            files = LoadDirectoryFilesEx(dir, FILE_FILTER, false);

            dirsUpdated = true;

            // Reset GUI state
            scrollIndex = 0;
            active = -1;
            focus = -1;
        }

        // Move into selected directory
        if (active >= 0 && active < numFiles && DirectoryExists(files.paths[active])) {
            strcpy(dir, files.paths[active]);
            dirLen = strlen(dir);

            UnloadDirectoryFiles(files);
            files = LoadDirectoryFilesEx(dir, FILE_FILTER, false);

            dirsUpdated = true;

            // Reset GUI state
            scrollIndex = 0;
            active = -1;
            focus = -1;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        drawMain();

        moveUpDir = GuiButton((Rectangle) {30, 90, 40, 40}, "<");

        EndDrawing();
    }

    // Clean up
    for (int i = 0; i < numFiles; i++) {
        free(curFilenames[i]);
    }
    free(curFilenames);

    CloseWindow();

    return 0;
}
