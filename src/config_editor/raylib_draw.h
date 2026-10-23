#ifndef RAYLIB_DRAW_H
#define RAYLIB_DRAW_H

#include <raylib.h>

void drawCenteredText(char *text, int centerX, int centerY, int size, Color color);
int drawTextWrapped(char *text, int maxRowChars, int x, int y, int deltaY, int size, Color color);
int drawTextWrappedAtSlash(char *text, int maxRowChars, int x, int y, int deltaY, int size, Color color);

#endif
