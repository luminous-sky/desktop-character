#ifndef PREVIEW_H
#define PREVIEW_H

#include "../character.h"


// Used for PreviewMem
const extern char *SEMAPHORE_NAME;
const extern char *BACKING_FILENAME;

typedef struct PreviewArgs {
    Character *person;
    int *retVal;
} PreviewArgs;


void *showPreview(void *voidArgs);

#endif
