#include "mytimer.h"
#include "common.h"


// create clear timer instance
MTimer *mytimer_create( void ) {
    MTimer* my_timer = malloc(sizeof(MTimer));
    my_timer->min_time = 0;
    my_timer->max_time = TIME_DEFAULT;
    my_timer->is_repeat = 0;
    my_timer->is_running = 0;
    my_timer->remaining_time = 0;
    my_timer->ptr_callback_finish = NULL;
    my_timer->ptr_callback_start = NULL;
    return my_timer;
};

// destroy
void mytimer_destroy ( MTimer *my_timer ) {
    if ( my_timer == NULL ) return;
    if ( my_timer != NULL ) {
        free( my_timer );
        my_timer = NULL;
    };
};


// assign callback functions
// start - function is called by timer when it finish. Add some greetings on it
// finish - function is called by timer when it wants to start. Add start system hw timer here
//  and in hw timer callback's function please call mytimer_tick() every second. 
void mytimer_set_callback( MTimer *my_timer, void (*ptr_fn_start)(uint16_t), void (*ptr_fn_finish)(void) ) {
    ASSERT( my_timer );
    my_timer->ptr_callback_finish = ptr_fn_finish;
    my_timer->ptr_callback_start = ptr_fn_start;
};


// start random
uint16_t mytimer_start_random( MTimer *my_timer ) {
    ASSERT ( my_timer );
    uint16_t res=0;
    srand( time( NULL ));
    // start from 1
    res=1 + my_timer->min_time + rand() % ( my_timer->max_time - my_timer->min_time );
    LOG( "RND=%d (%d-%d)", res, my_timer->min_time, my_timer->max_time );
    my_timer->is_running = true;
    my_timer->remaining_time = res;
    // start callback for hw timer
    if ( my_timer->ptr_callback_start ) {
        (*my_timer->ptr_callback_start)(res);
    };
    return res;
};


// pause on current timestamp
void mytimer_pause ( MTimer *my_timer, time_t timestamp ) {
    ASSERT( my_timer );
    if (( ! my_timer->is_running ) || ( ! timestamp )) return;
    my_timer->last_timestamp = timestamp; 
};


//resume on current timestamp
void mytimer_resume ( MTimer *my_timer, time_t timestamp ) {
    ASSERT( my_timer );
    if (( ! my_timer->is_running ) || ( ! timestamp )) return;
    if ( timestamp < my_timer->last_timestamp ) return ; // that can't be
    uint16_t sleep_delta = timestamp-my_timer->last_timestamp;
    if ( sleep_delta > my_timer->remaining_time ) {
        my_timer->remaining_time = 1;
        // finish
    } else {
        my_timer->remaining_time -= sleep_delta;
    };
    // start hw timer
    if ( my_timer->ptr_callback_start ) {
        (*my_timer->ptr_callback_start)(my_timer->remaining_time);
    };
};


// tick timer. must be call from hw timer
void mytimer_tick( MTimer *my_timer ) {
    ASSERT ( my_timer );
    if ( ! my_timer->is_running ) return;
    if (my_timer->remaining_time > 0 ) {
            my_timer->remaining_time--;
            return;
    } else {
            my_timer->is_running = false;
            // finish!
            if ( my_timer->ptr_callback_finish ) {
                (*my_timer->ptr_callback_finish)();
            };
            if ( my_timer->is_repeat ) {
                mytimer_start_random( my_timer );
            };
    };
};


// stop timer
void mytimer_stop( MTimer *my_timer ) {
    ASSERT ( my_timer );
    my_timer->is_running = false;
};


// get is running
bool mytimer_is_running( MTimer *my_timer ) { 
    ASSERT ( my_timer );
    return my_timer->is_running;
};


// set min value with corrects
void mytimer_set_min( MTimer *my_timer, uint16_t value ) { 
    ASSERT ( my_timer );
    // min can't be > MAX
    if ( value > TIME_MAX ) value = TIME_MAX-1;
    // min can't be > max
    my_timer->min_time = ( value > my_timer->max_time )?my_timer->max_time:value;
}; 


// set max value with corrects
void mytimer_set_max( MTimer *my_timer, uint16_t value ) { 
    ASSERT ( my_timer );
    // max can't be = 0
    if ( value < TIME_MIN ) value=TIME_MIN;
    // max can't be > MAX
    if ( value > TIME_MAX ) value=TIME_MAX;
    // max can't be < min
    my_timer->max_time = ( value < my_timer->min_time )?my_timer->min_time:value ; 
    // can't be = min
    if ( my_timer->max_time == my_timer->min_time ) {
        my_timer->max_time++;
    };
}; 


// get min
uint16_t mytimer_get_min( MTimer *my_timer ) { 
    ASSERT ( my_timer );
    return my_timer->min_time ; 
}; 


// get max
uint16_t mytimer_get_max( MTimer *my_timer ) { 
    ASSERT ( my_timer );
    return my_timer->max_time ; 
}; 


// get repeat flag
bool mytimer_get_repeat( MTimer *my_timer ) { 
    ASSERT ( my_timer );
    return my_timer->is_repeat ; 
};


// set repeat flag
void mytimer_set_repeat( MTimer *my_timer, bool value ) { 
    ASSERT ( my_timer );
    my_timer->is_repeat = value ; 
};
