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
#include "mytimer.h"
#include "window-set-time.h"
#include "window-vibrate.h"
#include "window-about.h"

static Window *s_main_window=NULL;
static Layer *s_main_layer=NULL;
static TextLayer *s_label_layer=NULL;
static ActionBarLayer *s_action_bar=NULL;

static GBitmap *s_menu_bitmap=NULL;
static GBitmap *s_play_bitmap=NULL;
static GBitmap *s_stop_bitmap=NULL;
static GBitmap *s_rnd_bitmap=NULL;

static BitmapLayer *s_rnd_bitmap_layer=NULL;

static ActionMenu *s_action_menu=NULL;
static ActionMenuLevel *s_root_level=NULL;

// software my timer
static MTimer *mt=NULL; 

static Settings settings;

// heap vars for menu
static char char_buff[64];
static char min_buff[64];
static char max_buff[64];
static char repeat_buff[64];
static char vib_buff[64];
static char label_buff[128];
bool is_menu_changed = false;


//////////////////// Persistent storage//////////////////// 

// load timer settings
static void mt_load_persist( void ) {
     LOG("mt load persist");
     persist_read_data( MTIMER_KEY, mt, sizeof( MTimer ) ); 
     persist_read_data( SETTINGS_KEY, &settings, sizeof( Settings ) );
};


// save timer settings and status
static void mt_save_persist( void ) {
    LOG("mt save persist");
     persist_write_data( MTIMER_KEY, mt, sizeof( MTimer ));
     persist_write_data( SETTINGS_KEY, &settings, sizeof( Settings ) );
};


// clear config
static void mt_config_clear( void ) {
    settings.vibration_pattern = VIB_ID_LONG;
};

//////////////////// Action Menu //////////////////// 

