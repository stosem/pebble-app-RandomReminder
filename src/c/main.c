////////////////////
//
// pebble application
// Random Timer by stosem <alex.p.burlutsky@gmail.com>
// Licensed under GPL v3
//
//
#include <pebble.h>

#include "main.h"
#include "common.h"
#include "myreminder.h"
#include "window-set-time.h"
#include "window-set-number.h"
#include "window-vibrate.h"
#include "window-about.h"

static Window *s_main_window=NULL;
static Layer *s_main_layer=NULL;
static TextLayer *s_label_layer=NULL;
static TextLayer *s_status_layer=NULL;
static ActionBarLayer *s_action_bar=NULL;

static GBitmap *s_menu_bitmap=NULL;
static GBitmap *s_play_bitmap=NULL;
static GBitmap *s_stop_bitmap=NULL;

static ActionMenu *s_action_menu=NULL;
static ActionMenuLevel *s_root_level=NULL;

static Settings settings;

// heap vars for menu
static char char_buff[64];
static char start_buff[64];
static char stop_buff[64];
static char num_buff[64];
static char vib_buff[64];
static char label_buff[128];
static char status_buff[128];
static bool is_menu_changed = false;
static bool is_main_visible = false;


//////////////////// Persistent storage//////////////////// 

// load settings
static void settings_load_persist( void ) {
     LOG("settings load persist");
     persist_read_data( SETTINGS_KEY, &settings, sizeof( Settings ) );
};


// save settings and status
static void settings_save_persist( void ) {
    LOG("settings save persist");
    persist_write_data( SETTINGS_KEY, &settings, sizeof( Settings ) );
};


// clear config
static void settings_clear( void ) {
    settings.vibration_pattern = VIB_ID_LONG;
};

//////////////////// Action Menu //////////////////// 

// action on menu
static void action_performed_callback( ActionMenu *action_menu, const ActionMenuItem *action, void *context ) {
  // An action was selected, determine which one
  int m_id = (int)action_menu_item_get_action_data(action);
  LOG( "Menu selected '%d'", m_id);
  switch ( m_id ) {
       case MENU_ID_START : 
            window_set_time_show( &myreminder_set_start, 
                                myreminder_get_start(), 0, myreminder_get_stop()-1 );
            is_menu_changed = true; // TODO: maybe detect if time realy changed by user, but... 
       break;
       case MENU_ID_STOP:
            window_set_time_show( &myreminder_set_stop,
                                myreminder_get_stop(), myreminder_get_start()+1, TIME_MAX-1 ); 
            is_menu_changed = true;
       break;
       case MENU_ID_NUMBER: 
            window_set_number_show( &myreminder_set_number,
                                    myreminder_get_number(), 1, MAX_REMINDS );
           is_menu_changed = true;
       break;
       case MENU_ID_VIB: 
            // vib pattern
            // maybe make submenu with many patterns
            // but now only switch
           if ( settings.vibration_pattern == VIB_ID_LONG ) {
               settings.vibration_pattern = VIB_ID_SOLID;
           } else {
              settings.vibration_pattern = VIB_ID_LONG;
           };
           is_menu_changed = true;
       break;
       case MENU_ID_ABOUT:
            window_about_show();
       break;;
  };
};


