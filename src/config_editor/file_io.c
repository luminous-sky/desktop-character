#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>

#include "file_io.h"

#define MAX_PATH_LEN 1024


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
bool iterNextMatch(DIR *dir, const char *path, char **matchingExts, int numExts, bool includeDirs, struct dirent **curListing, int *filetype) {
    errno = 0;

    // Set up path buffer
    char buf[MAX_PATH_LEN] = {0};
    int dirLen = strlen(path);
    memcpy(buf, path, dirLen);
    buf[dirLen] = SEPARATOR;
    buf[dirLen+1] = '\0';
    
    struct dirent *listing = readdir(dir);
    while (listing) {
        // Exclude hidden files and directories
        if ((listing -> d_name)[0] == '.') {
            listing = readdir(dir);
            continue;
        }

        int filenameLen = strlen(listing -> d_name);
        memcpy(buf+dirLen+1, listing -> d_name, filenameLen);
        buf[dirLen+1+filenameLen] = '\0';
        
        struct stat sb;
        stat(buf, &sb);

        *filetype = sb.st_mode & S_IFMT;

        switch (*filetype) {
            case S_IFDIR:
                if (!includeDirs) break;

                // Exclude "." or ".." or other hidden directories
                if ((listing -> d_name)[0] == '.') break;
                
                *curListing = listing;
                return true;
            case S_IFREG:
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
    int filetype = 0;
    while (iterNextMatch(dir, path, matchingExts, numExts, includeDirs, &curListing, &filetype)) {
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
    
    // NOTE: This malloc call can fail if there are too many matches
    if (numMatched > INT64_MAX / sizeof(FileInfo *)) {
        closedir(dir);
        *numFiles = -1;
        return NULL;
    }
    FileInfo **res = malloc(numMatched * sizeof(FileInfo *));
    struct dirent *curListing = NULL;
    int filetype = 0;

    for (int i = 0; i < numMatched; i++) {
        iterNextMatch(dir, path, matchingExts, numExts, includeDirs, &curListing, &filetype);
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
        info -> isDir = (filetype == S_IFDIR);


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
