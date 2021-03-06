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

#include "common.h"

int fskit_test_readdir( struct fskit_core* core, char const* path ) {

   int rc = 0;
   struct fskit_dir_handle* dh = fskit_opendir( core, path, 0, 0, &rc );

   if( rc != 0 ) {
      fskit_error("fskit_opendir('%s') rc = %d\n", path, rc );
      return rc;
   }

   int num_to_read = 1;
   int offset = 0;
   uint64_t num_read = 0;
   int count = 0;
   char type_str[10];
   struct fskit_dir_entry** dents;


   fskit_debug("In directory %s:\n", path);

   // read 1, then 2, then 3, etc.
   while( true ) {

      dents = fskit_readdir( core, dh, offset, num_to_read, &num_read, &rc );

      if( rc != 0 ) {
         fskit_error("fskit_readdir('%s') rc = %d\n", path, rc );
         return rc;
      }

      if( num_read == 0 ) {
         break;
      }

      for( uint64_t i = 0; i < num_read; i++ ) {

         fskit_type_to_string( dents[i]->type, type_str );

         fskit_debug("   %s: %" PRIX64 " %s\n", type_str, dents[i]->file_id, dents[i]->name );
      }

      fskit_debug("%s", "\n");

      fskit_dir_entry_free_list( dents );
      dents = NULL;

      offset += num_read;
      num_to_read ++;
      count++;
   }

   fskit_debug("Read %d entries\n", count );

   rc = fskit_closedir( core, dh );
   if( rc != 0 ) {
      fskit_error("fskit_closedir('%s') rc = %d\n", path, rc );
   }

   return rc;
}


int main( int argc, char** argv ) {

   struct fskit_core core;
   int rc;
   void* output;

   rc = fskit_test_begin( &core, NULL );
   if( rc != 0 ) {
      exit(1);
   }

   rc = fskit_test_mkdir_LR_recursive( &core, "/root", 2 );
   if( rc != 0 ) {
      fskit_error("fskit_test_mkdir_LR_recursive('/root') rc = %d\n", rc );
      exit(1);
   }


   rc = fskit_test_readdir( &core, "/root" );
   if( rc != 0 ) {
      fskit_error("fskit_test_readdir('/root') rc = %d\n", rc );
      exit(1);
   }

   rc = fskit_test_readdir( &core, "/root/L" );
   if( rc != 0 ) {
      fskit_error("fskit_test_readdir('/root') rc = %d\n", rc );
      exit(1);
   }

   rc = fskit_test_readdir( &core, "/root/R" );
   if( rc != 0 ) {
      fskit_error("fskit_test_readdir('/root') rc = %d\n", rc );
      exit(1);
   }

   fskit_print_tree( stdout, &core.root );

   fskit_test_end( &core, &output );

   return 0;
}
