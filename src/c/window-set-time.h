#pragma once
#include "pebble.h"


static void selected_callback( NumberWindow *window, void *context );
void window_set_time_init( void );
void window_set_time_deinit( void );
void window_set_time_show( void (*set_fn_ptr)(uint16_t), uint16_t current_time , uint16_t min, uint16_t max ) ;
