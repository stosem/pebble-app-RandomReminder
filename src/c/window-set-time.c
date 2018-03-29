#include "pebble.h"
#include "window-set-time.h"
#include "common.h"
#include "mytimer.h"


//////////////////// Local variables 

// set mode
enum Mode {
    MODE_HOUR=1,
    MODE_MIN=2,
    MODE_SEC=3
};
// start from HOUR
static uint8_t current_mode = MODE_HOUR;

char char_buff[64]; // for display time
static uint16_t mtime=0; // store time
static uint16_t mmax=0; // store max 
static uint16_t mmin=0; // store min
static NumberWindow *window;
static void (*set_function)( uint16_t ); // save value by calling this function 

//////////////////// APP 

// window init
void window_set_time_init( void ) {
		window = number_window_create("Empty",
			(NumberWindowCallbacks){.selected = selected_callback},
			NULL);
};


// deinit window
void window_set_time_deinit( void ) {
	number_window_destroy( window );
};


// show window 
// parameters: ptr fo save function, start value as current time (if set) 
// minimum time , maximum allowed time
void window_set_time_show( void (*set_fn_ptr)(uint16_t), uint16_t current_time , uint16_t min, uint16_t max ) {
    LOG( "window set time show" );
    current_mode = MODE_HOUR;
    mtime = current_time;
    mmax = (max > 0)?max:TIME_MAX;
    mmin = min;
    LOG( "mode=%d, mtime=%d, min=%d, max=%d", current_mode, mtime, mmin, mmax );
    set_function = set_fn_ptr;
    // prepare for select hour
	number_window_set_value( window, time_h( mtime ) );
	number_window_set_min( window, time_h( mmin ) );
	number_window_set_max( window, time_h( mmax )  ); 
    sntime2str( char_buff, sizeof( char_buff ), mtime, MODE_HOUR );
    strcat( char_buff, "\nset hour" );
	number_window_set_label( window, char_buff );
	window_stack_push( number_window_get_window( window ), true );
};


// select function
static void selected_callback( NumberWindow *window, void *context ){
    LOG( "select callback mode %d", current_mode );
    uint16_t tmax=0;
    uint16_t tmin=0;
    switch ( current_mode ) {
        case MODE_HOUR :
            // get hour + min + sec (maybe mtime already have value)
            //mtime = (number_window_get_value( window ) * 3600)  + (time_m(mtime) * 60) + time_s(mtime);
            mtime = (number_window_get_value( window ) * 3600); //  + (time_m(mtime) * 60) + time_s(mtime);
            // prepare for minute
            // show current time
	        number_window_set_value( window, time_m( mtime ) );
            tmin=0;
            if ( mtime < mmin) { tmin = time_m ( mmin-mtime ); }
	        number_window_set_min( window, tmin );
            // preserve uint overflow
            if ( mtime > mmax ) mtime = mmax ;
            // available minutes = left minutes or 59 if greater
            tmax = ( (mmax-mtime) < 59*60 )?time_m(mmax-mtime):59 ;
            LOG("set mtime=%d, min=%d, max=%d. Set tmin=%d, tmax=%d", mtime, mmin, mmax, tmin, tmax );
        	number_window_set_max( window, tmax );
            // show label
            sntime2str( char_buff, sizeof( char_buff ), mtime, MODE_MIN );
            strcat( char_buff, "\nset minute" );
		    number_window_set_label( window, char_buff );
            // set next mode
            current_mode++;
        break;
        case MODE_MIN :
            // get new value
            //mtime = time_h(mtime)*3600 + number_window_get_value( window ) * 60 + time_s(mtime) ;
            mtime = time_h(mtime)*3600 + number_window_get_value( window ) * 60; // + time_s(mtime) ;
            // prepare for second
    	    number_window_set_value( window, time_s( mtime ) );        
            // limit by available
            tmin=0;
            if ( mtime < mmin) { tmin = ( mmin-mtime ); }
	        number_window_set_min( window, tmin );
            // preserve uint overflow
            if ( mtime > mmax ) mtime = mmax ;
            tmax = ( (mmax-mtime) < 59 )?(mmax-mtime):59 ;
            LOG("set mtime=%d, min=%d, max=%d. Set tmin=%d, tmax=%d", mtime, mmin, mmax, tmin, tmax );
        	number_window_set_max( window, tmax );
            // show label
            sntime2str( char_buff, sizeof( char_buff ), mtime, MODE_SEC );
            strcat( char_buff, "\nset second" );
		    number_window_set_label( window, char_buff );
            // set next mode
			current_mode++;
		break;
        case MODE_SEC :		
            // get new value
            mtime = time_h(mtime)*3600 + time_m(mtime)*60 + number_window_get_value( window ) ;
            // log to me :)
            LOG( "time set to %d", mtime );
            // update value
            if ( set_function ) { 
                (*set_function)( mtime ); /// Call function by pointer, defined on show window
            };
            // close window
			window_stack_pop(true);
        break;
        default:
            LOG("ERROR: Unknown select time mode %d", current_mode );
        break;
	};
};
