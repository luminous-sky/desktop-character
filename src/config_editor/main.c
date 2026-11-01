#include <stdio.h>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include "raylib_draw.h"
#include "file_io.h"

// Allow the preview option to be removed on unsupported platforms
#ifndef DISABLE_PREVIEW
#include <pthread.h>
#include "preview.h"
#include "preview_mem.h"
#endif

#include "../config.h"
#include "../character.h"

// Inclusive max filename length
#define MAX_FILENAME_LEN 255
#define FILE_FILTER "DIRS*;.png;.jpg;.jpeg;.bmp"


// Fixes Raylib's paths into just filenames
char *convertPathToFilename(const char *path, unsigned int baseDirLen) {
    int filenameLen = strlen(path);
    int newLen = filenameLen - baseDirLen;

    char *filename = malloc((newLen+1) * sizeof(char));
    filename[newLen] = '\0';
    memcpy(filename, path+baseDirLen, newLen);

    return filename;
}

char **getFilenames(FileInfo **files, int numFiles, unsigned int baseDirLen) {
    char **fixed = malloc(numFiles * sizeof(char *));
    for (int i = 0; i < numFiles; i++) {
        // Don't create a new string
        fixed[i] = files[i] -> filename;
    }

    return fixed;
}

// Converts absolute paths to a path relative to the current directory
char *convertAbsolutePathToRelative(const char *absPath, const char *dir, unsigned int dirLen) {
    unsigned int absPathLen = strlen(absPath);

    // Fix the working directory to end in a slash
    char dirFixed[dirLen+2];
    dirFixed[dirLen+1] = '\0';
    memcpy(dirFixed, dir, dirLen);

    if (dir[dirLen] != '/') {
        dirFixed[dirLen] = '/';
    } else {
        dirFixed[dirLen] = '\0';
    }
    
    char *res = NULL;
    
    
    // Match as much of the initial prefix as possible
    unsigned int lastSlash = 0;
    for (int i = 0; i < absPathLen && i < dirLen+1; i++) {
        if (absPath[i] == '/' || absPath[i] == '\\') {
            lastSlash = i;
            continue;
        }

        if (absPath[i] != dirFixed[i]) break;
    }

    // Find how many slashes were skipped in the directory
    unsigned int missingSlashes = 0;
    for (int i = lastSlash; i < dirLen; i++) {
        if (dir[i] != '/' && dir[i] != '\\') continue;
        missingSlashes++;
    }

    // Calculate new string length
    unsigned int newStrLen = lastSlash+1 + 3*missingSlashes + absPathLen-lastSlash;
    res = malloc((newStrLen+1) * sizeof(char));
    res[newStrLen] = '\0';

    // Copy over parts
    unsigned int cur = 0;
    res[cur] = '.';
    res[cur+1] = SEPARATOR;
    cur += 2;

    for (int i = 0; i < missingSlashes; i++) {
        res[cur] = '.';
        res[cur+1] = '.';
        res[cur+2] = SEPARATOR;
        cur += 3;
    }

    memcpy(res+cur, absPath + lastSlash+1, absPathLen - lastSlash);

    return res;
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
char workingDir[MAX_FILENAME_LEN];
unsigned int dirLen = 0;
unsigned int workingDirLen = 0;
FileInfo **files = NULL;

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
        curFilenames = getFilenames(files, numFiles, dirLen);
    } else if (dirsUpdated) {
        // Update filenames
        dirsUpdated = false;

        // Only the array needs to be freed as the FileInfo array is freed in main
        free(curFilenames);
        curFilenames = getFilenames(files, numFiles, dirLen);
    }
    
    Rectangle bounds = {35, 90 + numRows*LINE_SPACE, WIDTH - 70, HEIGHT-INIT_Y - (numRows+1)*LINE_SPACE};
    GuiSetStyle(LISTVIEW, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiListViewEx(bounds, curFilenames, numFiles, &scrollIndex, &active, &focus);

    moveUpDir = GuiButton((Rectangle) {30, 90, 40, 40}, "<");
    exitSelection = GuiButton((Rectangle) {WIDTH-30 - 40, 90, 40, 40}, "X");
}


Direction curSelected = 0;
FileButton buttons[] = {
    {NULL, "Up Spritesheet: ", NULL},
    {NULL, "Right Spritesheet: ", NULL},
    {NULL, "Down Spritesheet: ", NULL},
    {NULL, "Left Spritesheet: ", NULL},
};

// Ordered: width, height, numFrames, FPS
const int NUM_INPUTS = 4;
ValueInput inputs[] = {
    {"Width: ", 1, 255, 1, false},
    {"Height: ", 1, 255, 1, false},
    {"# of Frames: ", 1, 255, 1, false},
    {"FPS: ", 1, 255, 1, false},
};

// Thread of the preview program
#ifndef DISABLE_PREVIEW
int previewThreadId = -1;
int retVal = 0;
PreviewMem *previewMem = NULL;
PreviewArgs previewArgs = {NULL, &retVal};
pthread_t previewThread = {};
#endif


