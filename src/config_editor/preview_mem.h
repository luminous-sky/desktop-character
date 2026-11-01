#ifndef PREVIEW_MEM_H
#define PREVIEW_MEM_H

#include <stdbool.h>

typedef struct PreviewMem {
    const char *semaphoreName;
    const char *backingFilename;
    void *extra;
} PreviewMem;

PreviewMem *createPreviewMem(const char *semaphoreName, const char *backingFilename, bool isInit);
void destroyPreviewMem(PreviewMem *mem, bool destroyAll);

void lockPreviewMem(PreviewMem *mem);
void unlockPreviewMem(PreviewMem *mem);

void sharedMemWrite(PreviewMem *mem, void *data);
void sharedMemWriteInt(PreviewMem *mem, int data);
void sharedMemRead(PreviewMem *mem, void *data);
int sharedMemReadInt(PreviewMem *mem);

#endif
