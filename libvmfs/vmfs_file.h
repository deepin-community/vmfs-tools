/*
 * vmfs-tools - Tools to access VMFS filesystems
 * Copyright (C) 2009 Christophe Fillot <cf@utc.fr>
 * Copyright (C) 2009 Mike Hommey <mh@glandium.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef VMFS_FILE_H
#define VMFS_FILE_H

#include <sys/stat.h>

/* File types (in inode and directory entries) */
#define VMFS_FILE_TYPE_DIR      0x02
#define VMFS_FILE_TYPE_FILE     0x03
#define VMFS_FILE_TYPE_SYMLINK  0x04
#define VMFS_FILE_TYPE_META     0x05
#define VMFS_FILE_TYPE_RDM      0x06

/* File flags */
#define VMFS_FILE_FLAG_RW  0x01

/* === VMFS file abstraction === */
struct vmfs_file {
   vmfs_inode_t *inode;
   u_int flags;
};

static inline const vmfs_fs_t *vmfs_file_get_fs(vmfs_file_t *f)
{
   if (f && f->inode)
      return(f->inode->fs);

   return NULL;
}

static inline mode_t vmfs_file_type2mode(uint32_t type) {
   switch (type) {
   case VMFS_FILE_TYPE_DIR:
      return S_IFDIR;
   case VMFS_FILE_TYPE_SYMLINK:
      return S_IFLNK;
   default:
      return S_IFREG;
   }
}

/* Get file size */
static inline uint64_t vmfs_file_get_size(const vmfs_file_t *f)
{
   return(f->inode->size);
}

/* Open a file based on an inode buffer */
vmfs_file_t *vmfs_file_open_from_inode(const vmfs_inode_t *inode);

/* Open a file based on a block id */
vmfs_file_t *vmfs_file_open_from_blkid(const vmfs_fs_t *fs,uint32_t blk_id);

/* Open a file */
vmfs_file_t *vmfs_file_open_at(vmfs_dir_t *dir,const char *path);

/* Create a new file entry */
int vmfs_file_create(vmfs_dir_t *d,const char *name,mode_t mode,
                     vmfs_inode_t **inode);

/* Create a file */
vmfs_file_t *vmfs_file_create_at(vmfs_dir_t *dir,const char *path,mode_t mode);

/* Close a file */
int vmfs_file_close(vmfs_file_t *f);

/* Read data from a file at the specified position */
ssize_t vmfs_file_pread(vmfs_file_t *f,u_char *buf,size_t len,off_t pos);

/* Write data to a file at the specified position */
ssize_t vmfs_file_pwrite(vmfs_file_t *f,u_char *buf,size_t len,off_t pos);

/* Dump a file */
int vmfs_file_dump(vmfs_file_t *f,off_t pos,uint64_t len,FILE *fd_out);

/* Get file status */
int vmfs_file_fstat(const vmfs_file_t *f,struct stat *buf);

/* Get file file status (follow symlink) */
int vmfs_file_stat_at(vmfs_dir_t *dir,const char *path,struct stat *buf);

/* Get file file status (do not follow symlink) */
int vmfs_file_lstat_at(vmfs_dir_t *dir,const char *path,struct stat *buf);

/* Truncate a file (using a file descriptor) */
int vmfs_file_truncate(vmfs_file_t *f,off_t length);

/* Truncate a file (using a path) */
int vmfs_file_truncate_at(vmfs_dir_t *dir,const char *path,off_t length);

/* Change permissions of a file */
int vmfs_file_chmod(vmfs_file_t *f,mode_t mode);

/* Change permissions of a file (using a path) */
int vmfs_file_chmod_at(vmfs_dir_t *dir,const char *path,mode_t mode);

/* Delete a file */
int vmfs_file_delete(vmfs_dir_t *dir,const char *name);

#endif
