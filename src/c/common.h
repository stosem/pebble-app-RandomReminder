////////////////////
// Common stuff
//
/////////////////////

//#define DEBUG 1

#ifdef DEBUG
#define LOG(str...) app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, str); 
#else
#define LOG(str...) 
#endif

#define ASSERT(ptr) assert(ptr, __FILE__, __LINE__);

#define TIME_MIN 2
// max 18 hours in uint16_t
#define TIME_MAX 12*3600
#define TIME_DEFAULT 60

//////////////////// Useful functions
#define time_h(time) ((time) / 3600)
#define time_m(time) ((time) % 3600 / 60)
#define time_s(time) ((time) % 3600 % 60)
int sntime2str( char* str_ptr, uint8_t size, uint16_t time, uint8_t hide );
void assert( void *ptr, const char *file, int line );
