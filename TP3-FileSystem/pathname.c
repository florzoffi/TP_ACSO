
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (pathname == NULL || pathname[0] != '/') {
        return -1; 
    }

    char pathCopy[strlen(pathname) + 1];
    strcpy(pathCopy, pathname);

    int currentInode = 1;

    char *token = strtok(pathCopy, "/");
    while (token != NULL) {
        struct direntv6 entry;
        if (directory_findname(fs, token, currentInode, &entry) == -1) {
            return -1;
        }

        currentInode = entry.d_inumber;
        token = strtok(NULL, "/");
    }

    return currentInode;
}
