#include <pebble.h>

#define NUM_MENU_ITEMS 5
#define DEP_NUM_MENU_ITEMS 5

static Window *window;
static Window *dep_window;

static MenuLayer *menu_layer;
static MenuLayer *dep_menu_layer;

typedef struct stop_t {
  char *name;
  char *code;
} Stop;
static Stop stops[NUM_MENU_ITEMS];
static int menu_size = 0;

typedef struct departure_t {
  char *headsign;
  char *time;
} Departure;
static Departure departures[DEP_NUM_MENU_ITEMS];
static char *dep_code = "Departures";
static int dep_menu_size = 0;

static const bool animated = true;

enum {
  stop = 0,
  code = 1
};

enum {
  headsign = 2,
  esttime = 3
};

static void update_code(char *new_code) {
  dep_code = malloc(sizeof(char)*(strlen(new_code)+1+12));
  strcpy(dep_code, "Departures: ");
  strcat(dep_code, new_code);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "New Dep Code: %s", dep_code);
}

static void append_menu(char *stop, char *code) {
  if (menu_size == NUM_MENU_ITEMS) {
    return;
  }
  stops[menu_size].name = malloc(sizeof(char)*(strlen(stop)+1));
  strcpy(stops[menu_size].name, stop);

  stops[menu_size].code = malloc(sizeof(char)*(strlen(code)+1));
  strcpy(stops[menu_size].code, code);

  menu_size += 1;

  menu_layer_reload_data(menu_layer);
}

static void dep_append_menu(char *headsign, char *minutes) {
  if (dep_menu_size == DEP_NUM_MENU_ITEMS) {
    return;
  }
  departures[dep_menu_size].headsign = malloc(sizeof(char)*(strlen(headsign)+1));
  strcpy(departures[dep_menu_size].headsign, headsign);

  departures[dep_menu_size].time = malloc(sizeof(char)*(strlen(minutes)+1));
  strcpy(departures[dep_menu_size].time, minutes);

  dep_menu_size += 1;

  menu_layer_reload_data(dep_menu_layer);
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 1;
}

static uint16_t dep_menu_get_num_sections_callback(MenuLayer *dep_menu_layer, void *data) {
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  if (section_index == 0) {
    return menu_size;
  }
  else {
    return 0;
  }
}

static uint16_t dep_menu_get_num_rows_callback(MenuLayer *dep_menu_layer, uint16_t section_index, void *data) {
  if (section_index == 0) {
    return dep_menu_size;
  }
  else {
    return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t dep_menu_get_header_height_callback(MenuLayer *dep_menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  if (section_index == 0) {
    menu_cell_basic_header_draw(ctx, cell_layer, "Nearby Stops");
  }
}

static void dep_menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  if (section_index == 0) {
    menu_cell_basic_header_draw(ctx, cell_layer, dep_code);
  }
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->section == 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Drawing Row: %s, %s", stops[cell_index->row].name, stops[cell_index->row].code);
    menu_cell_basic_draw(ctx, cell_layer, stops[cell_index->row].name, stops[cell_index->row].code, NULL);
  }
}

static void dep_menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->section == 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Drawing Row: %s, %s", departures[cell_index->row].headsign, departures[cell_index->row].time);
    menu_cell_basic_draw(ctx, cell_layer, departures[cell_index->row].headsign, departures[cell_index->row].time, NULL);
  }
}

void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->section == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    Tuplet value = TupletCString(code, stops[cell_index->row].code);
    dict_write_tuplet(iter, &value);
    app_message_outbox_send();
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Sending Subtitle: %s", stops[cell_index->row].code);
  window_stack_push(dep_window, animated);
}

void dep_menu_select_long_click_callback(MenuLayer *dep_menu_layer, MenuIndex *cell_index, void *data) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Reloading Departures");
  window_stack_pop(false);
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet value = TupletCString(code, dep_code+12);
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
  window_stack_push(dep_window, false);
}

