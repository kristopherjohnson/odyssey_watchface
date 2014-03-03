/*
Copyright (c) 2014 Kristopher Johnson

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <pebble.h>
#include <ctype.h>

#define RESOURCE_ID_TIME_FONT RESOURCE_ID_FONT_DAYS_40
#define RESOURCE_ID_DATE_FONT RESOURCE_ID_FONT_DAYS_20

#define USE_FIXED_TEXT 0

static Window *window;
static TextLayer *time_text_layer;
static TextLayer *date_text_layer;

static void convert_to_uppercase(char s[]) {
  for (unsigned i = 0; s[i] != 0; ++i) {
      s[i] = toupper((unsigned)s[i]);
  }
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
#if USE_FIXED_TEXT
  text_layer_set_text(time_text_layer, "22:33");
  text_layer_set_text(date_text_layer, "WED 29")
#else
  static char time_text[] = "00:00";
  static char date_text[] = "XXX 00";

  char *time_format;

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(time_text_layer, time_text);

  strftime(date_text, sizeof(date_text), "%a %d", tick_time);
  convert_to_uppercase(date_text);
  text_layer_set_text(date_text_layer, date_text);
#endif
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  time_text_layer= text_layer_create((GRect) { .origin = { 0, 75 }, .size = { bounds.size.w, 40 } });
  text_layer_set_font(time_text_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TIME_FONT)));
  text_layer_set_text_alignment(time_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time_text_layer));

  date_text_layer= text_layer_create((GRect) { .origin = { 2, 55 }, .size = { bounds.size.w - 2, 20 } });
  text_layer_set_font(date_text_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DATE_FONT)));
  text_layer_set_text_alignment(date_text_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(date_text_layer));

  // Set initial text
  time_t t;
  time(&t);
  struct tm *tmst = localtime(&t);
  handle_minute_tick(tmst, 0);
}

static void window_unload(Window *window) {
  text_layer_destroy(time_text_layer);
  text_layer_destroy(date_text_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
