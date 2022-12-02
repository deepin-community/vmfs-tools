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
/*
 * Utility functions.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <uuid.h>
#include <libgen.h>
#ifdef NO_POSIX_MEMALIGN
#include <malloc.h>
#endif

#include "utils.h"

/* Convert an UUID into a string */
char *m_uuid_to_str(const uuid_t uuid,char *str)
{
   uint32_t time_low;
   uint32_t time_mid;
   uint16_t clock_seq;
   time_low = read_le32(uuid,0);
   time_mid = read_le32(uuid,4);
   clock_seq = read_le16(uuid,8);
   sprintf(str,
      "%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
      time_low >> 24,
      time_low >> 16 & 0xff,
      time_low >> 8 & 0xff,
      time_low & 0xff,
      time_mid >> 24,
      time_mid >> 16 & 0xff,
      time_mid >> 8 & 0xff,
      time_mid & 0xff,
      clock_seq >> 8 & 0xff,
      clock_seq & 0xff,
      uuid[10],
      uuid[11],
      uuid[12],
      uuid[13],
      uuid[14],
      uuid[15]);
   return str;
}

/* Convert a timestamp to a string */
char *m_ctime(const time_t *ct,char *buf,size_t buf_len)
{
   struct tm ctm;

   localtime_r(ct,&ctm);

   snprintf(buf,buf_len,"%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
            ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday,
            ctm.tm_hour, ctm.tm_min, ctm.tm_sec);
   return buf;
}

struct fmode_info {
   u_int flag;
   char c;
   int pos;
};

static struct fmode_info fmode_flags[] = {
   { S_IFDIR, 'd', 0 },
   { S_IFLNK, 'l', 0 },
   { S_IRUSR, 'r', 1 },
   { S_IWUSR, 'w', 2 },
   { S_IXUSR, 'x', 3 },
   { S_IRGRP, 'r', 4 },
   { S_IWGRP, 'w', 5 },
   { S_IXGRP, 'x', 6 },
   { S_IROTH, 'r', 7 },
   { S_IWOTH, 'w', 8 },
   { S_IXOTH, 'x', 9 },
   { S_ISUID, 's', 3 },
   { S_ISVTX, 't', 9 },
   { 0, 0, -1, },
};

/* Convert a file mode to a string */
char *m_fmode_to_str(u_int mode,char *buf)
{
   struct fmode_info *fi;
   int i;

   for(i=0;i<10;i++) 
      buf[i] = '-';
   buf[10] = 0;

   for(i=0;fmode_flags[i].flag;i++) {
      fi = &fmode_flags[i];

      if ((mode & fi->flag) == fi->flag)
         buf[fi->pos] = fi->c;
   }

   return buf;
}

/* Count the number of bits set in a byte */
int bit_count(u_char val)
{
   static int qb[16] = {
      0, 1, 1, 2, 1, 2, 2, 3,
      1, 2, 2, 3, 2, 3, 3, 4,
   };

   return(qb[val >> 4] + qb[val & 0x0F]);
}

/* Allocate a buffer with alignment compatible for direct I/O */
u_char *iobuffer_alloc(size_t len)
{
   size_t buf_len;
   void *buf;

   buf_len = ALIGN_NUM(len,M_DIO_BLK_SIZE);

#ifdef NO_POSIX_MEMALIGN
   if (!(buf = memalign(M_DIO_BLK_SIZE,buf_len)))
#else
   if (posix_memalign((void **)&buf,M_DIO_BLK_SIZE,buf_len))
#endif
      return NULL;

   return buf;
}

/* Free a buffer previously allocated by iobuffer_alloc() */
void iobuffer_free(u_char *buf)
{
   free(buf);
}

/* Read from file descriptor at a given offset */
ssize_t m_pread(int fd,void *buf,size_t count,off_t offset)
{
   int max_retries = 10;
   u_char *ptr = (u_char *)buf;
   size_t hlen = 0;
   ssize_t len;

   while(hlen < count) {
      len = pread(fd,ptr,count-hlen,offset+hlen);
      
      if (len < 0) {
         if (errno == EIO) {
            if (max_retries-- == 0)
               return(-1);

            continue;
         }

         if (errno != EINTR)
            return(-1);
      } else {
         if (len == 0)
            break;

         hlen += len;
         ptr  += len;
      }
   }

   return(hlen);
}

/* Write to a file descriptor at a given offset */
ssize_t m_pwrite(int fd,const void *buf,size_t count,off_t offset)
{   
   int max_retries = 10;
   u_char *ptr = (u_char *)buf;
   size_t hlen = 0;
   ssize_t len;

   while(hlen < count) {
      len = pwrite(fd,ptr,count-hlen,offset+hlen);
      
      if (len < 0) {
         if (errno == EIO) {
            if (max_retries-- == 0)
               return(-1);

            continue;
         }

         if (errno != EINTR)
            return(-1);
      } else {
         if (len == 0)
            break;

         hlen += len;
         ptr  += len;
      }
   }

   return(hlen);
}

/* Returns directory name */
char *m_dirname(const char *path)
{
   char *dirc = strdup(path);
   char *dname = strdup(dirname(dirc));
   free(dirc);
   return(dname);
}

/* Returns base name */
char *m_basename(const char *path)
{
   char *basec = strdup(path);
   char *bname = strdup(basename(basec));
   free(basec);
   return(bname);
}
