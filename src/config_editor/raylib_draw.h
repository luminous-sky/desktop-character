#ifndef RAYLIB_DRAW_H
#define RAYLIB_DRAW_H

#include <raylib.h>

// Holds the button's text and the current filename
typedef struct FileButton {
    char *filePath;
    char *defaultLabel;
    char *buttonLabel;
} FileButton;


// Holds the input's text and current value
typedef struct ValueInput {
    char *defaultLabel;
    int minValue;
    int maxValue;
    int value;
    bool isEditing;
} ValueInput;


void drawCenteredText(char *text, int centerX, int centerY, int size, Color color);
int drawTextWrapped(char *text, int maxRowChars, int x, int y, int deltaY, int size, Color color);
int drawTextWrappedAtSlash(char *text, int maxRowChars, int x, int y, int deltaY, int size, Color color);

#endif
