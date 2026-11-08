#ifndef ANIM_H
#define ANIM_H


const extern char *SEMAPHORE_NAME;
const extern char *BACKING_FILENAME;

typedef struct AnimArgs {
    int index;

    const char *configFilename;
    int *retVal;
} AnimArgs;


// Buffer should be length (strlen(str)+2) + 1
void copyWithSuffix(const char *str, char *buf, char suffix);

void *animMain(void *voidArgs);

#endif
