#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

int file_getblock( struct unixfilesystem *fs, int inumber, int blockNum, void *buf ) {
    struct inode in;
    if ( inode_iget( fs, inumber, &in ) < 0 ) { return -1; }

    int filesize = inode_getsize( &in );
    int numBlocks = ( filesize + DISKIMG_SECTOR_SIZE - 1 ) / DISKIMG_SECTOR_SIZE;
    if ( blockNum < 0 || blockNum >= numBlocks ) { return -1; }

    memset( buf, 0, DISKIMG_SECTOR_SIZE );
    int sector = inode_indexlookup( fs, &in, blockNum );
    if ( sector == 0 || sector >= fs->superblock.s_fsize ) { return 0; }
    
    int bytes = diskimg_readsector( fs->dfd, sector, buf );
    if ( bytes != DISKIMG_SECTOR_SIZE ) { return 0; }
    if ( blockNum == numBlocks - 1 ) {
        int remainder = filesize % DISKIMG_SECTOR_SIZE;
        if ( remainder != 0 ) { return remainder; }
    }
    return DISKIMG_SECTOR_SIZE;
}