// init action menu
static void action_menu_init( void ) {
  // Create the root level
  s_root_level = action_menu_level_create(5);
  // create item depend on value 
  sntime2str_hm ( char_buff, sizeof( char_buff ), myreminder_get_start(), 0 );
  strcpy ( start_buff, "Start time\n" );
  strcat ( start_buff, char_buff );
  sntime2str_hm ( char_buff, sizeof( char_buff ), myreminder_get_stop(), 0 );
  strcpy ( stop_buff, "End time\n" );
  strcat ( stop_buff, char_buff );
  snprintf ( num_buff, sizeof( num_buff ), "Num of reminds: %d", myreminder_get_number() );
    switch ( settings.vibration_pattern ) {
      case VIB_ID_LONG: strcpy( vib_buff, "Vibro: Once" );
      break;;
      case VIB_ID_SOLID: strcpy( vib_buff, "Vibro: Solid" );
      break;;
    };
  // Set up the actions for this level, using action context to pass types
  action_menu_level_add_action(s_root_level, num_buff , action_performed_callback, (void *) MENU_ID_NUMBER );
  action_menu_level_add_action(s_root_level, start_buff, action_performed_callback, (void *) MENU_ID_START );
  action_menu_level_add_action(s_root_level, stop_buff, action_performed_callback, (void *) MENU_ID_STOP );
  action_menu_level_add_action(s_root_level, vib_buff, action_performed_callback, (void *) MENU_ID_VIB );
  action_menu_level_add_action(s_root_level, "About", action_performed_callback, (void *) MENU_ID_ABOUT );
  is_menu_changed = false;
};


//////////////////// Button click //////////////////// 
// play click 
static void play_click_handler( ClickRecognizerRef recognizer, void *context ) {
  if ( myreminder_is_enabled() ) {
      // stop 
      myreminder_disable(); 
  } else {
      myreminder_start();
  };
  LOG( "click on start/stop. status: %s", myreminder_is_enabled()?"enabled":"disabled"  );
  window_update_status();
}; 


// select menu 
static void menu_click_handler( ClickRecognizerRef recognizer, void *context ) {
  LOG( "select handler" );
  // recreate menu 
  if ( s_root_level && is_menu_changed ) { 
    action_menu_hierarchy_destroy( s_root_level, NULL, NULL ); 
    action_menu_init();
  };

  // Configure the ActionMenu Window about to be shown
  ActionMenuConfig config = ( ActionMenuConfig ) {
    .root_level = s_root_level,
    .colors = {
      .background = PBL_IF_COLOR_ELSE( GColorChromeYellow, GColorWhite ),
      .foreground = GColorBlack,
    },
    .align = ActionMenuAlignCenter
  };
  // Show the ActionMenu
  s_action_menu = action_menu_open( &config );
};


// click config provider on action bar
static void click_config_provider( void *context ) {
  LOG( "click config" );
  window_single_click_subscribe( BUTTON_ID_DOWN, menu_click_handler );
  window_single_click_subscribe( BUTTON_ID_SELECT, play_click_handler );
};

//////////////////// Callback 


// wakeup
static void wakeup_handler(WakeupId id, int32_t reason) {
    LOG("Wakeup handler!");
    myreminder_validate_wakeups();
    window_update_status();
    switch ( settings.vibration_pattern ) {
        case VIB_ID_SOLID : window_vibrate_show( 0 ) ;
        break;;
        default : window_vibrate_show( 1 ) ;
        break;;
    };
};

// update text on label
static void window_update_status( void ) {
 if ( is_main_visible == false ) return; 
 LOG("Update text on label");
 // update label enabled/disabled
 snprintf ( label_buff, sizeof( label_buff ), "Random Reminder\n%s", 
                                                myreminder_is_enabled()?"ENABLED":"disabled" );
 text_layer_set_text(s_label_layer, label_buff );
 // udate  status left/all
 snprintf ( status_buff, sizeof( status_buff), "Reminders left:%d/%d", 
                                                myreminder_get_left(), myreminder_get_scheduled() );
 text_layer_set_text(s_status_layer, status_buff );
 action_bar_layer_set_icon( s_action_bar, BUTTON_ID_SELECT, myreminder_is_enabled()?s_stop_bitmap:s_play_bitmap );
};


// on show 
static void window_appear(Window* window) {
  is_main_visible = true;
};


// on hide
void window_disappear(Window* window) {
  is_main_visible = false;
};


// draw main window 
static void layer_update_proc_main( Layer *layer, GContext *ctx ) {
 // LOG("Redraw: main window");
};


//////////////////// Main Window ////////////////////   

