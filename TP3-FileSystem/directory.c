#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int directory_findname(struct unixfilesystem *fs, const char *name, int dirinumber, struct direntv6 *dirEnt) {
  struct inode in;
  if (inode_iget(fs, dirinumber, &in) == -1) {
      return -1;
  }
  if ((in.i_mode & IFMT) != IFDIR) {
      return -1;
  }
  int fileSize = inode_getsize(&in);
  int totalBlocks = (fileSize + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;
  for (int b = 0; b < totalBlocks; b++) {
      struct direntv6 block[DISKIMG_SECTOR_SIZE / sizeof(struct direntv6)];
      int bytes = file_getblock(fs, dirinumber, b, block);
      if (bytes == -1) {
          return -1;
      }
      int entries = bytes / sizeof(struct direntv6);
      for (int i = 0; i < entries; i++) {
          if (strncmp(name, block[i].d_name, sizeof(block[i].d_name)) == 0) {
              *dirEnt = block[i];
              return 0;
          }
      }
  }
  return -1; 
}
