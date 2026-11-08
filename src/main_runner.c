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


// Thread variables
PreviewMem *sharedMemory[MAX_CHARACTERS] = {NULL};
int threadIds[MAX_CHARACTERS] = {};
pthread_t threads[MAX_CHARACTERS] = {};

// Animation arguments
AnimArgs args[MAX_CHARACTERS] = {};


// Window variables
const int WIDTH = 480;
const int HEIGHT = 270;
const int TARGET_FPS = 60;

const int CENTER_X = WIDTH / 2;
const int CENTER_Y = HEIGHT / 2;


int curChars = 0;
bool startAnim = false;
bool clearAll = false;

void drawMain() {
    drawCenteredText("Desktop Character", CENTER_X, 48, 32, BLACK);

    if (curChars == MAX_CHARACTERS) {
        GuiSetState(STATE_DISABLED);
    } else {
        GuiSetState(STATE_NORMAL);
    }
    startAnim = GuiButton((Rectangle) {CENTER_X-60 - 80, HEIGHT-40 - 32, 120, 40}, "Start Character");
    if (startAnim) {
        printf("Pressed!\n");
    }

    if (curChars == 0) {
        GuiSetState(STATE_DISABLED);
    } else {
        GuiSetState(STATE_NORMAL);
    }
    clearAll = GuiButton((Rectangle) {CENTER_X-60 + 80, HEIGHT-40 - 32, 120, 40}, "Clear All");
}


void clearAllChars(bool destroySharedMem) {
    for (int i = 0; i < MAX_CHARACTERS; i++) {
        if (threadIds[i] >= 0) {
            PreviewMem *mem = sharedMemory[i];
            sharedMemWriteInt(mem, 1);

            pthread_t thread = threads[i];
            if (pthread_join(thread, NULL)) {
                printf("An error occurred while stopping an animation (%s) (%d)\n", args[i].configFilename, *(args[i].retVal));
            }

            threadIds[i] = -1;

            if (destroySharedMem) {
                destroyPreviewMem(mem, true);
                sharedMemory[i] = NULL;
            }
        }
    }
}


int retVal = 0;
int main() {
    // Set default values for thread variables
    for (int i = 0; i < MAX_CHARACTERS; i++) {
        threadIds[i] = -1;
    }


    InitWindow(WIDTH, HEIGHT, "Desktop Character");
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL);

    bool exitEarly = false;
    while (!WindowShouldClose() && !exitEarly) {
        // Handle buttons
        if (clearAll) {
            clearAllChars(false);   
            curChars = 0;
        }

        if (startAnim) {
            // Find first clear space
            for (int i = 0; i < MAX_CHARACTERS; i++) {
                if (threadIds[i] >= 0) continue;

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
                        exitEarly = true;
                    }

                    sharedMemory[i] = mem;
                    args[i].index = i;
                }

                args[i].configFilename = "./config.charconf";
                args[i].retVal = &retVal;
                
                sharedMemWriteInt(sharedMemory[i], 0);
                threadIds[i] = pthread_create(&(threads[i]), NULL, &animMain, &(args[i]));

                curChars++;
                break;
            }
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);
        drawMain();

        EndDrawing();
    }

    // Stop animations
    clearAllChars(true);
    CloseWindow();

    // Fully quit out of SDL as quitting SDL in subthreads causes issues with the video subsystem
    SDL_Quit();

    return 0;
}
