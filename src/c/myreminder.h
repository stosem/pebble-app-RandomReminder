#pragma once

#include <pebble.h>

#define MAX_REMINDS 8
// in minutes
#define TIME_MAX 24*60
// max try regenerate rnd
#define MAX_RND_TRY 10

typedef struct MReminder {
    // array of wakeup id s
    WakeupId a_wakeups_id[MAX_REMINDS]; 
    time_t  a_wakeups_time[MAX_REMINDS];
    uint16_t t_start;
    uint16_t t_stop;
    // left today
    uint8_t n_wakeups_left; 
    uint8_t n_wakeups_set;
    uint8_t n_wakeups_scheduled;
    bool is_enabled;
} MReminder;

// public
void myreminder_start( void ) ;
void myreminder_disable( void ) ;
bool myreminder_is_enabled( void );
void myreminder_set_start( uint16_t value ) ;
void myreminder_set_stop( uint16_t value ) ;
void myreminder_set_number ( uint8_t number ) ;
void myreminder_deinit( void ) ;
void myreminder_init( void ) ;
void myreminder_validate_wakeups( void ) ;
uint8_t myreminder_get_number( void ) ;
uint8_t myreminder_get_scheduled( void ) ;
uint8_t myreminder_get_left( void ) ;
uint16_t myreminder_get_start( void ) ; 
uint16_t myreminder_get_stop( void ) ;
time_t myreminder_get_next_wakeup_time( void ) ;
// private
static void myreminder_cancel_wakeups( void ) ;
static void myreminder_set_rnd_wakeups( uint8_t number, uint8_t day_after ) ;
static void myreminder_load_persist( void ) ;
static void myreminder_save_persist( void ) ;

