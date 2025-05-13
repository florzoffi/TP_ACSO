#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"
#include "filsys.h"

#define INODES_PER_BLOCK (DISKIMG_SECTOR_SIZE / sizeof(struct inode))

/**
 * TODO
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1 || inumber > fs->superblock.s_isize * INODES_PER_BLOCK) {
        return -1;
    }

    int inodeIndex = inumber - 1;
    int blockNum = INODE_START_SECTOR + inodeIndex / INODES_PER_BLOCK;

    struct inode inodes[INODES_PER_BLOCK];
    int bytesRead = diskimg_readsector(fs->dfd, blockNum, inodes);
    if (bytesRead == -1) {
        perror("Error leyendo sector de inodo");
        return -1;
    }

    *inp = inodes[inodeIndex % INODES_PER_BLOCK];
    return 0;
}

/**
 * TODO
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
    if ((inp->i_mode & IALLOC) == 0) {
        return -1;
    }

    if ((inp->i_mode & ILARG) == 0) {
        if (blockNum < 0 || blockNum >= 8) {
            return -1;
        }
        return inp->i_addr[blockNum];
    }

    int indirectPtrsPerBlock = DISKIMG_SECTOR_SIZE / sizeof(unsigned int);

    if (blockNum < 7 * indirectPtrsPerBlock) {
        int indirectBlockIndex = blockNum / indirectPtrsPerBlock;
        int offset = blockNum % indirectPtrsPerBlock;

        if (indirectBlockIndex >= 7) {
            return -1;
        }

        unsigned int pointers[indirectPtrsPerBlock];
        int bytesRead = diskimg_readsector(fs->dfd, inp->i_addr[indirectBlockIndex], pointers);
        if (bytesRead == -1) {
            return -1;
        }

        return pointers[offset];
    }

    int remaining = blockNum - 7 * indirectPtrsPerBlock;

    if (remaining >= indirectPtrsPerBlock * indirectPtrsPerBlock) {
        return -1;
    }

    int first = remaining / indirectPtrsPerBlock;
    int second = remaining % indirectPtrsPerBlock;

    unsigned int firstLevel[indirectPtrsPerBlock];
    int bytesRead = diskimg_readsector(fs->dfd, inp->i_addr[8], firstLevel);
    if (bytesRead == -1) {
        return -1;
    }

    unsigned int secondLevel[indirectPtrsPerBlock];
    bytesRead = diskimg_readsector(fs->dfd, firstLevel[first], secondLevel);
    if (bytesRead == -1) {
        return -1;
    }

    return secondLevel[second];
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
