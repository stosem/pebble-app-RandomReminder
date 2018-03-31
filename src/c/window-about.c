#include <pebble.h>
#include "window-about.h"

static void click_config_provider(Window* window);
static void click_handler(ClickRecognizerRef recognizer, void* context);
static void window_appear(Window* window);
static void window_disappear(Window* window);

static Window *window;
static Layer *s_main_layer;
static TextLayer *s_label_layer;
static BitmapLayer *bmp_layer;
static GBitmap *bmp=NULL;


// init window
void window_about_init(void) {
  window = window_create();
  window_set_background_color( window, GColorWhite );
  window_set_click_config_provider( window, (ClickConfigProvider)click_config_provider );
  Layer *window_layer = window_get_root_layer( window );
  GRect bounds = layer_get_bounds( window_layer );
  // create main canvas layer
  s_main_layer = layer_create( bounds );
  layer_add_child( window_layer, s_main_layer );
  bmp_layer = bitmap_layer_create( GRect(PBL_IF_ROUND_ELSE(78 ,1), PBL_IF_ROUND_ELSE(2,2), 25, 25) );
  if ( bmp ) {
  gbitmap_destroy( bmp );
  bmp = NULL;
  };
  bmp=gbitmap_create_with_resource( RESOURCE_ID_menu_icon );
  bitmap_layer_set_compositing_mode( bmp_layer, GCompOpSet );
  bitmap_layer_set_bitmap( bmp_layer, bmp );
  layer_add_child( s_main_layer , bitmap_layer_get_layer( bmp_layer ));
  s_label_layer = text_layer_create( GRect( 8 , bounds.origin.y+PBL_IF_ROUND_ELSE(24,4), bounds.size.w-12, bounds.size.h));
  text_layer_set_font( s_label_layer, fonts_get_system_font( FONT_KEY_GOTHIC_18 ) );
  text_layer_set_text_color(s_label_layer, GColorBlack);
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
  text_layer_set_text( s_label_layer, " RandomReminder\n\
Free and OpenSource.\n\
github.com/stosem/pebble-app-RandomReminder\n\
You can support it by donate.\n\
paypal.me/stosem\n\
Thank you!");
  layer_add_child(s_main_layer, text_layer_get_layer( s_label_layer ));
#if defined(PBL_ROUND)
  text_layer_enable_screen_text_flow_and_paging(s_label_layer, 2 );
#endif 
};


void window_about_show(void) {
  if (window_stack_contains_window(window)) {
    return;
  };
  window_stack_push(window, false);
};


// deinit
void window_about_deinit(void) {
  if ( bmp ) {
  gbitmap_destroy( bmp );
  bmp = NULL;
  };
  bitmap_layer_destroy(bmp_layer);
  text_layer_destroy( s_label_layer );
  layer_destroy(s_main_layer);
  window_destroy(window);
};


//----------------------------------------------------------------------------//

static void click_config_provider(Window* window) {
  window_single_click_subscribe(BUTTON_ID_UP, click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler);
};


static void click_handler(ClickRecognizerRef recognizer, void* context) {
  window_stack_pop(true);
};
