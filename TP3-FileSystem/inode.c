#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"
#include "filsys.h"

#define INODES_PER_BLOCK (DISKIMG_SECTOR_SIZE / sizeof(struct inode))

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (!fs || !inp || inumber < 1 || inumber > (int)(fs->superblock.s_isize * INODES_PER_BLOCK)) {
        return -1;
    }

    int blockIndex = (inumber - 1) / INODES_PER_BLOCK;
    int blockNum = INODE_START_SECTOR + blockIndex;
    int offset = (inumber - 1) % INODES_PER_BLOCK;

    struct inode buffer[INODES_PER_BLOCK];
    if (diskimg_readsector(fs->dfd, blockNum, buffer) == -1) {
        return -1;
    }

    *inp = buffer[offset];
    return 0;
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
    if ((inp->i_mode & IALLOC) == 0 || blockNum < 0) {
        return -1;
    }

    int ptrsPerBlock = DISKIMG_SECTOR_SIZE / sizeof(uint16_t);

    uint16_t addr[9] = {0};
    for (int i = 0; i < 8; i++) {
        addr[i] = inp->i_addr[i];
    }

    const char *raw = (const char *)inp;
    size_t offset_bytes = offsetof(struct inode, i_addr) + 8 * sizeof(uint16_t);
    addr[8] = *((const uint16_t *)(raw + offset_bytes));

    if ((inp->i_mode & ILARG) == 0) {
        if (blockNum >= 8) return -1;
        return addr[blockNum];
    }

    if (blockNum < 7 * ptrsPerBlock) {
        int indBlockIdx = blockNum / ptrsPerBlock;
        int offset = blockNum % ptrsPerBlock;

        if (indBlockIdx >= 7 || addr[indBlockIdx] == 0) return -1;

        uint16_t indirect[ptrsPerBlock];
        if (diskimg_readsector(fs->dfd, addr[indBlockIdx], indirect) == -1) return -1;

        if (indirect[offset] == 0) return -1;

        return indirect[offset];
    }

    int remaining = blockNum - 7 * ptrsPerBlock;

    if (remaining >= ptrsPerBlock * ptrsPerBlock || addr[8] == 0 || addr[8] >= fs->superblock.s_fsize) {
        return -1;
    }

    int outer = remaining / ptrsPerBlock;
    int inner = remaining % ptrsPerBlock;

    uint16_t firstLevel[ptrsPerBlock];
    if (diskimg_readsector(fs->dfd, addr[8], firstLevel) == -1) return -1;

    if (firstLevel[outer] == 0 || firstLevel[outer] >= fs->superblock.s_fsize) return -1;

    uint16_t secondLevel[ptrsPerBlock];
    if (diskimg_readsector(fs->dfd, firstLevel[outer], secondLevel) == -1) return -1;

    if (secondLevel[inner] == 0) return -1;

    return secondLevel[inner];
}

int inode_getsize(struct inode *inp) {
    return (inp->i_size0 << 16) | inp->i_size1;
}