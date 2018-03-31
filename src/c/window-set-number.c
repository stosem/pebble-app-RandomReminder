#include "pebble.h"
#include "window-set-number.h"
#include "common.h"


//////////////////// Local variables 

char char_buff[64]; // for display 
static uint16_t mnum=0; // store new 
static uint16_t mmax=0; // store max 
static uint16_t mmin=0; // store min
static NumberWindow *window;
static void (*set_function)( uint8_t ); // save value by calling this function 

//////////////////// APP 

// window init
void window_set_number_init( void ) {
		window = number_window_create("Empty",
			(NumberWindowCallbacks){.selected = selected_callback},
			NULL);
};


// deinit window
void window_set_number_deinit( void ) {
	number_window_destroy( window );
};


// show window 
// parameters: ptr fo save function, start value as current_num (if set) 
// minimum num , maximum allowed num
void window_set_number_show( void (*set_fn_ptr)(uint8_t), uint16_t current_num , uint16_t min, uint16_t max ) {
    LOG( "window set number show with current_num=%d, min=%d, max=%d", current_num, min, max );
    mnum = current_num;
    mmax = max;
    mmin = min;
    set_function = set_fn_ptr;
    // prepare for select hour
	number_window_set_min( window, mmin );
	number_window_set_max( window, mmax ); 
	number_window_set_value( window, mnum );
    strcpy( char_buff, "Set number" );
	number_window_set_label( window, char_buff );
	window_stack_push( number_window_get_window( window ), true );
};


// select function
static void selected_callback( NumberWindow *window, void *context ){
    LOG( "select callback ");
    // get new value
    mnum = number_window_get_value( window ) ;
    // log to me :)
    LOG( "num set to %d", mnum );
    // update value
    if ( set_function ) { 
     (*set_function)( mnum ); /// Call function by pointer, defined on show window
    };
    // close window
	window_stack_pop(true);
};
