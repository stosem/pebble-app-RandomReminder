/// timer functions

#include <pebble.h> // for base types (uint8_t , etc ) can be replaced for porting! 


// store timer settings here
typedef struct MTimer {
    uint16_t min_time;
    uint16_t max_time;
    uint16_t remaining_time;
    time_t last_timestamp;
    bool     is_repeat;
    bool     is_running;
    void (*ptr_callback_finish)(void);
    void (*ptr_callback_start)(uint16_t);
} MTimer;


// public function

MTimer *mytimer_create ( void );
void mytimer_destroy ( MTimer* );
void mytimer_set_callback( MTimer*, void (*ptr_fn_start)(uint16_t), void (*ptr_fn_finish)(void) );
uint16_t mytimer_start_random ( MTimer* );
void mytimer_pause ( MTimer *my_timer, time_t timestamp ) ;
void mytimer_resume ( MTimer *my_timer, time_t timestamp ) ;
void mytimer_tick ( MTimer* );
void mytimer_stop ( MTimer* );
bool mytimer_is_running ( MTimer* );
void mytimer_set_min ( MTimer*, uint16_t value );
void mytimer_set_max ( MTimer*, uint16_t value );
uint16_t mytimer_get_min ( MTimer* );
uint16_t mytimer_get_max ( MTimer* );
bool mytimer_get_repeat ( MTimer* );
void mytimer_set_repeat ( MTimer*, bool value );

