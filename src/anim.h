#ifndef ANIM_H
#define ANIM_H


const extern char *SEMAPHORE_NAME;
const extern char *BACKING_FILENAME;

typedef struct AnimArgs {
    const char *configFilename;
    int *retVal;
} AnimArgs;


void *animMain(void *voidArgs);

#endif
