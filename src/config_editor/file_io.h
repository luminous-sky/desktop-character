#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdbool.h>

// Custom file reading implementation due to Raylib's directory file function not working on Windows
typedef struct FileInfo {
    char *filename;
    bool isDir;
} FileInfo;

#ifdef _WIN32
#define SEPARATOR '\\'
#else
#define SEPARATOR '/'
#endif


bool checkExt(const char *str, const char *ext);
int countNumFiles(const char *path, const char **matchingExts, int numExts, bool includeDirs);

FileInfo **readFiles(const char *path, const char **matchingExts, int numExts, bool includeDirs, int *numFiles);
void freeFiles(FileInfo **files, int numFiles);

#endif
