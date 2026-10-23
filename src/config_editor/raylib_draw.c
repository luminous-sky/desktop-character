#include <raylib.h>
#include <string.h>

#include "raylib_draw.h"


// Draws Raylib text centered at a point
void drawCenteredText(char *text, int centerX, int centerY, int size, Color color) {
    int textWidth = MeasureText(text, size);
    DrawText(text, centerX - textWidth / 2, centerY - size / 2, size, color);
}


// Draws Raylib text wrapped to a certain number of characters
int drawTextWrapped(char *text, int maxRowChars, int x, int y, int deltaY, int size, Color color) {
    int textLen = strlen(text);
    int lastRow = textLen / maxRowChars;

    // Copy complete rows
    for (int i = 0; i < lastRow; i++) {
        char partial[maxRowChars+1];
        partial[maxRowChars] = '\0';
        memcpy(partial, text + (i*maxRowChars), maxRowChars);

        DrawText(partial, x, y + (i*deltaY), size, color);
    }

    // Copy last row
    char last[maxRowChars+1];
    
    int numRemaining = textLen % maxRowChars;
    last[numRemaining] = '\0';
    memcpy(last, text + (lastRow*maxRowChars), numRemaining);

    DrawText(last, x, y + (lastRow*deltaY), size, color);

    // Return how many rows were used
    return lastRow+1;
}


// Draws Raylib text except wrapped to slashes over characters
int drawTextWrappedAtSlash(char *text, int maxRowChars, int x, int y, int deltaY, int size, Color color) {
    int rowCount = 0;

    // Copy complete rows
    int prev = 0;
    int lastCur = 0;
    int cur = lastCur;
    while (text[cur]) {
        // Find last slash
        if (text[cur] == '/' || text[cur] == '\\') lastCur = cur+1;

        // Write row
        if (cur-prev >= maxRowChars) {
            int len = lastCur-prev;

            char partial[len+1];
            partial[len] = '\0';
            memcpy(partial, text + prev, len);

            DrawText(partial, x, y + (rowCount*deltaY), size, color);

            prev = lastCur;
            rowCount++;
        }

        cur++;
    }

    // Write last row if needed
    if (cur > prev) {
        int len = cur-prev;

        char partial[len+1];
        partial[len] = '\0';
        memcpy(partial, text + prev, len);

        DrawText(partial, x, y + (rowCount*deltaY), size, color);
        rowCount++;
    }

    // Return how many rows were used
    return rowCount+1;
}
