#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int directory_findname(struct unixfilesystem *fs, const char *name, int dir_ino, struct direntv6 *result) {
  if ( fs == NULL || name == NULL || result == NULL || dir_ino < 1 ) { return -1; }

struct inode dir_inode;
if (inode_iget(fs, dir_ino, &dir_inode) < 0) {
    return -1;
}

if ((dir_inode.i_mode & IFMT) != IFDIR) {
    return -1;
}

int size_bytes = inode_getsize(&dir_inode);
int total_blocks = (size_bytes + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

for (int blk = 0; blk < total_blocks; blk++) {
    char buffer[DISKIMG_SECTOR_SIZE];
    int read_bytes = file_getblock(fs, dir_ino, blk, buffer);
    if (read_bytes < 0) {
        return -1;
    }

    int entry_count = read_bytes / sizeof(struct direntv6);
    struct direntv6 *entries = (struct direntv6 *) buffer;

    for (int j = 0; j < entry_count; j++) {
        if (strncmp(name, entries[j].d_name, sizeof(entries[j].d_name)) == 0) {
            *result = entries[j];
            return 0;
        }
    }
}

return -1;
}
