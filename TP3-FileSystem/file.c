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
        fprintf(stderr, "file_getblock: inode_iget failed for inumber %d\n", inumber);
        return -1;
    }

    int fileSize = inode_getsize(&in);
    int totalBlocks = (fileSize + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

    if (blockNum < 0 || blockNum >= totalBlocks) {
        fprintf(stderr, "file_getblock: blockNum %d out of range (0..%d)\n", blockNum, totalBlocks - 1);
        return -1;
    }

    int physicalBlock = inode_indexlookup(fs, &in, blockNum);
    if (physicalBlock == -1) {
        fprintf(stderr, "file_getblock: inode_indexlookup returned -1 for inumber %d blockNum %d\n", inumber, blockNum);
        return -1;
    }

    int bytesRead = diskimg_readsector(fs->dfd, physicalBlock, buf);
    if (bytesRead == -1) {
        fprintf(stderr, "file_getblock: diskimg_readsector failed for physical block %d (inumber %d)\n", physicalBlock, inumber);
        return -1;
    }

    int remaining = fileSize - (blockNum * DISKIMG_SECTOR_SIZE);
    if (remaining >= DISKIMG_SECTOR_SIZE) {
        return DISKIMG_SECTOR_SIZE;
    }

    return remaining;
}

