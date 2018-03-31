/*

Multi Timer v3.4
http://matthewtole.com/pebble/multi-timer/

----------------------

The MIT License (MIT)
Copyright © 2013 - 2015 Matthew Tole
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

--------------------

src/windows/win-vibrate.c

*/

#include <pebble.h>

static void click_config_provider(Window* window);
static void click_handler(ClickRecognizerRef recognizer, void* context);
static void window_appear(Window* window);
static void window_disappear(Window* window);
static void do_vibrate(void);
static void vibe_timer_callback(void* data);

static Window* window;
static BitmapLayer* bmp_layer;
static GBitmap *bmp=NULL;
static AppTimer* vibe_timer = NULL;
static bool is_visible = false;

static uint8_t num_vibrate = 0;
#define MAX_VIBRATE 60

void window_vibrate_init(void) {
  window = window_create();
  window_set_background_color(window, GColorWhite);
  window_set_click_config_provider(window, (ClickConfigProvider)click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .appear = window_appear,
    .disappear = window_disappear
  });
  bmp_layer = bitmap_layer_create( GRect(PBL_IF_ROUND_ELSE(58,40), PBL_IF_ROUND_ELSE(54,44), 64, 64) );
  if ( bmp ) {
  gbitmap_destroy( bmp );
  bmp = NULL;
  };
  bmp=gbitmap_create_with_resource( RESOURCE_ID_IMAGE_ALARM );
  bitmap_layer_set_compositing_mode( bmp_layer, GCompOpSet );
  bitmap_layer_set_bitmap( bmp_layer, bmp );
  layer_add_child( window_get_root_layer(window) , bitmap_layer_get_layer( bmp_layer ));
};


void window_vibrate_show(void) {
  num_vibrate = MAX_VIBRATE;
  if (window_stack_contains_window(window)) {
    return;
  }
  window_stack_push(window, false);
};


void window_vibrate_deinit(void) {
  if ( bmp ) {
  gbitmap_destroy( bmp );
  bmp = NULL;
  };
  bitmap_layer_destroy(bmp_layer);
  window_destroy(window);
};


bool window_vibrate_is_visible(void) {
  return is_visible;
};

//----------------------------------------------------------------------------//

void click_config_provider(Window* window) {
  window_single_click_subscribe(BUTTON_ID_UP, click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler);
};


void click_handler(ClickRecognizerRef recognizer, void* context) {
  window_stack_pop(true);
};


void window_appear(Window* window) {
  do_vibrate();
  is_visible = true;
};


void window_disappear(Window* window) {
  if ( vibe_timer != NULL ) {
      app_timer_cancel(vibe_timer);
      vibe_timer = NULL;
  };
  is_visible = false;
};


void do_vibrate(void) {
  num_vibrate--;
  // max reach
  if ( num_vibrate <= 1 ) {
    // close window
    window_stack_pop(true);
  } else {
  vibes_long_pulse();
  vibe_timer = app_timer_register(1000, vibe_timer_callback, NULL);
  };
};


void vibe_timer_callback(void* data) {
  vibe_timer = NULL;
  do_vibrate();
};
