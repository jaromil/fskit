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

#include <fskit/link.h>
#include <fskit/path.h>
#include <fskit/fskit.h>
#include <fskit/util.h>

#include "fskit_private/private.h"

// link the inode at "from" to the location "to".  Increment its link count on success.
// return 0 on success.
// return -ENOMEM if out-of-memory
// return -EEXIST if 'to' refers to an existing path
// return -EACCES if 'to's parent cannot be written to
// return -EPERM if 'from' is a directory
// return the usual path resolution errors if path resolution fails in any way.
int fskit_link( struct fskit_core* core, char const* from, char const* to, uint64_t uid, uint64_t gid ) {

   int err = 0;
   struct fskit_entry* from_fent = NULL;
   struct fskit_entry* to_parent_fent = NULL;
   struct fskit_entry* to_fent = NULL;
   char to_parent[ PATH_MAX+1 ];
   char to_child[ FSKIT_FILESYSTEM_NAMEMAX+1 ];

   memset( to_parent, 0, PATH_MAX+1 );

   fskit_dirname( to, to_parent );
   fskit_basename( to, to_child );

   // find 'from'
   from_fent = fskit_entry_resolve_path( core, from, uid, gid, true, &err );
   if( from_fent == NULL || err != 0 ) {
      return err;
   }

   // can't be a directory
   if( from_fent->type == FSKIT_ENTRY_TYPE_DIR ) {

      fskit_entry_unlock( from_fent );
      return -EPERM;
   }

   // find parent of 'to'
   to_parent_fent = fskit_entry_resolve_path( core, to_parent, uid, gid, true, &err );
   if( to_parent_fent == NULL || err != 0 ) {

      fskit_entry_unlock( from_fent );
      return err;
   }

   // directory?
   if( to_parent_fent->type != FSKIT_ENTRY_TYPE_DIR ) {

      fskit_entry_unlock( to_parent_fent );
      fskit_entry_unlock( from_fent );
      return -ENOTDIR;
   }

   // writable?
   if( !FSKIT_ENTRY_IS_WRITEABLE( to_parent_fent->mode, to_parent_fent->owner, to_parent_fent->group, uid, gid ) ) {

      fskit_entry_unlock( to_parent_fent );
      fskit_entry_unlock( from_fent );
      return -EACCES;
   }

   // does the requested child exist in the parent of 'to'?
   to_fent = fskit_entry_set_find_name( to_parent_fent->children, to_child );
   if( to_fent != NULL ) {

      // exists
      fskit_entry_unlock( to_parent_fent );
      fskit_entry_unlock( from_fent );
      return -EEXIST;
   }

   // create the child as a hardlink
   fskit_entry_attach_lowlevel_ex( to_parent_fent, from_fent, to_child );

   fskit_entry_unlock( to_parent_fent );
   fskit_entry_unlock( from_fent );

   return 0;
}