void drawMain() {
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;

    drawCenteredText("Desktop Character Editor", centerX, 48, 32, BLACK);


    switch (curMode) {
        case MAIN:
            bool selectFile = false;
            bool saveFile = false;
            bool preview = false;

            for (int i = 0; i < LENGTH; i++) {
                FileButton button = buttons[i];
                if (drawFileButton(button.defaultLabel, 30, INIT_Y + i*LINE_SPACE + i*PADDING, 28, BLACK, &button)) {
                    curSelected = i;
                    selectFile = true;
                }
            }

            int yOffset = INIT_Y + LENGTH*LINE_SPACE + LENGTH*PADDING;
            for (int i = 0; i < NUM_INPUTS; i++) {
                ValueInput input = inputs[i];
                
                bool toggled = drawValueInput(input.defaultLabel, 30, yOffset + i*LINE_SPACE + i*PADDING, 28, BLACK, &input);
                if (toggled) {
                    input.isEditing = !input.isEditing;
                }

                // Fix for pass-by-value errors
                inputs[i] = input;
            }

            yOffset += NUM_INPUTS*LINE_SPACE + NUM_INPUTS*PADDING;
            #ifndef DISABLE_PREVIEW
            saveFile = GuiButton((Rectangle) {centerX-90 - 120, yOffset, 180, 40}, "Save configuration");
            preview = GuiButton((Rectangle) {centerX-90 + 120, yOffset, 180, 40}, "See configuration");
            #else
            saveFile = GuiButton((Rectangle) {centerX-90, yOffset, 180, 40}, "Save configuration");
            #endif

            // Check if any buttons were pressed
            if (selectFile) {
                selectFile = false;
                curMode = FILE_SELECT;
            }

            if (saveFile) {
                // Save data
                saveCharacter(
                    "config.charconf",
                    buttons[UP].filePath,
                    buttons[RIGHT].filePath,
                    buttons[DOWN].filePath,
                    buttons[LEFT].filePath,
                    // Width and height
                    inputs[0].value, inputs[1].value,
                    // Number of frames, FPS
                    inputs[2].value, inputs[3].value
                );
            }

            #ifndef DISABLE_PREVIEW
            if (preview) {
                // Stop preview if it is still running
                bool success = true;
                if (previewMem) {
                    sharedMemWriteInt(previewMem, 1);

                    if (pthread_join(previewThread, NULL)) {
                        printf("An error occurred while stopping the preview (%d)\n", retVal);
                        success = false;
                    }
                } else {
                    previewMem = createPreviewMem(SEMAPHORE_NAME, BACKING_FILENAME, true);
                    if (!previewMem) {
                        printf("An error occured while starting the preview\n");
                        success = false;
                    }
                }

                if (success) {
                    // TODO: This uses a default SDL_PixelFormat as there isn't an SDL window available
                    // Make preview arguments
                    previewArgs.person = createCharacter(
                        buttons[UP].filePath,
                        buttons[RIGHT].filePath,
                        buttons[DOWN].filePath,
                        buttons[LEFT].filePath,
                        SDL_PIXELFORMAT_ARGB8888,
                        inputs[0].value, inputs[1].value,
                        inputs[2].value, inputs[3].value
                    );
                    sharedMemWriteInt(previewMem, 0);

                    previewThreadId = pthread_create(&previewThread, NULL, &showPreview, &previewArgs);
                }
            }
            #endif
            
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
    
    strcpy(workingDir, dir);
    workingDirLen = dirLen;
    
    char *fileExts[4] = {".png", ".jpg", ".jpeg", ".bmp"};
    int numFileExts = 4;

    files = readFiles(dir, fileExts, numFileExts, true, &numFiles);

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
                freeFiles(files, numFiles);

                strcpy(dir, GetPrevDirectoryPath(dir));
                dirLen = strlen(dir);
                files = readFiles(dir, fileExts, numFileExts, true, &numFiles);

                dirsUpdated = true;

                // Reset GUI state
                scrollIndex = 0;
                active = -1;
                focus = -1;
            }

            // Check selected file
            if (active >= 0 && active < numFiles) {
                // Move into selected directory
                if (files[active] -> isDir) {
                    dir[dirLen] = SEPARATOR;
                    strcpy(dir+dirLen+1, files[active] -> filename);
                    dirLen = strlen(dir);

                    // UnloadDirectoryFiles(files);
                    // files = LoadDirectoryFilesEx(dir, FILE_FILTER, false);
                    freeFiles(files, numFiles);
                    files = readFiles(dir, fileExts, numFileExts, true, &numFiles);

                    dirsUpdated = true;

                    // Reset GUI state
                    scrollIndex = 0;
                    active = -1;
                    focus = -1;
                } else {
                    // NOTE: Copies of the strings need to be made to avoid UAF from directory traversal
                    // File was selected, set the current file
                    if (buttons[curSelected].filePath) {
                        free(buttons[curSelected].filePath);
                    }

                    int filenameLen = strlen(files[active] -> filename);
                    int pathLen = dirLen + filenameLen + 1;
                    
                    char *absPath = malloc((pathLen+1) * sizeof(char));
                    absPath[pathLen] = '\0';
                    memcpy(absPath, dir, dirLen);
                    absPath[dirLen] = SEPARATOR;
                    memcpy(absPath+dirLen+1, files[active] -> filename, filenameLen);

                    buttons[curSelected].filePath = convertAbsolutePathToRelative(absPath, workingDir, workingDirLen);
                    free(absPath);
                    absPath = NULL;
                    
                    // Update the button's label
                    if (buttons[curSelected].buttonLabel) {
                        free(buttons[curSelected].buttonLabel);
                    }

                    char *newLabel = malloc((filenameLen+1) * sizeof(char));
                    newLabel[filenameLen] = '\0';
                    memcpy(newLabel, files[active] -> filename, filenameLen);

                    buttons[curSelected].buttonLabel = newLabel;


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
    if (curFilenames) {
        for (int i = 0; i < numFiles; i++) {
            free(curFilenames[i]);
        }
        free(curFilenames);
    }

    for (int i = 0; i < LENGTH; i++) {
        if (buttons[i].filePath) {
            free(buttons[i].filePath);
        }

        if (buttons[i].buttonLabel) {
            free(buttons[i].buttonLabel);
        }
    }

    #ifndef DISABLE_PREVIEW
    if (previewMem) {
        destroyPreviewMem(previewMem, true);
    }
    #endif

    CloseWindow();

    return 0;
}