// window load
static void window_load( Window *window ) {
  LOG(" Create layers " );
  Layer *window_layer = window_get_root_layer( window );
  GRect bounds = layer_get_bounds( window_layer );

  // setup layers

  // create main canvas layer
  s_main_layer = layer_create(bounds);
  layer_set_update_proc(s_main_layer, layer_update_proc_main);
  layer_add_child(window_layer, s_main_layer);

  // label layer
  s_label_layer = text_layer_create(GRect(bounds.origin.x, bounds.origin.y, bounds.size.w - ACTION_BAR_WIDTH, 80));
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_color(s_label_layer, GColorBlack);
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
  layer_add_child(s_main_layer, text_layer_get_layer(s_label_layer));
#if defined(PBL_ROUND)
  text_layer_enable_screen_text_flow_and_paging(s_label_layer, 2 );
#endif

  // status layer
  s_status_layer = text_layer_create(GRect(bounds.origin.x, bounds.origin.y+100, bounds.size.w - ACTION_BAR_WIDTH, 60));
  text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_color(s_status_layer, GColorBlack);
  text_layer_set_background_color(s_status_layer, GColorClear);
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
  layer_add_child(s_main_layer, text_layer_get_layer(s_status_layer));
#if defined(PBL_ROUND)
  text_layer_enable_screen_text_flow_and_paging(s_status_layer, 2 );
#endif


  // create action bar 
  LOG(" Create action bar" );
  s_menu_bitmap = gbitmap_create_with_resource( RESOURCE_ID_ICON_MENU );
  s_play_bitmap = gbitmap_create_with_resource( RESOURCE_ID_ICON_PLAY );
  s_stop_bitmap = gbitmap_create_with_resource( RESOURCE_ID_ICON_STOP );
  s_action_bar = action_bar_layer_create();
  action_bar_layer_set_click_config_provider( s_action_bar, click_config_provider );
  // assign to down key
  action_bar_layer_set_icon( s_action_bar, BUTTON_ID_DOWN, s_menu_bitmap );
  action_bar_layer_add_to_window( s_action_bar, window );

  LOG(" Interface initilazed" );

  LOG(" Update statuses" );
  action_menu_init();
  window_update_status();
};


// window unload
static void window_unload( Window *window ) {
  // unsubscribe
  tick_timer_service_unsubscribe();
  // text layer
  text_layer_destroy( s_label_layer );
  text_layer_destroy( s_status_layer );
  // layer
  layer_destroy(s_main_layer);
  // action bar
  action_bar_layer_destroy( s_action_bar );
  // menu
  if ( s_root_level ) {
      action_menu_hierarchy_destroy( s_root_level, NULL, NULL );
      s_root_level = NULL;
  };
  // bitmaps
  if ( s_menu_bitmap ) gbitmap_destroy( s_menu_bitmap );
  if ( s_play_bitmap ) gbitmap_destroy( s_play_bitmap );
  if ( s_stop_bitmap ) gbitmap_destroy( s_stop_bitmap );
};

//////////////////// APP ////////////////////   

// init 
static void init( void ) {
  LOG("Init, lunch reason is %d", (int)launch_reason() );
  settings_clear(); 

  LOG(" Create window" );
  s_main_window = window_create();
  window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite));
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
    .appear = window_appear,
    .disappear = window_disappear
  });

  wakeup_service_subscribe(wakeup_handler);

  // window init
  LOG(" Init window " );
  window_vibrate_init();
  window_set_time_init();
  window_set_number_init();
  window_about_init();

  LOG(" Create settings" );
  // load persist settings
  settings_load_persist ();
  myreminder_init();

  if (launch_reason() == APP_LAUNCH_WAKEUP) {
      wakeup_handler( 0, 0 );
  } else {
    is_main_visible = true;
    window_stack_push( s_main_window, true );
  }

};


// deinit
static void deinit( void ) {
  LOG("deinit run");
  // destroy windows
  window_destroy(s_main_window);
  window_set_time_deinit();
  window_set_number_deinit();
  window_vibrate_deinit();
  window_about_deinit();
  // save settings
  myreminder_deinit();
  settings_save_persist() ;
};


// main enter point
int main( void ) {
  init();
  app_event_loop();
  deinit();
};


