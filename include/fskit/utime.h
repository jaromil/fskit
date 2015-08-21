/*
   fskit: a library for creating multi-threaded in-RAM filesystems
   Copyright (C) 2014  Jude Nelson

   This program is dual-licensed: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License version 3 or later as
   published by the Free Software Foundation. For the terms of this
   license, see LICENSE.LGPLv3+ or <http://www.gnu.org/licenses/>.

   You are free to use this program under the terms of the GNU Lesser General
   Public License, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU Lesser General Public License for more details.

   Alternatively, you are free to use this program under the terms of the
   Internet Software Consortium License, but WITHOUT ANY WARRANTY; without
   even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   For the terms of this license, see LICENSE.ISC or
   <http://www.isc.org/downloads/software-support-policy/isc-license/>.
*/

#ifndef _FSKIT_UTIME_H_
#define _FSKIT_UTIME_H_

#include <fskit/debug.h>
#include <fskit/common.h>
#include <fskit/entry.h>

FSKIT_C_LINKAGE_BEGIN 

#ifndef _UTIME_H
struct utimbuf {
   time_t actime;
   time_t modtime;
};
#endif

// time
int fskit_entry_set_ctime( struct fskit_entry* fent, struct timespec* now );
int fskit_entry_set_mtime( struct fskit_entry* fent, struct timespec* now );
int fskit_entry_set_atime( struct fskit_entry* fent, struct timespec* now );

// POSIX methods
int fskit_utime( struct fskit_core* core, char const* path, uint64_t user, uint64_t group, const struct utimbuf* times );
int fskit_utimes( struct fskit_core* core, char const* path, uint64_t user, uint64_t group, const struct timeval times[2] );

FSKIT_C_LINKAGE_END 

#endif
