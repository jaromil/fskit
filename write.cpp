/*
   fskit: a library for creating multi-threaded in-RAM filesystems
   Copyright (C) 2014  Jude Nelson

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "write.h"
#include "utime.h"
#include "route.h"

// run the user-given write route callback 
// return the number of bytes written on success
// return negative on failure
ssize_t fskit_run_user_write( struct fskit_core* core, char const* path, struct fskit_entry* fent, char* buf, size_t buflen, off_t offset ) {
   
   int rc = 0;
   int cbrc = 0;
   struct fskit_route_dispatch_args dargs;
   
   fskit_route_io_args( &dargs, buf, buflen, offset );
   
   rc = fskit_route_call_write( core, path, fent, &dargs, &cbrc );
   
   if( rc == -EPERM || rc == -ENOSYS ) {
      // can't write, so nothing written
      return 0;
   }
   
   return (ssize_t)cbrc;
}


// write up to buflen bytes into buf, starting at the given offset in the file.
// return the number of bytes written on success.
// return negative on failure.
ssize_t fskit_write( struct fskit_core* core, struct fskit_file_handle* fh, char* buf, size_t buflen, off_t offset ) {
   
   fskit_file_handle_rlock( fh );
   
   // sanity check 
   if( (fh->flags & (O_RDWR | O_WRONLY)) == 0 ) {

      fskit_file_handle_unlock( fh );
      return -EBADF;
   }
   
   ssize_t num_written = fskit_run_user_write( core, fh->path, fh->fent, buf, buflen, offset );
   
   if( num_written >= 0 ) {
      
      // update metadata
      fskit_entry_wlock( fh->fent );
      
      fskit_entry_set_mtime( fh->fent, NULL );
      fskit_entry_set_atime( fh->fent, NULL );
      
      fh->fent->size += num_written; 
      
      fskit_entry_unlock( fh->fent );
   }
   
   fskit_file_handle_unlock( fh );
   
   return num_written;
}