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

#include <fskit/getxattr.h>
#include <fskit/path.h>

#include "fskit_private/private.h"

// wrapper around fgetxattr, which resolves the path to the fskit entry
// returns whatever fgetxattr returns, plus any errors in path resolution
int fskit_getxattr( struct fskit_core* core, char const* path, uint64_t user, uint64_t group, char const* name, char* value, size_t size ) {

   int err = 0;
   int rc = 0;

   // get the fent
   struct fskit_entry* fent = fskit_entry_resolve_path( core, path, user, group, false, &err );
   if( fent == NULL || err != 0 ) {
      return err;
   }

   // get the xattr
   rc = fskit_fgetxattr( core, fent, name, value, size );

   fskit_entry_unlock( fent );

   return rc;
}

// get an xattr value.  fent must be at least read-locked
// return the length of the attribute on success
// on error:
// * -ENOATTR if the attribute doesn't exist
// * -ERANGE if the buffer isn't big enough
// * -ENOMEM if there isn't enough memory
// * anything due to a failure in stat
// if size == 0 or value == NULL, then just return the length of the attribute requested
// NOTE: fent must be at least read-locked
int fskit_fgetxattr( struct fskit_core* core, struct fskit_entry* fent, char const* name, char* value_buf, size_t size ) {

   int rc = 0;
   char const* value = NULL;
   size_t value_len = 0;
   
   value = fskit_xattr_set_find( fent->xattrs, name, &value_len );
   if( value == NULL ) {
      
      return -ENOATTR;
   }
   
   if( value_buf == NULL || size == 0 ) {
      
      // just want size 
      return (int)value_len;
   }
   
   if( value_len > size ) {
      
      // too small 
      return -ERANGE;
   }
   
   memcpy( value_buf, value, value_len );
   return value_len;
}
