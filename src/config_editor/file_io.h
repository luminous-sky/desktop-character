#ifndef FILE_IO_H
#define FILE_IO_H

// Custom file reading implementation due to Raylib's directory file function not working on Windows
typedef struct FileInfo {
    char *filename;
    bool isDir;
} FileInfo;


bool checkExt(const char *str, char *ext);
int countNumFiles(const char *path, char **matchingExts, int numExts, bool includeDirs);

FileInfo **readFiles(const char *path, char **matchingExts, int numExts, bool includeDirs, int *numFiles);
void freeFiles(FileInfo **files, int numFiles);

#endif
