#include <raylib.h>
#include <raygui.h>
#include <string.h>
#include <stdlib.h>

#include "file_io.h"
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


// Draws the FileButton struct
bool drawFileButton(char *label, int labelX, int labelY, int labelSize, Color labelColor, FileButton *button) {
    DrawText(label, labelX, labelY, labelSize, labelColor);
    int leftOffset = MeasureText(label, labelSize);
    
    char *text = button -> buttonLabel;
    if (!text) text = "Select File";

    return GuiButton(
        (Rectangle) {labelX+leftOffset, labelY, 3*labelSize, labelSize}, text
    );
}


// Draws the ValueInput struct
bool drawValueInput(char *label, int labelX, int labelY, int labelSize, Color labelColor, ValueInput *input) {
    DrawText(label, labelX, labelY, labelSize, labelColor);
    int leftOffset = MeasureText(label, labelSize);

    return GuiValueBox(
        (Rectangle) {labelX+leftOffset, labelY, 3*labelSize, labelSize}, "",
        &(input -> value),
        input -> minValue,
        input -> maxValue,
        input -> isEditing
    );
}


// Puts all of the filenames into a single array
char **getFilenames(FileInfo **files, int numFiles, unsigned int baseDirLen) {
    char **fixed = malloc(numFiles * sizeof(char *));
    for (int i = 0; i < numFiles; i++) {
        // Don't create a new string
        fixed[i] = files[i] -> filename;
    }

    return fixed;
}


// Draws the file selection window
void drawFileSelection(Rectangle bounds, int wrapChars, int lineSpace, FileSelectState *state) {
    // Current directory
    int numRows = drawTextWrappedAtSlash(state -> dir, wrapChars, bounds.x + 50, bounds.y, lineSpace, 24, BLACK);

    state -> moveUpDir = GuiButton((Rectangle) {bounds.x, bounds.y, 40, 40}, "<");
    state -> exitSelect = GuiButton((Rectangle) {bounds.x + bounds.width - 40, bounds.y, 40, 40}, "X");
    
    // Previous directory
    bool dirUpdated = false;
    if (state -> moveUpDir) {
        strcpy(state -> dir, GetPrevDirectoryPath(state -> dir));
        state -> dirLen = strlen(state -> dir);
        
        dirUpdated = true;
    }
    
    // Selected directory
    // Selected file handling should be done outside of the draw call
    if (state -> active >= 0 && state -> active < state -> numFiles) {
        FileInfo *selected = state -> files[state -> active];

        if (selected -> isDir) {
            state -> dir[state -> dirLen] = SEPARATOR;
            strcpy((state -> dir) + (state -> dirLen)+1, selected -> filename);
            state -> dirLen = strlen(state -> dir);

            dirUpdated = true;
        }
    }
    
    if (dirUpdated) {
        // Update files
        freeFiles(state -> files, state -> numFiles);
        state -> files = readFiles(
            state -> dir,
            state -> fileExts,
            state -> numFileExts,
            true,
            &(state -> numFiles)
        );

        // Old Raylib calls
        // UnloadDirectoryFiles(files);
        // files = LoadDirectoryFilesEx(dir, FILE_FILTER, false);

        // Only the outer array needs to be freed
        // The filename pointers are freed by freeFiles()
        free(state -> filenames);
        state -> filenames = getFilenames(
            state -> files,
            state -> numFiles,
            state -> dirLen
        );

        // Reset GUI state
        state -> scrollIndex = 0;
        state -> active = -1;
        state -> focus = -1;
    }

    Rectangle viewBounds = (Rectangle) {bounds.x, bounds.y + numRows*lineSpace, bounds.width, bounds.height - (numRows+1)*lineSpace};

    GuiSetStyle(LISTVIEW, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiListViewEx(
        viewBounds,
        state -> filenames, state -> numFiles,
        &(state -> scrollIndex), &(state -> active), &(state -> focus)
    );
}


// Converts absolute paths to a path relative to the current directory
char *convertAbsolutePathToRelative(const char *absPath, const char *dir, unsigned int dirLen) {
    unsigned int absPathLen = strlen(absPath);

    // Fix the working directory to end in a slash
    char dirFixed[dirLen+2];
    dirFixed[dirLen+1] = '\0';
    memcpy(dirFixed, dir, dirLen);

    if (dir[dirLen] != '/') {
        dirFixed[dirLen] = '/';
    } else {
        dirFixed[dirLen] = '\0';
    }
    
    char *res = NULL;
    
    
    // Match as much of the initial prefix as possible
    unsigned int lastSlash = 0;
    for (int i = 0; i < absPathLen && i < dirLen+1; i++) {
        if (absPath[i] == '/' || absPath[i] == '\\') {
            lastSlash = i;
            continue;
        }

        if (absPath[i] != dirFixed[i]) break;
    }

    // Find how many slashes were skipped in the directory
    unsigned int missingSlashes = 0;
    for (int i = lastSlash; i < dirLen; i++) {
        if (dir[i] != '/' && dir[i] != '\\') continue;
        missingSlashes++;
    }

    // Calculate new string length
    unsigned int newStrLen = lastSlash+1 + 3*missingSlashes + absPathLen-lastSlash;
    res = malloc((newStrLen+1) * sizeof(char));
    res[newStrLen] = '\0';

    // Copy over parts
    unsigned int cur = 0;
    res[cur] = '.';
    res[cur+1] = SEPARATOR;
    cur += 2;

    for (int i = 0; i < missingSlashes; i++) {
        res[cur] = '.';
        res[cur+1] = '.';
        res[cur+2] = SEPARATOR;
        cur += 3;
    }

    memcpy(res+cur, absPath + lastSlash+1, absPathLen - lastSlash);

    return res;
}