void dep_menu_select_callback(MenuLayer *dep_menu_layer, MenuIndex *cell_index, void *data) {
/*  if (cell_index->section == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    Tuplet value = TupletCString(code, codes[cell_index->row]);
    dict_write_tuplet(iter, &value);
    app_message_outbox_send();
  }*/
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Sending Subtitle: %s", departures[cell_index->row].time);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  menu_layer = menu_layer_create(bounds);

  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  menu_layer_set_click_config_onto_window(menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void dep_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  dep_menu_layer = menu_layer_create(bounds);

  menu_layer_set_callbacks(dep_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = dep_menu_get_num_sections_callback,
    .get_num_rows = dep_menu_get_num_rows_callback,
    .get_header_height = dep_menu_get_header_height_callback,
    .draw_header = dep_menu_draw_header_callback,
    .draw_row = dep_menu_draw_row_callback,
    .select_click = dep_menu_select_callback,
    .select_long_click = dep_menu_select_long_click_callback,
  });

  menu_layer_set_click_config_onto_window(dep_menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(dep_menu_layer));
}

static void window_unload(Window *window) {
  int i;
  for (i = 0 ; i < menu_size ; i += 1) {
    if (stops[i].name) {
      free(stops[i].name);
    }
    if (stops[i].code) {
      free(stops[i].code);
    }
  }
  menu_size = 0;
  menu_layer_destroy(menu_layer);
}

static void dep_window_unload(Window *window) {
  int i;
  for (i = 0 ; i < dep_menu_size ; i += 1) {
    if (departures[i].headsign) {
      free(departures[i].headsign);
    }
    if (departures[i].time) {
      free(departures[i].time);
    }
  }
  dep_menu_size = 0;
  if (dep_code) {
    free(dep_code);
  }
  menu_layer_destroy(dep_menu_layer);
}

static void deinit(void) {
  window_destroy(dep_window);
  window_destroy(window);
}

void out_sent_handler(DictionaryIterator *sent, void *context) {
  // outgoing message was delivered
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Outgoing Message Delivered");
}


void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  // outgoing message failed
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Outgoing Message Failed");
}

void in_received_handler(DictionaryIterator *received, void *context) {
  // incoming message received
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message");

  // Check for fields you expect to receive
  Tuple *stop_tuple = dict_find(received, stop);
  if(!stop_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "NO STOP");
  }
  else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "HAVE STOP");
  }
  Tuple *code_tuple = dict_find(received, code);
  if(!code_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "NO CODE");
  }
  else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "HAVE CODE");
  }

  if (stop_tuple && code_tuple) {
    char *stopString = stop_tuple->value->cstring;
    char *codeString = code_tuple->value->cstring;
  
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stop: %s", stopString);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Code: %s", codeString);

    append_menu(stopString, codeString);
    return;
  }
  if (code_tuple) {
    char *codeString = code_tuple->value->cstring;

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Code: %s", codeString);

    update_code(codeString);
  }
  Tuple *hs_tuple = dict_find(received, headsign);
  if(!hs_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "NO HS");
  }
  else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "HAVE HS");
  }
  Tuple *time_tuple = dict_find(received, esttime);
  if(!time_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "NO TIME");
  }
  else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "HAVE TIME");
  }
  if (hs_tuple && time_tuple) {
    char *hsString = hs_tuple->value->cstring;
    char *timeString = time_tuple->value->cstring;

    APP_LOG(APP_LOG_LEVEL_DEBUG, "HS: %s", hsString);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "TIME: %s", timeString);

    dep_append_menu(hsString, timeString);
    return;
  }
}


void in_dropped_handler(AppMessageResult reason, void *context) {
  // incoming message dropped
}

static void init(void) {
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);

  const uint32_t inbound_size = 64;
  const uint32_t outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  window = window_create();
//  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  dep_window = window_create();
//  window_set_click_config_provider(dep_window, click_config_provider);
  window_set_window_handlers(dep_window, (WindowHandlers) {
    .load = dep_window_load,
    .unload = dep_window_unload,
  });
  window_stack_push(window, animated);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
