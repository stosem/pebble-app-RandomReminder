//
// Common fucntions for all project
//
#include <pebble.h>
#include "common.h"

// Check pointer for null and assert if true
void assert(void *ptr, const char *file, int line) {
  if (ptr) {
    return;
  }
  APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid pointer: (%s:%d)", file, line);
  // assert
  void (*exit)(void) = NULL;
  exit();
};


// calculate time from sec to human string
int sntime2str_hm( char* str_ptr, uint8_t size, uint16_t time, uint8_t hide ) {
   int res=0;
   switch ( hide ) {
       case 1 : 
           res=snprintf( str_ptr, size, "_:%02d", time_m(time) );
       break;;
       case 2 :
            res=snprintf( str_ptr, size, "%02d:__", time_h(time) );
       break;;
       default:
            res=snprintf( str_ptr, size, "%02d:%02d", time_h(time), time_m(time) );
       break;;
   };
   return res;
};

