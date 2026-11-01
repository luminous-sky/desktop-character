#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#include <stdio.h>

#include "preview_mem.h"


// Internal struct to cache pointers to the objects
typedef struct MemPtrs {
    sem_t *semptr;

    int fileFd;
    int *memptr;
    int memBytes;
} MemPtrs;


PreviewMem *createPreviewMem(const char *semaphoreName, const char *backingFilename, bool isInit) {
    PreviewMem *previewMem = malloc(sizeof(PreviewMem));
    previewMem -> semaphoreName = semaphoreName;
    previewMem -> backingFilename = backingFilename;

    MemPtrs *ptrs = malloc(sizeof(MemPtrs));
    previewMem -> extra = ptrs;


    int sharedMemFlags = O_RDWR;
    if (isInit) {
        sharedMemFlags |= (O_CREAT);
    }
    
    int fd = shm_open(backingFilename, sharedMemFlags, 0644);
    if (fd == -1) {
        printf("Failed to open backing file (%d)\n", errno);
        destroyPreviewMem(previewMem, true);
        return NULL;
    }
    ptrs -> fileFd = fd;

    // Don't forget to ftruncate the file to avoid SIGBUS
    ptrs -> memBytes = sizeof(int);
    if (isInit) {
        ftruncate(fd, ptrs -> memBytes);
    }

    // Pointer to an int value
    int *sharedMem = mmap(NULL, ptrs -> memBytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (sharedMem == MAP_FAILED) {
        printf("Failed to mmap memory (%d)\n", errno);
        destroyPreviewMem(previewMem, true);
        return NULL;
    }
    ptrs -> memptr = sharedMem;
    *(sharedMem) = 0;

    sem_t *semptr = sem_open(semaphoreName, O_CREAT, 0644, 1);
    if (semptr == SEM_FAILED) {
        printf("Failed to create semaphore (%d)\n", errno);
        destroyPreviewMem(previewMem, true);
        return NULL;
    }
    ptrs -> semptr = semptr;

    return previewMem;
}

void destroyPreviewMem(PreviewMem *mem, bool destroyAll) {
    if (mem -> extra) {
        MemPtrs *ptrs = mem -> extra;

        if (ptrs -> memptr) {
            munmap(ptrs -> memptr, ptrs -> memBytes);
        }
        
        close(ptrs -> fileFd);
        
        if (ptrs -> semptr) {
            sem_close(ptrs -> semptr);
            if (destroyAll) sem_unlink(mem -> semaphoreName);
        }

        free(mem -> extra);
    }
    
    unlink(mem -> backingFilename);
    if (destroyAll) shm_unlink(mem -> backingFilename);

    free(mem);
}


// This operation is blocking
void lockPreviewMem(PreviewMem *mem) {
    MemPtrs *ptrs = mem -> extra;
    sem_wait(ptrs -> semptr);
}

void unlockPreviewMem(PreviewMem *mem) {
    MemPtrs *ptrs = mem -> extra;
    sem_post(ptrs -> semptr);
}


// Both read and write operations lock the semaphore, so they are both blocking
// Data is assumed to be an integer value
void sharedMemWrite(PreviewMem *mem, void *data) {
    lockPreviewMem(mem);

    MemPtrs *ptrs = mem -> extra;
    int *convData = (int *) data;
    *(ptrs -> memptr) = *(convData);

    unlockPreviewMem(mem);
}

void sharedMemWriteInt(PreviewMem *mem, int data) {
    sharedMemWrite(mem, &data);
}

void sharedMemRead(PreviewMem *mem, void *data) {
    lockPreviewMem(mem);

    MemPtrs *ptrs = mem -> extra;
    int *convData = (int *) data;
    *(convData) = *(ptrs -> memptr);

    unlockPreviewMem(mem);
}

int sharedMemReadInt(PreviewMem *mem) {
    int res = 0;
    sharedMemRead(mem, &res);

    return res;
}
