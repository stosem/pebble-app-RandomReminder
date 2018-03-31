#pragma once

#include "common.h"

// Menu items
enum MenuID {
    MENU_ID_NUMBER=1,
    MENU_ID_START=2,
    MENU_ID_STOP=3,
    MENU_ID_VIB=4,
    MENU_ID_ABOUT=5
};

enum VibID {
    VIB_ID_LONG=0,
    VIB_ID_SOLID=1
};

// store some app settings
typedef struct Settings {
    uint8_t vibration_pattern;  // VibID
} Settings;

static void settings_load_persist( void ) ;
static void settings_save_persist( void ) ;
static void settings_clear( void ) ;
static void action_performed_callback( ActionMenu *action_menu, const ActionMenuItem *action, void *context ) ;
static void action_menu_init( void ) ;
static void menu_click_handler( ClickRecognizerRef recognizer, void *context ) ;
static void play_click_handler( ClickRecognizerRef recognizer, void *context ) ;
static void click_config_provider( void *context ) ;
static void window_update_status( void ) ;
static void layer_update_proc_main( Layer *layer, GContext *ctx ) ;
static void window_load( Window *window ) ;
static void window_unload( Window *window ) ;
static void init( void ) ;
static void deinit( void ) ;
