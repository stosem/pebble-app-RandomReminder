////////////////////
//
// pebble random wakeup
//
//

#include "myreminder.h"
#include "common.h"

// local variable
static MReminder mr;
static uint8_t try=0;

// call when stop clicked
static void myreminder_cancel_wakeups( void ) {
    LOG("cancel all wakeups");
    wakeup_cancel_all();
    mr.n_wakeups_scheduled = 0;
    mr.n_wakeups_left = 0;
    for ( uint8_t i=0; i < mr.n_wakeups_set ; i++ ) {
        mr.a_wakeups_id[i] = 0;
        mr.a_wakeups_time[i] = 0;
    };
};


// request all wakeups
void myreminder_validate_wakeups( void ) {
    LOG("validate wakeups");
    if ( mr.n_wakeups_scheduled == 0 ) return;  /// doesn't set any wakeup
    uint8_t i=0;
    uint8_t valid=0;
    // count valid queryes
    for ( i=0; i < mr.n_wakeups_set ; i++ ) {
        if ( mr.a_wakeups_id[i] > 0 ) {
            if ( wakeup_query( mr.a_wakeups_id[i], NULL ) &&  ( mr.a_wakeups_time[i] > time(NULL) ) ) {
                valid++;
            } else {
                mr.a_wakeups_id[i] = 0;
            };
        };
    };
    mr.n_wakeups_left=valid;
    LOG("valid wakeups=%d", valid);
    // LAST wakeup then rechedule for tommorow
    if (( mr.n_wakeups_left == 0 ) && mr.is_enabled ) {
        LOG("This is last wakeup. Reschedule wakeups! ");
        myreminder_set_rnd_wakeups( mr.n_wakeups_set, 1 ); 
    };
};


// generate uniq time
static time_t myreminder_rnd( time_t start, time_t stop ) {
    srand( time_ms( NULL, NULL ) );
    uint8_t i=0;

    bool is_exist=false;
    time_t t=0;
    t = start + (rand() % ((stop-start)/60) )*60 ; 
    LOG("rnd=%d", (unsigned int) t );
    for ( i=0; i<mr.n_wakeups_set; i++ ) {
        // if already exist in array or scheduled
        if ( mr.a_wakeups_time[i] == t ) {
            is_exist=true;
        };
    };
    // regenerate
    if (is_exist) {
        LOG("exitst. regenerate, try=%d", try);
        try++;
        if ( try < MAX_RND_TRY ) {
            t=myreminder_rnd( start, stop );
        };
    };
        try=0;
        return t;
};


// set timers
static void myreminder_set_rnd_wakeups( uint8_t number, uint8_t day_after ) {
    LOG("mr generating %d events for %d day", number, day_after );
    uint8_t i=0;
    WakeupId twid=0;
    mr.n_wakeups_scheduled=0;

    for ( i=0; i< number; i++ ) {
        // detect period
        static time_t temp;
        temp = time(NULL);
        static struct tm *tm_time;
        tm_time = localtime(&temp);
        tm_time->tm_mday+=day_after;
        // detect start
        tm_time->tm_hour=time_h( mr.t_start );
        tm_time->tm_min=time_m( mr.t_start );
        tm_time->tm_sec=0;
        static time_t ttstart;
        ttstart = mktime( tm_time );
        // detect stop
        temp = time (NULL);
        tm_time = localtime(&temp);
        tm_time->tm_mday+=day_after;
        tm_time->tm_hour=time_h( mr.t_stop );
        tm_time->tm_min=time_m( mr.t_stop );
        tm_time->tm_sec=0;
        static time_t ttstop;
        ttstop = mktime( tm_time );
        // try set for today
        if ( ( time(NULL) > ttstart ) && ( time(NULL) < ttstop ) ) { ttstart = time(NULL) ; };
        // if can't set all for today, you need set it for tomorrow
        if ( (time(NULL)+number*2*60) > ttstop ) return;
        // get rnd
        time_t rnd_timestamp = myreminder_rnd( ttstart, ttstop );
        LOG("RND generated timestam=%ld (%ld-%ld)", rnd_timestamp, ttstart, ttstop);
        twid = wakeup_schedule( rnd_timestamp, 0, false );
        LOG("wakeup #%d id=%d is set to %ld", i, (int)twid, rnd_timestamp ); 
        // success set
        if ( twid > 0 ) {
            mr.a_wakeups_id[i] = twid;
            mr.a_wakeups_time[i] = rnd_timestamp;
            mr.n_wakeups_scheduled++;
            mr.is_enabled = true;
        };
    };
    // refresh
    myreminder_validate_wakeups();
};


