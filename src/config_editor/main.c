#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raylib_draw.h"

#include "../config.h"
#include "../character.h"

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


// Fixes Raylib's paths into just filenames
char *convertPathToFilename(const char *path, unsigned int baseDirLen) {
    int filenameLen = strlen(path);
    int newLen = filenameLen - baseDirLen;

    char *filename = malloc((newLen+1) * sizeof(char));
    filename[newLen] = '\0';
    memcpy(filename, path+baseDirLen, newLen);

    return filename;
}

char **getFilenames(FilePathList files, unsigned int baseDirLen) {
    char **fixed = malloc(files.count * sizeof(char *));
    for (int i = 0; i < files.count; i++) {
        // Create a new string for each file
        fixed[i] = convertPathToFilename(files.paths[i], baseDirLen);
    }

    return fixed;
}


// Window variables
const int WIDTH = 640;
const int HEIGHT = 480;
const int TARGET_FPS = 60;

// Text alignment variables
const int INIT_Y = 88;
const int LINE_SPACE = 24;
const int PADDING = 16;

// Different modes for what the window is currently showing
typedef enum WINDOW_MODE {
    MAIN,
    FILE_SELECT,
} WINDOW_MODE;
WINDOW_MODE curMode = MAIN;

// File listing varaibles
char dir[MAX_FILENAME_LEN];
unsigned int dirLen = 0;
FilePathList files = {};

// Current directory properties
bool dirsUpdated = false;
char **curFilenames = NULL;
int numFiles = 0;

// ListView selection variables
int scrollIndex = 0;
int active = -1;
int focus = -1;

bool exitSelection = false;
bool moveUpDir = false;


// Draws the file selection window
void drawFileSelection() {
    // Current directory
    int numRows = drawTextWrappedAtSlash(dir, 35, 83, INIT_Y, LINE_SPACE, 24, BLACK);
    
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

    moveUpDir = GuiButton((Rectangle) {30, 90, 40, 40}, "<");
    exitSelection = GuiButton((Rectangle) {WIDTH-30 - 40, 90, 40, 40}, "X");
}


bool drawFileButton(char *label, int labelX, int labelY, int labelSize, Color labelColor, FileButton *button) {
    DrawText(label, labelX, labelY, labelSize, labelColor);
    int leftOffset = MeasureText(label, labelSize);
    
    char *text = button -> buttonLabel;
    if (!text) text = "Select File";

    return GuiButton((Rectangle) {30+leftOffset, labelY, 3*labelSize, labelSize}, text);
}


Direction curSelected = 0;
FileButton buttons[] = {
    {NULL, "Up Spritesheet: ", NULL},
    {NULL, "Right Spritesheet: ", NULL},
    {NULL, "Down Spritesheet: ", NULL},
    {NULL, "Left Spritesheet: ", NULL},
};

void drawMain() {
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;

    drawCenteredText("Desktop Character Editor", centerX, 48, 32, BLACK);


    switch (curMode) {
        case MAIN:
            bool selectFile = false;

            for (int i = 0; i < LENGTH; i++) {
                FileButton button = buttons[i];
                if (drawFileButton(button.defaultLabel, 30, INIT_Y + i*LINE_SPACE + i*PADDING, 28, BLACK, &button)) {
                    curSelected = i;
                    selectFile = true;
                }
            }

            // Check if any buttons were pressed
            if (selectFile) {
                selectFile = false;
                curMode = FILE_SELECT;
            }

            break;
        case FILE_SELECT:
            drawFileSelection();
            break;
    }
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

    while (!WindowShouldClose()) {
        // File selection section
        if (curMode == FILE_SELECT) {
            // Handle exiting
            if (exitSelection) {
                exitSelection = false;
                curMode = MAIN;
            }

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

            // Check selected file
            if (active >= 0 && active < numFiles) {
                // Move into selected directory
                if (DirectoryExists(files.paths[active])) {
                    strcpy(dir, files.paths[active]);
                    dirLen = strlen(dir);

                    UnloadDirectoryFiles(files);
                    files = LoadDirectoryFilesEx(dir, FILE_FILTER, false);

                    dirsUpdated = true;

                    // Reset GUI state
                    scrollIndex = 0;
                    active = -1;
                    focus = -1;
                } else {
                    // File was selected, set the current file
                    buttons[curSelected].filePath = files.paths[active];
                    if (buttons[curSelected].buttonLabel) {
                        free(buttons[curSelected].buttonLabel);
                    }
                    buttons[curSelected].buttonLabel = convertPathToFilename(files.paths[active], dirLen);

                    // Run exit operation
                    curMode = MAIN;

                    scrollIndex = 0;
                    active = -1;
                    focus = -1;
                }
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        drawMain();

        EndDrawing();
    }

    // Clean up
    for (int i = 0; i < numFiles; i++) {
        free(curFilenames[i]);
    }
    free(curFilenames);

    for (int i = 0; i < LENGTH; i++) {
        if (buttons[i].buttonLabel) {
            free(buttons[i].buttonLabel);
        }
    }

    CloseWindow();

    return 0;
}
