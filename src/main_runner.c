#include <SDL3/SDL.h>
#include <pthread.h>
#include <stdio.h>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "config_editor/preview_mem.h"
#include "config_editor/raylib_draw.h"
#include "anim.h"

#define MAX_CHARACTERS 5
#define MAX_FILENAME_LEN 1024


// Thread variables
PreviewMem *sharedMemory[MAX_CHARACTERS] = {NULL};
int activeThreads[MAX_CHARACTERS] = {};
pthread_t threads[MAX_CHARACTERS] = {};

// Animation arguments
AnimArgs args[MAX_CHARACTERS] = {};


// Different modes for what the window is currently showing
typedef enum WINDOW_MODE {
    MAIN,
    FILE_SELECT,
} WINDOW_MODE;
WINDOW_MODE curMode = MAIN;

// Window variables
const int WIDTH = 640;
const int HEIGHT = 360;
const int TARGET_FPS = 30;

const int CENTER_X = WIDTH / 2;
const int CENTER_Y = HEIGHT / 2;


// File listing varaibles
char dir[MAX_FILENAME_LEN];
char workingDir[MAX_FILENAME_LEN];
unsigned int dirLen = 0;
unsigned int workingDirLen = 0;

// ListView selection variables
const char *FILE_EXTS[] = {".charconf"};
const int NUM_FILE_EXTS = 1;
FileSelectState state = {
    // files, filenames, numFiles
    NULL, NULL, -1,
    // fileExts, numFileExts
    FILE_EXTS, NUM_FILE_EXTS,
    // dir, dirLen
    dir, -1,
    
    // scrollIndex, active, focus
    0, -1, -1,
    // moveUpDir, exitSelection
    false, false,
};

void drawFileSelectionFiles() {
    drawFileSelection((Rectangle) {25, 35, WIDTH-50, HEIGHT-75}, 35, 25, &state);
}


FileButton configButton = {NULL, "", NULL};

int curChars = 0;
bool startAnim = false;
bool clearAll = false;

void drawMain() {
    drawCenteredText("Desktop Character", CENTER_X, 48, 32, BLACK);

    if (curChars == MAX_CHARACTERS || !configButton.filePath) {
        GuiSetState(STATE_DISABLED);
    } else {
        GuiSetState(STATE_NORMAL);
    }
    startAnim = GuiButton((Rectangle) {CENTER_X-60 - 80, HEIGHT-40 - 32, 120, 40}, "Start Character");
    if (startAnim) {
        printf("Starting animation\n");
    }

    if (curChars == 0) {
        GuiSetState(STATE_DISABLED);
    } else {
        GuiSetState(STATE_NORMAL);
    }
    clearAll = GuiButton((Rectangle) {CENTER_X-60 + 80, HEIGHT-40 - 32, 120, 40}, "Clear All");
    
    GuiSetState(STATE_NORMAL);
    drawCenteredText("Config File:", CENTER_X, 105, 28, BLACK);
    if(drawFileButton(configButton.defaultLabel, CENTER_X - (3*32) / 2, 135, 32, BLACK, &configButton)) {
        // Disable event waiting as it causes issues in the file selector
        DisableEventWaiting();

        curMode = FILE_SELECT;
    }
}


void clearAllChars(bool destroySharedMem) {
    for (int i = 0; i < MAX_CHARACTERS; i++) {
        if (activeThreads[i] >= 0) {
            PreviewMem *mem = sharedMemory[i];
            sharedMemWriteInt(mem, 1);

            pthread_t thread = threads[i];
            if (pthread_join(thread, NULL)) {
                printf("An error occurred while stopping an animation (%s) (%d)\n", args[i].configFilename, *(args[i].retVal));
            }

            activeThreads[i] = -1;

            if (destroySharedMem) {
                destroyPreviewMem(mem, true);
                sharedMemory[i] = NULL;
            }
        }
    }
}


bool startCharacter();
void handleSelectedFile();