// start 
void myreminder_start( void ) {
    uint8_t n=mr.n_wakeups_set;
    LOG("mr start %d reminds!", n);
    myreminder_cancel_wakeups(); 
    myreminder_set_rnd_wakeups( n , 0 );
    // if can't set all for today make it for tomorrow
    if ( mr.n_wakeups_scheduled != mr.n_wakeups_set ) {
        myreminder_cancel_wakeups(); // if clear - remove all for today 
        LOG("not scheduled, regenerate for tommorow"); 
        myreminder_set_rnd_wakeups( n , 1 );
    };
};


// return fist scheduled time
time_t myreminder_get_next_wakeup_time( void ) {
	bool found=false;
    time_t t = time(NULL);
    struct tm* t_tm = localtime(&t);
    t_tm->tm_mday++;
    time_t res = mktime(t_tm);
    for ( uint8_t i=0; i < mr.n_wakeups_set ; i++ ) {
        if ( ( mr.a_wakeups_time[i] > time( NULL )) &&  ( mr.a_wakeups_time[i] < res ) ) {
            res = mr.a_wakeups_time[i];
			found=true;
        };
    };
    return found?res:0;
};


// get number of set wakeups
uint8_t myreminder_get_number( void ) {
    return mr.n_wakeups_set;
};


// get number of realy scheduled wakeups
uint8_t myreminder_get_scheduled( void ) {
    return mr.n_wakeups_scheduled;
};


// get number of left wakeups for today (call it after validate! )
uint8_t myreminder_get_left( void ) {
    myreminder_validate_wakeups();
    return mr.n_wakeups_left;
};


// get start from settings
uint16_t myreminder_get_start( void ) { 
    return mr.t_start; 
};


// get stop from settings
uint16_t myreminder_get_stop( void ) { 
    return mr.t_stop; 
};


// get enabled 
bool myreminder_is_enabled( void ) {
    return mr.is_enabled;
};


// disable
void myreminder_disable( void ) {
    LOG("mr disable");
    mr.is_enabled = false;
    myreminder_cancel_wakeups();
};


// start 

// set from
void myreminder_set_start( uint16_t value ) {
    // min can't be > MAX
    if ( value >= TIME_MAX ) value = TIME_MAX-2;
    // min can't be > max
    if ( value >= mr.t_stop ) value -= 60; // -1 hour
    mr.t_start = value;
    LOG("mr set start %d", (int)value );
};


// set to 
void myreminder_set_stop( uint16_t value ) {
   // max can't be < min
    if ( value < mr.t_start) value=mr.t_start ; 
    // can't be = min
    if ( value == mr.t_start ) {
        value += 60; // add hour
    };
    // max can't be > MAX
    if ( value > TIME_MAX ) value=TIME_MAX-1;
    mr.t_stop = value;
    LOG("mr set stop %d", (int)value );
};


// set number 
void myreminder_set_number ( uint8_t number ) {
    if ( number > MAX_REMINDS ) number = MAX_REMINDS;
    mr.n_wakeups_set = number;
    LOG("mr set number %d", number );
};


// load settings
static void myreminder_load_persist( void ) {
     LOG("mr load");
     persist_read_data( MREMINDER_KEY, &mr, sizeof( MReminder ) ); 
};


// save settings
static void myreminder_save_persist( void ) {
     LOG("mr save");
     persist_write_data( MREMINDER_KEY, &mr, sizeof( MReminder ));
};


// exit
void myreminder_deinit( void ) {
    LOG("mr deinit");
    myreminder_save_persist();
};


// init object
void myreminder_init( void ) {
    LOG("mr init");
    mr.t_start=10*60; // 10:00
    mr.t_stop=22*60;  // 22:00
    mr.n_wakeups_left=0;
    mr.n_wakeups_set=0;
    mr.n_wakeups_scheduled=0;
    for ( uint8_t i=0; i < MAX_REMINDS ; i++ ) {
        mr.a_wakeups_time[i] = 0;
        mr.a_wakeups_id[i] = 0;
    }
    mr.is_enabled=false;
    myreminder_load_persist();
    myreminder_validate_wakeups();
};

