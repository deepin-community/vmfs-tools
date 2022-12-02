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
#ifndef VMFS_VOLUME_H
#define VMFS_VOLUME_H

#include <stddef.h>

/* === Volume Info === */
#define VMFS_VOLINFO_BASE   0x100000
#define VMFS_VOLINFO_MAGIC  0xc001d00d

struct vmfs_volinfo_raw {
   uint32_t magic;
   uint32_t ver;
   u_char _unknown0[6];
   u_char lun;
   u_char _unknown1[3];
   char name[28];
   u_char _unknown2[49]; /* The beginning of this array looks like it is a LUN
                          * GUID for 3.31 * filesystems, and the LUN identifier
                          * string as given by ESX for 3.21 filesystems. */
   uint32_t size; /* Size of the physical volume, divided by 256 */
   u_char _unknown3[31];
   uuid_t uuid;
   uint64_t ctime; /* ctime? in usec */
   uint64_t mtime; /* mtime? in usec */
} __attribute__((packed));

#define VMFS_VOLINFO_OFS_MAGIC offsetof(struct vmfs_volinfo_raw, magic)
#define VMFS_VOLINFO_OFS_VER   offsetof(struct vmfs_volinfo_raw, ver)
#define VMFS_VOLINFO_OFS_LUN   offsetof(struct vmfs_volinfo_raw, lun)
#define VMFS_VOLINFO_OFS_NAME  offsetof(struct vmfs_volinfo_raw, name)
#define VMFS_VOLINFO_OFS_SIZE  offsetof(struct vmfs_volinfo_raw, size)
#define VMFS_VOLINFO_OFS_UUID  offsetof(struct vmfs_volinfo_raw, uuid)

#define VMFS_VOLINFO_OFS_NAME_SIZE \
   sizeof(((struct vmfs_volinfo_raw *)(0))->name)

/* === LVM Info === */
#define VMFS_LVMINFO_OFFSET            0x0200

struct vmfs_lvminfo_raw {
   uint64_t size;
   uint64_t blocks; /* Seems to always be sum(num_segments for all extents) +
                    * num_extents */
   uint32_t _unknown0;
   char uuid_str[35];
   u_char _unknown1[29];
   uuid_t uuid;
   uint32_t _unknown2;
   uint64_t ctime; /* ctime? in usec */
   uint32_t _unknown3;
   uint32_t num_segments;
   uint32_t first_segment;
   uint32_t _unknown4;
   uint32_t last_segment;
   uint32_t _unknown5;
   uint64_t mtime; /* mtime? in usec */
   uint32_t num_extents;
} __attribute__((packed));

#define VMFS_LVMINFO(field) \
  (VMFS_LVMINFO_OFFSET + offsetof(struct vmfs_lvminfo_raw, field))

#define VMFS_LVMINFO_OFS_SIZE          VMFS_LVMINFO(size)
#define VMFS_LVMINFO_OFS_BLKS          VMFS_LVMINFO(blocks)
#define VMFS_LVMINFO_OFS_UUID_STR      VMFS_LVMINFO(uuid_str)
#define VMFS_LVMINFO_OFS_UUID          VMFS_LVMINFO(uuid)
#define VMFS_LVMINFO_OFS_NUM_SEGMENTS  VMFS_LVMINFO(num_segments)
#define VMFS_LVMINFO_OFS_FIRST_SEGMENT VMFS_LVMINFO(first_segment)
#define VMFS_LVMINFO_OFS_LAST_SEGMENT  VMFS_LVMINFO(last_segment)
#define VMFS_LVMINFO_OFS_NUM_EXTENTS   VMFS_LVMINFO(num_extents)

/* 
 * Segment bitmap is at 0x80200.
 * Segment information are at 0x80600 + i * 0x80 for i between 0 and 
 * VMFS_LVMINFO_OFS_NUM_SEGMENTS 
 * 
 * At 0x10 (64-bits) or 0x14 (32-bits) within a segment info, it seems like 
 * something related to the absolute segment number in the logical volume 
 * (looks like absolute segment number << 4 on 32-bits).
 * Other segment information seem relative to the extent (always the same 
 * pattern on all extents) 
 */

struct vmfs_volinfo {
   uint32_t magic;
   uint32_t version;
   char *name;
   uuid_t uuid;
   int lun;

   uint32_t size;
   uint64_t lvm_size;
   uint64_t blocks;
   uuid_t lvm_uuid;
   uint32_t num_segments,
           first_segment,
           last_segment,
           num_extents;
};

/* === VMFS mounted-volume === */
struct vmfs_volume {
   vmfs_device_t dev;
   char *device;
   int fd;
   vmfs_flags_t flags;
   int is_blkdev;
   int scsi_reservation;

   /* VMFS volume base */
   off_t vmfs_base;

   /* Volume and FS information */
   vmfs_volinfo_t vol_info;
};

/* Open a VMFS volume */
vmfs_volume_t *vmfs_vol_open(const char *filename,vmfs_flags_t flags);

#endif