// action on menu
static void action_performed_callback( ActionMenu *action_menu, const ActionMenuItem *action, void *context ) {
  // An action was selected, determine which one
  int m_id = (int)action_menu_item_get_action_data(action);
  LOG( "Menu selected '%d'", m_id);
  switch ( m_id ) {
       case MENU_ID_MIN : 
            window_set_time_show( &window_set_time_callback_set_min, 
                                mytimer_get_min( mt ), 0, mytimer_get_max( mt )-1 );
            is_menu_changed = true; // TODO: maybe detect if time realy changed by user, but... 
       break;
       case MENU_ID_MAX:
            window_set_time_show( &window_set_time_callback_set_max, 
                                mytimer_get_max( mt ), mytimer_get_min( mt )+1, TIME_MAX ); 
            is_menu_changed = true;
       break;
       case MENU_ID_REPEAT: 
            // want to change name in menu
           mytimer_set_repeat( mt, !mytimer_get_repeat( mt ) );
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
  sntime2str ( char_buff, sizeof( char_buff ), mytimer_get_min( mt ), 0 );
  strcpy ( min_buff, "Min time\n" );
  strcat ( min_buff, char_buff );
  sntime2str ( char_buff, sizeof( char_buff ), mytimer_get_max( mt ), 0 );
  strcpy ( max_buff, "Max time\n" );
  strcat ( max_buff, char_buff );
  snprintf ( repeat_buff, sizeof( repeat_buff ), "Repeat: %s", mytimer_get_repeat( mt )?"ON":"OFF" );
    switch ( settings.vibration_pattern ) {
      case VIB_ID_LONG: strcpy( vib_buff, "Vibro: Once" );
      break;;
      case VIB_ID_SOLID: strcpy( vib_buff, "Vibro: Solid" );
      break;;
    };
  // Set up the actions for this level, using action context to pass types
  action_menu_level_add_action(s_root_level, max_buff, action_performed_callback, (void *) MENU_ID_MAX );
  action_menu_level_add_action(s_root_level, min_buff, action_performed_callback, (void *) MENU_ID_MIN );
  action_menu_level_add_action(s_root_level, repeat_buff , action_performed_callback, (void *) MENU_ID_REPEAT );
  action_menu_level_add_action(s_root_level, vib_buff, action_performed_callback, (void *) MENU_ID_VIB );
  action_menu_level_add_action(s_root_level, "About", action_performed_callback, (void *) MENU_ID_ABOUT );
  is_menu_changed = false;
};


// make window animation
static void window_rnd_animation( void ) {
    // rnd bitmap
        if ( s_rnd_bitmap ) {
            gbitmap_destroy( s_rnd_bitmap );
            s_rnd_bitmap=NULL;
        };
        srand( time( NULL ) );
        uint8_t trnd = 1+ ( rand() % 6 );
        LOG(" Some animation icon=%d", trnd ); 
        switch ( trnd ) {
            case 1: s_rnd_bitmap = gbitmap_create_with_resource( RESOURCE_ID_RND_1 );
            break;;
            case 2: s_rnd_bitmap = gbitmap_create_with_resource( RESOURCE_ID_RND_2 );
            break;;
            case 3: s_rnd_bitmap = gbitmap_create_with_resource( RESOURCE_ID_RND_3 );
            break;;
            case 4: s_rnd_bitmap = gbitmap_create_with_resource( RESOURCE_ID_RND_4 );
            break;;
            case 5: s_rnd_bitmap = gbitmap_create_with_resource( RESOURCE_ID_RND_5 );
            break;;
            default: s_rnd_bitmap = gbitmap_create_with_resource( RESOURCE_ID_RND_6 );
            break;;
        };
        bitmap_layer_set_bitmap( s_rnd_bitmap_layer, s_rnd_bitmap ); 
};


//////////////////// Callback//////////////////// 


// callback from hw timer
void app_timer_callback_proc( struct tm *tick_time, TimeUnits units_changed ) {
    LOG(" app timer tick. remaining=%d", mt->remaining_time );
    // tick soft timer
    mytimer_tick( mt );
    if ( mytimer_is_running( mt ) ) {
        // xxx: recreate app_timer here if you use app_timer instead tick_handler
        // make some animation
        window_rnd_animation() ;
    };
};


// timer finish callback by my timer
void mt_finish_callback_proc ( void ) {
    LOG(" Timer finish! " );
    tick_timer_service_unsubscribe();
    window_update_status();
    if ( ! quiet_time_is_active() ) {
        switch ( settings.vibration_pattern ) {
            case VIB_ID_LONG : vibes_long_pulse();
            break ;;
            case VIB_ID_SOLID: window_vibrate_show();
            break ;;
        }
    };
};


// timer start callback from my timer
void mt_start_callback_proc ( uint16_t num ) {
    LOG(" Timer start for %d sec", num );
    tick_timer_service_subscribe( SECOND_UNIT, app_timer_callback_proc );
    window_update_status();
};


// callback from set_time window after set min
void window_set_time_callback_set_min ( uint16_t value ) {
    // save value from called window to timer
    mytimer_set_min( mt, value );
};


// callback from set_time window after set max
void window_set_time_callback_set_max ( uint16_t value ) {
    // save value from called window to timer
    mytimer_set_max( mt, value );
};


//////////////////// Button click //////////////////// 
// play click 
static void play_click_handler( ClickRecognizerRef recognizer, void *context ) {
  if ( mytimer_is_running( mt ) ) {
      // stop soft timer
      mytimer_stop( mt ); 
      // stop hard timer
      tick_timer_service_unsubscribe();
  } else {
    mytimer_start_random( mt );
  };
  LOG( "click on start/stop timer. status: %s", mytimer_is_running( mt )?"running":"stopped"  );
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


// update text on label
static void window_update_status( void ) {
 LOG("Update text on label");
 // update status  Running / stoppped
 snprintf ( label_buff, sizeof( label_buff ), "Random timer %s", mytimer_is_running( mt )?"RUNNING":"stopped" );
 text_layer_set_text(s_label_layer, label_buff );
 action_bar_layer_set_icon( s_action_bar, BUTTON_ID_SELECT, mytimer_is_running( mt )?s_stop_bitmap:s_play_bitmap );
};


// draw main window 
static void layer_update_proc_main( Layer *layer, GContext *ctx ) {
  LOG("Redraw: main window");
  if ( ! mytimer_is_running( mt ) ) return;
  // make some animation
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  graphics_context_set_stroke_color( ctx, GColorBlack );
  graphics_context_set_stroke_width( ctx, 3 );
  GPoint start_temp_line = GPoint(PBL_IF_ROUND_ELSE(60, 26 ), PBL_IF_ROUND_ELSE(80, 60));
  GPoint end_temp_line = GPoint(PBL_IF_ROUND_ELSE(60, 26 )+t->tm_sec, PBL_IF_ROUND_ELSE(80, 60));
  graphics_draw_line(ctx, start_temp_line, end_temp_line);
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

  s_label_layer = text_layer_create(GRect(bounds.origin.x, bounds.origin.y, bounds.size.w - ACTION_BAR_WIDTH, bounds.size.h));
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_color(s_label_layer, GColorBlack);
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
  layer_add_child(s_main_layer, text_layer_get_layer(s_label_layer));
#if defined(PBL_ROUND)
  text_layer_enable_screen_text_flow_and_paging(s_label_layer, 2 );
#endif
  // create bitmap layer
  s_rnd_bitmap = gbitmap_create_with_resource( RESOURCE_ID_RND_1 ); 
  s_rnd_bitmap_layer = bitmap_layer_create( GRect(PBL_IF_ROUND_ELSE(64, 32), PBL_IF_ROUND_ELSE(100, 82), 50, 50) );
  bitmap_layer_set_compositing_mode( s_rnd_bitmap_layer, GCompOpSet );
  bitmap_layer_set_bitmap( s_rnd_bitmap_layer, s_rnd_bitmap);
  layer_add_child( s_main_layer, bitmap_layer_get_layer( s_rnd_bitmap_layer ));

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

  // load timer settings
  LOG(" Create timer" );
  mt=mytimer_create();
  // load persist settings
  mt_load_persist ();
  // refresh callbacks
  mytimer_set_callback( mt, &mt_start_callback_proc, &mt_finish_callback_proc );
  // resume timer if it runs
  if ( mytimer_is_running( mt ) ) {
      mytimer_resume( mt, time(NULL) ); 
  };

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
  // bitmap_layer
  bitmap_layer_destroy( s_rnd_bitmap_layer );
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
  if ( s_rnd_bitmap ) gbitmap_destroy( s_rnd_bitmap );
  // timers
  if ( mt ) mytimer_destroy( mt );
};

//////////////////// APP ////////////////////   

// init 
static void init( void ) {
  LOG("Init, lunch reason is %d", (int)launch_reason() );
  mt_config_clear(); 

  LOG(" Create window" );
  s_main_window = window_create();
  window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite));
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  wakeup_cancel_all();
 
  // window init
  LOG(" Init window " );
  window_set_time_init();
  window_vibrate_init();
  window_about_init();
};


// deinit
static void deinit( void ) {
  LOG("deinit run");
  window_destroy(s_main_window);
  window_set_time_deinit();
  window_vibrate_deinit();
  window_about_deinit();
  // subscribe wakeup if timer exist
  if ( mytimer_is_running( mt ) ) {
    mytimer_pause( mt, time(NULL) );
    // schedule system wakeup
    time_t wakeup_time = time(NULL) + mt->remaining_time ;
    int32_t wid = wakeup_schedule(wakeup_time, 0, false);
    LOG("ID=%d Now %d. Subscribe to wakeup in %d (after %d sec)", (int) wid, (int)time(NULL), (int)wakeup_time, 
            (int)(wakeup_time-time(NULL)) );
  };
  mt_save_persist() ;
};


// main enter point
int main( void ) {
  init();
  app_event_loop();
  deinit();
};


