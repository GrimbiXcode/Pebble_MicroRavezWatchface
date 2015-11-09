/********************************************************************
* Project:          MicroRavezWatchface
* Description:      Simple Watchface with the Logo of MircoRavez as Background
* Developer:        GrimbiXcode (David Grimbichler)
* File:             main.c
* First Release:    2015-07-30
********************************************************************/

/********************************************************************
* Includes
*
********************************************************************/
#include <pebble.h>
#include <pebble_fonts.h>

  
/********************************************************************
* Variables
*
********************************************************************/
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_d_layer;
static TextLayer *s_date_m_layer;
static TextLayer *s_battery_layer;
static GBitmap *s_background_bitmap;
static BitmapLayer *s_background_bitmap_layer;
// batteryBar Layer
static Layer *s_batteryBarLayer;
static uint8_t s_batteryLevel;
static GPath *s_path;
static GPathInfo PATH_INFO = {
  .num_points = 10,
  .points = (GPoint[]) { {0, 12}, {7, 12}, {17, 4}, {134, 4}, {124, 12}, {144, 12}, {144, 0}, {15, 0}, {0, 0} }
};

/********************************************************************
* Routines
*
********************************************************************/

/*===================================================================
* update time
===================================================================*/
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  // Create a long-lived buffer
  static char buffer[] = "00:00";
  static char buffer_d[] = "00";
  static char buffer_m[] = "00";

  // Write the current date (day and month)
  strftime(buffer_d, sizeof("00"), "%e", tick_time);
  strftime(buffer_m, sizeof("00"), "%m", tick_time);
  
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  text_layer_set_text(s_date_d_layer, buffer_d);
  text_layer_set_text(s_date_m_layer, buffer_m);
}

/*===================================================================
* batteryBar handler
===================================================================*/
static void layer_update_proc(Layer *layer, GContext *ctx) {
  // Set the color using RGB values for Green
  graphics_context_set_fill_color(ctx, GColorFromRGB(19,168,73));
  
  graphics_fill_rect(ctx, GRect(5, 3, s_batteryLevel, 5), 0, GCornersAll);
  
  // Set the color using RGB values for Grey
  graphics_context_set_fill_color(ctx, GColorFromRGB(110, 110, 110));
  
  // Draw the filled shape in above color
  gpath_draw_filled(ctx, s_path);
}

/*===================================================================
* battery level handler
===================================================================*/
static void battery_handler(BatteryChargeState charge_state) {
  static char battery_text[] = "100% charged";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
    text_layer_set_text(s_battery_layer, battery_text);
  } else {
    if(charge_state.is_plugged){
      snprintf(battery_text, sizeof(battery_text), "ready");
      text_layer_set_text(s_battery_layer, battery_text);
    }
    //snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
    
    snprintf(battery_text, sizeof(battery_text), " ");
    text_layer_set_text(s_battery_layer, battery_text);
  }
  // Save Battery Level
  s_batteryLevel = (charge_state.charge_percent/10)*13; // max 130

  // Call draw-update handler
  layer_set_update_proc(s_batteryBarLayer, layer_update_proc);
  
}
/*===================================================================
* time-tick handler
===================================================================*/
static void tick_handler(struct tm *tick_time, TimeUnits unit_changed){
  update_time();
}

/********************************************************************
* Init-code
*
********************************************************************/

/*===================================================================
* load main window
===================================================================*/
static void main_window_load(Window *window){
  // Create the Background
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds =layer_get_bounds(window_layer);
  
  //initialyze batteryLevel
  s_batteryLevel = 50;
  
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ONE);
  s_background_bitmap_layer = bitmap_layer_create(bounds);
  
  bitmap_layer_set_bitmap(s_background_bitmap_layer, s_background_bitmap);
  #ifdef PBL_PLATFORM_APLITE
    bitmap_layer_set_compositing_mode(s_background_bitmap_layer, GCompOpAssign);
  #elif PBL_PLATFORM_BASALT
    bitmap_layer_set_compositing_mode(s_background_bitmap_layer, GCompOpSet);
  #endif 
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 130, bounds.size.w, 35));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorFromRGB(19,168,73));
  text_layer_set_text(s_time_layer, "00:00");
  
  //Create date TextLayer
  s_date_d_layer = text_layer_create(GRect(0, 30, bounds.size.w, 30));
  text_layer_set_background_color(s_date_d_layer, GColorClear);
  text_layer_set_text_color(s_date_d_layer, GColorFromRGB(19,168,73));
  text_layer_set_text(s_date_d_layer, "00");
  s_date_m_layer = text_layer_create(GRect(0, 30, bounds.size.w, 30));
  text_layer_set_background_color(s_date_m_layer, GColorClear);
  text_layer_set_text_color(s_date_m_layer, GColorFromRGB(19,168,73));
  text_layer_set_text(s_date_m_layer, "00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_d_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_date_d_layer, GTextAlignmentLeft);
  text_layer_set_font(s_date_m_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_date_m_layer, GTextAlignmentRight);
  
  // Create battery TextLayer
  s_battery_layer = text_layer_create(GRect(0, 6, bounds.size.w, bounds.size.h-6));
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorFromRGB(19,168,73));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  
  // Improve the layout to be more like a battery state
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  
  // Create BatteryBarLayer
  s_path = gpath_create(&PATH_INFO);
  s_batteryBarLayer = layer_create(bounds);
  layer_set_update_proc(s_batteryBarLayer, layer_update_proc);

  // Add Layers as a child layer to the Window's root layer
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_bitmap_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_d_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_m_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  layer_add_child(window_layer, s_batteryBarLayer);
  
  // Make sure the time is displayed from the start
  update_time();
  
  // Get the current battery level
  battery_handler(battery_state_service_peek());
}

/*===================================================================
* unload main window
===================================================================*/
static void main_window_unload(Window *window){
  // Destroy Backgroundlayer
  bitmap_layer_destroy(s_background_bitmap_layer);
  gbitmap_destroy(s_background_bitmap);
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_d_layer);
  text_layer_destroy(s_date_m_layer);
  text_layer_destroy(s_battery_layer);
  // Destroy BatteryBarlayer
  layer_destroy(s_batteryBarLayer);
  gpath_destroy(s_path);
}

/*===================================================================
* initialising Watchface
===================================================================*/
static void init(){
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  #ifdef PBL_SDK_2
    window_set_fullscreen(s_main_window, true);
  #endif

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Subscribe to the Battery State Service
  battery_state_service_subscribe(battery_handler);
}

/*===================================================================
* deinitialising Watchface
===================================================================*/
static void deinit(){
  // Destroy Window
  window_destroy(s_main_window);
}

/********************************************************************
* Main-code
********************************************************************/

int main(void){
  init();
  app_event_loop();
  deinit();
}

/********************************************************************
* End of code
********************************************************************/