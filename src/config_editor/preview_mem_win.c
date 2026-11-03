#include <windows.h>
#include <conio.h>

#include <stdio.h>

#include "preview_mem.h"


// Internal struct to cache pointers to the objects
typedef struct MemPtrs {
    HANDLE semaphore;

    HANDLE mappedFile;
    int *memptr;
    int memBytes;
} MemPtrs;


PreviewMem *createPreviewMem(const char *semaphoreName, const char *backingFilename, bool isInit) {
    PreviewMem *previewMem = malloc(sizeof(PreviewMem));
    previewMem -> semaphoreName = semaphoreName;
    previewMem -> backingFilename = backingFilename;

    MemPtrs *ptrs = malloc(sizeof(MemPtrs));
    previewMem -> extra = ptrs;

    ptrs -> memBytes = sizeof(int);
    HANDLE mappedFile = NULL;
    if (isInit) {
        mappedFile = CreateFileMappingA(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0, ptrs -> memBytes,
            backingFilename
        );
    } else {
        mappedFile = OpenFileMappingA(
            FILE_MAP_READ,
            false,
            backingFilename
        );
    }

    if (!mappedFile) {
        printf("Failed to open backing file (%lu)\n", GetLastError());
        destroyPreviewMem(previewMem, true);
        return NULL;
    }
    ptrs -> mappedFile = mappedFile;


    DWORD viewFlags = 0;
    if (isInit) {
        viewFlags = FILE_MAP_ALL_ACCESS;
    } else {
        viewFlags = FILE_MAP_READ;
    }

    int *memptr = MapViewOfFile(mappedFile, viewFlags, 0, 0, ptrs -> memBytes);
    if (!memptr) {
        printf("Failed to map view of file (%lu)\n", GetLastError());
        destroyPreviewMem(previewMem, true);
        return NULL;
    }
    ptrs -> memptr = memptr;
    if (isInit) {
        *(memptr) = 0;
    }


    HANDLE semaphore = NULL;
    if (isInit) {
        semaphore = CreateSemaphoreA(NULL, 1, 1, semaphoreName);
    } else {
        DWORD flags = READ_CONTROL | SYNCHRONIZE | SEMAPHORE_MODIFY_STATE;
        semaphore = OpenSemaphoreA(flags, false, semaphoreName);
    }
    if (!semaphore) {
        printf("Failed to create semaphore (%lu)\n", GetLastError());
        destroyPreviewMem(previewMem, true);
        return NULL;
    }
    ptrs -> semaphore = semaphore;


    return previewMem;
}

void destroyPreviewMem(PreviewMem *mem, bool _destroyAll) {
    if (mem -> extra) {
        MemPtrs *ptrs = mem -> extra;

        if (ptrs -> memptr) {
            UnmapViewOfFile(ptrs -> memptr);
        }

        if (ptrs -> mappedFile) {
            CloseHandle(ptrs -> mappedFile);
        }

        // The semaphore is automatically closed once all handles are closed
        if (ptrs -> semaphore) {
            CloseHandle(ptrs -> semaphore);
        }

        free(mem -> extra);
    }

    free(mem);
}


// This operation is blocking as SYNCHRONIZE is set
void lockPreviewMem(PreviewMem *mem) {
    MemPtrs *ptrs = mem -> extra;
    WaitForSingleObject(ptrs -> semaphore, INFINITE);
    printf("Locked semaphore\n");
}

void unlockPreviewMem(PreviewMem *mem) {
    MemPtrs *ptrs = mem -> extra;
    if (!ReleaseSemaphore(ptrs -> semaphore, 1, NULL)) {
        printf("Failed to release semaphore\n");
    }
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
