#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"
#include "filsys.h"

#define INODES_PER_BLOCK ( DISKIMG_SECTOR_SIZE / sizeof( struct inode ) )

int inode_iget( struct unixfilesystem *fs, int inumber, struct inode *inp ) {
    if ( !fs || !inp || inumber < 1 || inumber > ( int )( fs->superblock.s_isize * INODES_PER_BLOCK ) ) { return -1; }
    int blockIndex = ( inumber - 1 ) / INODES_PER_BLOCK;
    int blockNum = INODE_START_SECTOR + blockIndex;
    int offset = ( inumber - 1 ) % INODES_PER_BLOCK;
    struct inode buffer[INODES_PER_BLOCK];

    if ( diskimg_readsector( fs->dfd, blockNum, buffer ) == -1 ) { return -1; }
    *inp = buffer[offset];

    return 0;
}

int inode_indexlookup( struct unixfilesystem *fs, struct inode *inp, int blockNum ) {
    if ( ( inp->i_mode & IALLOC ) == 0 || blockNum < 0 ) { return -1; }
    int ptrsPerBlock = DISKIMG_SECTOR_SIZE / sizeof( uint16_t );

    if ( ( inp->i_mode & ILARG ) == 0 ) {
        if ( blockNum >= 8 ) { return -1; }
        return inp->i_addr[blockNum];
    }

    if ( blockNum < 7 * ptrsPerBlock ) {
        int indBlockIdx = blockNum / ptrsPerBlock;
        int offset = blockNum % ptrsPerBlock;
        if ( inp->i_addr[indBlockIdx] == 0 ) { return -1; }
        uint16_t indirect[ptrsPerBlock];
        if ( diskimg_readsector( fs->dfd, inp->i_addr[indBlockIdx], indirect ) == -1 ) { return -1; }
        return indirect[offset];
    }

    blockNum -= 7 * ptrsPerBlock;
    if ( blockNum >= ptrsPerBlock * ptrsPerBlock ) { return -1; }

    int outer = blockNum / ptrsPerBlock;
    int inner = blockNum % ptrsPerBlock;
    if ( inp->i_addr[7] == 0 ) { return -1; }

    uint16_t firstLevel[ptrsPerBlock];
    if ( diskimg_readsector( fs->dfd, inp->i_addr[7], firstLevel ) == -1 ) { return -1; }

    if ( firstLevel[outer] == 0 ) { return -1; }

    uint16_t secondLevel[ptrsPerBlock];
    if ( diskimg_readsector( fs->dfd, firstLevel[outer], secondLevel ) == -1 ) { return -1; }
    
    return secondLevel[inner];
}

int inode_getsize(struct inode *inp) {
    return ((inp->i_size0 << 16) | inp->i_size1);
}