#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "file_io.h"


bool checkExt(const char *str, char *ext) {
    int extLen = strnlen(ext, 261);
    if (extLen == 261) {
        printf("File extension is not null-terminated\n");
        return false;
    }

    int strLen = strlen(str);
    if (!strcmp(str + (strLen-extLen), ext)) {
        return true;
    }

    return false;
}


// Iterates to the next directory (if includeDirs is set) or the next file with a matching file extension
bool iterNextMatch(DIR *dir, char **matchingExts, int numExts, bool includeDirs, struct dirent **curListing) {
    errno = 0;

    struct dirent *listing = readdir(dir);
    while (listing) {
        switch (listing -> d_type) {
            case DT_DIR:
                if (!includeDirs) break;

                // Exclude "." or ".." or other hidden directories
                if ((listing -> d_name)[0] == '.') break;
                
                *curListing = listing;
                return true;
            case DT_REG:
                // Match extensions
                bool found = false;
                for (int i = 0; i < numExts; i++) {
                    if (checkExt(listing -> d_name, matchingExts[i])) {
                        found = true;
                        break;
                    }
                }

                if (found) {
                    *curListing = listing;
                    return true;
                }

                break;
            default:
                break;
        }
        
        listing = readdir(dir);
    }

    // False is returned when no more matches are found
    return false;
}


int countNumFiles(const char *path, char **matchingExts, int numExts, bool includeDirs) {
    DIR *dir = opendir(path);
    if (!dir) return -1;

    int count = 0;
    struct dirent *curListing = NULL;
    while (iterNextMatch(dir, matchingExts, numExts, includeDirs, &curListing)) {
        count++;
    }

    closedir(dir);
    return count;
}


FileInfo **readFiles(const char *path, char **matchingExts, int numExts, bool includeDirs, int *numFiles) {
    DIR *dir = opendir(path);
    if (!dir) {
        *numFiles = -1;
        return NULL;
    }

    int numMatched = countNumFiles(path, matchingExts, numExts, includeDirs);
    *numFiles = numMatched;

    FileInfo **res = malloc(numMatched * sizeof(FileInfo *));
    struct dirent *curListing = NULL;

    for (int i = 0; i < numMatched; i++) {
        iterNextMatch(dir, matchingExts, numExts, includeDirs, &curListing);
        if (!curListing) {
            printf("Ended unexpectedly at %d!\n", i);
            break;
        }

        
        int strLen = strlen(curListing -> d_name);
        char *filename = malloc((strLen+1) * sizeof(char));
        filename[strLen] = '\0';

        memcpy(filename, curListing -> d_name, strLen);


        FileInfo *info = malloc(sizeof(FileInfo));
        info -> filename = filename;
        info -> isDir = (curListing -> d_type == DT_DIR);


        res[i] = info;
    }

    closedir(dir);

    return res;
}


void freeFiles(FileInfo **files, int numFiles) {
    for (int i = 0; i < numFiles; i++) {
        free(files[i] -> filename);
        free(files[i]);
    }

    free(files);
}
