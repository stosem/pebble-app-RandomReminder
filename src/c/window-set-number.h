#pragma once
#include "pebble.h"

static void selected_callback( NumberWindow *window, void *context );
void window_set_number_init( void );
void window_set_number_deinit( void );
void window_set_number_show( void (*set_fn_ptr)(uint8_t), uint16_t current_num , uint16_t min, uint16_t max ) ;
