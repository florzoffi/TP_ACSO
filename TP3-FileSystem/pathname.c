
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if ( fs == NULL || pathname == NULL || pathname[0] != '/' ) { return -1; }

    char pathCopy[1024];
    snprintf( pathCopy, sizeof(pathCopy), "%s", pathname );
    int currentInode = 1;
    struct direntv6 entry;
    char *token = strtok( pathCopy, "/" );

    while ( token != NULL ) {
        if ( directory_findname( fs, token, currentInode, &entry ) < 0 ) { return -1; }
        currentInode = entry.d_inumber;
        token = strtok( NULL, "/" );
    }
    
    return currentInode;
}
