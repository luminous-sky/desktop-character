#include <SDL3/SDL.h>
#include <pthread.h>
#include <stdio.h>

#include "config_editor/preview_mem.h"
#include "anim.h"


int main() {
    PreviewMem *previewMem = NULL;
    int previewThreadId = -1;
    pthread_t thread = {};

    int retVal = 0;
    AnimArgs args = {
        "./config.charconf",
        &retVal
    };


    bool success = true;
    if (!previewMem) {
        // Initialize shared memory
        previewMem = createPreviewMem(SEMAPHORE_NAME, BACKING_FILENAME, true);
        if (!previewMem) {
            printf("An error occured while starting the animation\n");
            success = false;
        }
    } else if (previewThreadId >= 0) {
        // Stop running animation
        sharedMemWriteInt(previewMem, 1);

        if (pthread_join(thread, NULL)) {
            printf("An error occurred while stopping the animation (%d)\n", retVal);
            success = false;
        }

        previewThreadId = -1;
    }

    if (success) {
        // Start thread
        sharedMemWriteInt(previewMem, 0);
        previewThreadId = pthread_create(&thread, NULL, &animMain, &args);
    }

    // Prevent the program from closing until an exit signal is sent
    bool exiting = false;
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                exiting = true;
                break;
            }
        }

        if (exiting) break;

        SDL_Delay(16);
    }

    // Stop animation
    if (previewThreadId >= 0) {
        sharedMemWriteInt(previewMem, 1);

        if (pthread_join(thread, NULL)) {
            printf("An error occurred while stopping the animation (%d)\n", retVal);
            success = false;
        }

        previewThreadId = -1;
    }

    if (previewMem) {
        destroyPreviewMem(previewMem, true);
    }

    return 0;
}
