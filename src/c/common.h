#pragma once
////////////////////
// Common stuff
//
/////////////////////

#define DEBUG 1

// persistent storage keys
#define MREMINDER_KEY 1
#define SETTINGS_KEY 2

#ifdef DEBUG
#define LOG(str...) app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, str); 
#else
#define LOG(str...) 
#endif

#define ASSERT(ptr) assert(ptr, __FILE__, __LINE__);


//////////////////// Useful functions
#define time_h(time) ((time) / 60)
#define time_m(time) ((time) % 60)
int sntime2str_hm( char* str_ptr, uint8_t size, uint16_t time, uint8_t hide );
void assert( void *ptr, const char *file, int line );