int retVal = 0;
int main() {
    // Set default values for thread variables
    for (int i = 0; i < MAX_CHARACTERS; i++) {
        activeThreads[i] = -1;
    }

    // Setup file selection state
    strcpy(workingDir, GetWorkingDirectory());
    workingDirLen = strlen(workingDir);

    strcpy(dir, workingDir);
    state.dirLen = workingDirLen;

    state.files = readFiles(dir, state.fileExts, state.numFileExts, true, &(state.numFiles));
    state.filenames = getFilenames(state.files, state.numFiles, state.dirLen);


    InitWindow(WIDTH, HEIGHT, "Desktop Character");
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL);

    // Idle when there are no events
    EnableEventWaiting();

    bool exitEarly = false;
    while (!WindowShouldClose() && !exitEarly) {
        // Handle main buttons
        if (curMode == MAIN) {
            if (clearAll) {
                clearAllChars(false);   
                curChars = 0;
            }

            if (startAnim) {
                exitEarly = startCharacter();
            }
        }

        // File selection section
        if (curMode == FILE_SELECT) {
            // Handle exiting
            if (state.exitSelect) {
                state.exitSelect = false;
                curMode = MAIN;

                EnableEventWaiting();
            }

            // Check selected file
            if (state.active >= 0 && state.active < state.numFiles) {
                handleSelectedFile();

                EnableEventWaiting();
            }
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        switch (curMode) {
            case MAIN:
                drawMain();
                break;
            case FILE_SELECT:
                drawFileSelectionFiles();
                break;
        }

        EndDrawing();
    }

    
    // Stop animations
    clearAllChars(true);
    CloseWindow();


    // Clear file selection variables
    if (state.filenames) {
        for (int i = 0; i < state.numFiles; i++) {
            free(state.filenames[i]);
        }
        free(state.filenames);
    }

    if (configButton.filePath) {
        free(configButton.filePath);
    }
    if (configButton.buttonLabel) {
        free(configButton.buttonLabel);
    }


    // Fully quit out of SDL as quitting SDL in subthreads causes issues with the video subsystem
    SDL_Quit();

    return 0;
}


// Returns the value of exitEarly
bool startCharacter() {
    // Find first clear space
    for (int i = 0; i < MAX_CHARACTERS; i++) {
        if (activeThreads[i] >= 0) continue;

        // Initialize memory
        if (!sharedMemory[i]) {
            // Semaphore name
            int semaphoreLen = strlen(SEMAPHORE_NAME);
            char semaphoreName[semaphoreLen+3];
            copyWithSuffix(SEMAPHORE_NAME, semaphoreName, '0' + i);

            // Backing filename
            int filenameLen = strlen(BACKING_FILENAME);
            char filename[filenameLen+3];
            copyWithSuffix(BACKING_FILENAME, filename, '0' + i);


            const char *semaphoreFinal = semaphoreName;
            const char *filenameFinal = filename;

            PreviewMem *mem = createPreviewMem(semaphoreFinal, filenameFinal, true);
            if (!mem) {
                printf("An error occured while starting the animation\n");
                return true;
            }

            sharedMemory[i] = mem;
            args[i].index = i;
        }

        args[i].configFilename = configButton.filePath;
        args[i].retVal = &retVal;

        sharedMemWriteInt(sharedMemory[i], 0);
        activeThreads[i] = pthread_create(&(threads[i]), NULL, &animMain, &(args[i]));

        curChars++;
        
        return false;
    }

    return false;
}


// Gets the selected file
void handleSelectedFile() {
    FileInfo *selected = state.files[state.active];

    if (!(selected -> isDir)) {
        // NOTE: Copies of the strings need to be made to avoid UAF from directory traversal
        // File was selected, set the current file
        if (configButton.filePath) {
            free(configButton.filePath);
        }

        int filenameLen = strlen(selected -> filename);
        int pathLen = state.dirLen + filenameLen + 1;
                    
        // Copy paths
        char *absPath = malloc((pathLen+1) * sizeof(char));
        absPath[pathLen] = '\0';

        memcpy(absPath, state.dir, state.dirLen);
        absPath[state.dirLen] = SEPARATOR;
        memcpy(absPath + (state.dirLen + 1), selected -> filename, filenameLen);

        configButton.filePath = convertAbsolutePathToRelative(absPath, workingDir, workingDirLen);
        free(absPath);
        absPath = NULL;
                    
        // Update the button's label
        if (configButton.buttonLabel) {
            free(configButton.buttonLabel);
        }

        char *newLabel = malloc((filenameLen+1) * sizeof(char));
        newLabel[filenameLen] = '\0';
        memcpy(newLabel, selected -> filename, filenameLen);

        configButton.buttonLabel = newLabel;


        // Run exit operation
        curMode = MAIN;

        state.scrollIndex = 0;
        state.active = -1;
        state.focus = -1;
    }
}
