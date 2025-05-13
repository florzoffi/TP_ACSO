#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

/**
 * TODO
 */
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode in;
    if (inode_iget(fs, inumber, &in) == -1) {
        return -1;
    }

    int fileSize = inode_getsize(&in);
    int totalBlocks = (fileSize + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

    if (blockNum < 0 || blockNum >= totalBlocks) {
        return -1;
    }

    int physicalBlock = inode_indexlookup(fs, &in, blockNum);
    if (physicalBlock == -1) {
        return -1;
    }

    int bytesRead = diskimg_readsector(fs->dfd, physicalBlock, buf);
    if (bytesRead == -1) {
        return -1;
    }

    int remaining = fileSize - (blockNum * DISKIMG_SECTOR_SIZE);
    if (remaining >= DISKIMG_SECTOR_SIZE) {
        return DISKIMG_SECTOR_SIZE;
    }

    return remaining;
}

