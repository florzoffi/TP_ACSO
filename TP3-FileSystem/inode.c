#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"
#include "filsys.h"

#define INODES_PER_BLOCK (DISKIMG_SECTOR_SIZE / sizeof(struct inode))

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1 || inumber > fs->superblock.s_isize * INODES_PER_BLOCK) {
        return -1;
    }

    int inodeIndex = inumber - 1;
    int blockNum = INODE_START_SECTOR + inodeIndex / INODES_PER_BLOCK;

    struct inode inodes[INODES_PER_BLOCK];
    int bytesRead = diskimg_readsector(fs->dfd, blockNum, inodes);
    if (bytesRead == -1) {
        return -1;
    }

    *inp = inodes[inodeIndex % INODES_PER_BLOCK];
    return 0;
}

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

    int ptrsPerBlock = DISKIMG_SECTOR_SIZE / sizeof(unsigned int);

    if (blockNum < 7 * ptrsPerBlock) {
        int indBlockIdx = blockNum / ptrsPerBlock;
        int offset = blockNum % ptrsPerBlock;

        if (indBlockIdx >= 7) {
            return -1;
        }

        if (inp->i_addr[indBlockIdx] == 0) {
            return -1;
        }

        unsigned int indirect[ptrsPerBlock];
        int bytesRead = diskimg_readsector(fs->dfd, inp->i_addr[indBlockIdx], indirect);
        if (bytesRead == -1) {
            return -1;
        }

        return indirect[offset];
    }

    int remaining = blockNum - 7 * ptrsPerBlock;
    if (remaining >= ptrsPerBlock * ptrsPerBlock) {
        return -1;
    }

    int outer = remaining / ptrsPerBlock;
    int inner = remaining % ptrsPerBlock;

    if (inp->i_addr[8] == 0) {
        return -1;
    }

    unsigned int firstLevel[ptrsPerBlock];
    int bytesRead = diskimg_readsector(fs->dfd, inp->i_addr[8], firstLevel);
    if (bytesRead == -1) {
        return -1;
    }

    if (firstLevel[outer] == 0) {
        return -1;
    }

    unsigned int secondLevel[ptrsPerBlock];
    bytesRead = diskimg_readsector(fs->dfd, firstLevel[outer], secondLevel);
    if (bytesRead == -1) {
        return -1;
    }

    return secondLevel[inner];
}

int inode_getsize(struct inode *inp) {
    return ((inp->i_size0 << 16) | inp->i_size1); 
}