#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    if (fs == NULL || buf == NULL || inumber < 1 || blockNum < 0) {
        return -1;
    }

    struct inode in;
    if (inode_iget(fs, inumber, &in) < 0) {
        return -1;
    }

    int fileSize = inode_getsize(&in);
    int totalBlocks = (fileSize + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

    if (blockNum >= totalBlocks) {
        return -1;
    }

    memset(buf, 0, DISKIMG_SECTOR_SIZE);

    int physicalBlock = inode_indexlookup(fs, &in, blockNum);
    if (physicalBlock <= 0 || physicalBlock >= fs->superblock.s_fsize) {
        return 0;
    }

    int readResult = diskimg_readsector(fs->dfd, physicalBlock, buf);
    if (readResult != DISKIMG_SECTOR_SIZE) {
        return 0;
    }

    int offset = blockNum * DISKIMG_SECTOR_SIZE;
    int remaining = fileSize - offset;

    return (remaining >= DISKIMG_SECTOR_SIZE) ? DISKIMG_SECTOR_SIZE : remaining;
}