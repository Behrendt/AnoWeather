#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_CITY 2

int RefreshTime = 30;
bool ShowDate = true;
double NotchLength = 0.92;


int TimeSinceRefresh = -3;
static Window *main_window;
static Layer *notch_layer;
static TextLayer *date_layer;
static TextLayer *weather_layer;
static TextLayer *city_layer;
static Layer *hand_layer;

static GFont s_weather_font;
static GPoint s_center;
static GRect bounds;
//bool updated = false;

static void update_weather(){
  APP_LOG(APP_LOG_LEVEL_INFO,"Try update weather");
  Tuplet value = TupletInteger(1, 1);
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  if(iter == NULL){
    return;
  }
  dict_write_tuplet(iter, &value);
  dict_write_end(iter);
  app_message_outbox_send(); 
}
static void hand_update(Layer *layer, GContext *ctx)
{
  graphics_context_set_fill_color(ctx, GColorClear);
  graphics_fill_circle(ctx, s_center, (int16_t)(bounds.size.w/2));
  //APP_LOG(APP_LOG_LEVEL_INFO,"Half size %d", (int16_t)(bounds.size.w/2));
  // Adjust for minutes through the hour
  
   // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  float minute_angle = TRIG_MAX_ANGLE * tick_time -> tm_min / 60;
  float hour_angle;
 
  hour_angle = TRIG_MAX_ANGLE * tick_time -> tm_hour / 12;  
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);
  
  GPoint hour_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(hour_angle) * (int16_t)(bounds.size.w*0.27) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(bounds.size.w*0.27) / TRIG_MAX_RATIO) + s_center.y,
  };
  
  GPoint minute_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * tick_time -> tm_min / 60) * (int32_t)(bounds.size.w*0.38) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * tick_time -> tm_min / 60) * (int32_t)(bounds.size.w*0.38) / TRIG_MAX_RATIO) + s_center.y,
  };
  
  graphics_context_set_stroke_color(ctx, GColorBrass);
  graphics_context_set_stroke_width(ctx, 4);
  graphics_draw_line(ctx, s_center, hour_hand);
  
  graphics_context_set_stroke_color(ctx, GColorBulgarianRose);  
  graphics_draw_line(ctx, s_center, minute_hand); 
   //APP_LOG(APP_LOG_LEVEL_INFO,"I drew stuff");
  if(TimeSinceRefresh >= RefreshTime)
  {
    TimeSinceRefresh = 0;
    update_weather();
  }
  else
    TimeSinceRefresh++;
  
  APP_LOG(APP_LOG_LEVEL_INFO,"Time since refresh: %d", TimeSinceRefresh);
  
  static char s_battery_buffer[16];

  BatteryChargeState charge_state = battery_state_service_peek();
  if (charge_state.is_charging) {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "charging");
  } else {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%% charged", charge_state.charge_percent);
  }
  
}
char *itoa(int num)
{
  static char buff[20] = {};
  int i = 0, temp_num = num, length = 0;
  char *string = buff;
  
  if(num >= 0) { //See NOTE
    // count how many characters in the number
    while(temp_num) {
      temp_num /= 10;
      length++;
    }
    // assign the number to the buffer starting at the end of the 
    // number and going to the begining since we are doing the
    // integer to character conversion on the last number in the
    // sequence
    for(i = 0; i < length; i++) {
      buff[(length-1)-i] = '0' + (num % 10);
      num /= 10;
    }
    buff[i] = '\0'; // can't forget the null byte to properly end our string
  }
  else {
    return "Unsupported Number";
  }
  
  return string;
}
static void set_date_text()
{
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int DayInt =  tick_time->tm_wday;
  int MonInt =  tick_time->tm_mon;
  //int MdayInt =  tick_time->tm_mday;
  char MdayChar[2];
  strcpy(MdayChar, itoa(tick_time->tm_mday));
  
  //APP_LOG(APP_LOG_LEVEL_INFO,"Day: %d", tick_time->tm_mday);
  char strDay[3];
  char strMon[5];
  static char DateChar[15];
  static char TempBuff[8];
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, GColorBlack);
  
  text_layer_set_text(date_layer, "TEMP");
  switch(DayInt)
    {
    case 0:  strcpy(strDay, "Sun");
      break;
    case 1:  strcpy(strDay, "Mon");
      break;
    case 2:  strcpy(strDay, "Tue");
      break;
    case 3:  strcpy(strDay, "Wed");
      break;
    case 4:  strcpy(strDay, "Thu");
      break;
    case 5:  strcpy(strDay, "Fri");
      break;
    case 6:  strcpy(strDay, "Sat");
      break;    
    default:
    break;
  }
  
  switch(MonInt)
    {
    case 0:  strcpy(strMon, "Jan");
      break;
    case 1:  strcpy(strMon, "Feb");
      break;
    case 2:  strcpy(strMon, "Mar");
      break;
    case 3:  strcpy(strMon, "Apr");
      break;
    case 4:  strcpy(strMon, "May");
      break;
    case 5:  strcpy(strMon, "Jun");
      break;
    case 6:  strcpy(strMon, "Jul");
      break;
    case 7:  strcpy(strMon, "Aug");
      break;
    case 8:  strcpy(strMon, "Sep");
      break;
    case 9:  strcpy(strMon, "Oct");
      break;
    case 10:  strcpy(strMon, "Nov");
      break;
    case 11:  strcpy(strMon, "Dec");
      break;
    default:
    break;
  }
  
  snprintf(TempBuff, sizeof(TempBuff), "%s %s", strDay, MdayChar);
  //APP_LOG(APP_LOG_LEVEL_INFO,"Date so far: %s", DateChar);
  snprintf(DateChar, sizeof(DateChar), "%s\n%s", TempBuff, strMon);
  //APP_LOG(APP_LOG_LEVEL_INFO,"Full Date: %s", DateChar);
   text_layer_set_text(date_layer, DateChar);
}
static void update_time()
{
  if(ShowDate)
    set_date_text();
  
  layer_set_update_proc(hand_layer, hand_update);
}
static void ticktime_handler(struct tm *tick_time, TimeUnits units_changed)
{
  update_time();
}
static void NotchCreate(Layer *layer, GContext *ctx)
{
  //make layer clear
  graphics_context_set_fill_color(ctx, GColorClear);
  graphics_fill_circle(ctx, s_center, (int16_t)(bounds.size.w/2));
  //APP_LOG(APP_LOG_LEVEL_INFO,"Half size %d", (int16_t)(bounds.size.w/2));
  
   for(int i=0; i<60; i++){
    graphics_context_set_stroke_color(ctx, GColorDarkGray);
    graphics_context_set_stroke_width(ctx, 1);
    GPoint Notch = (GPoint){
      .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * i / 60)*(int32_t)(bounds.size.w/2)/TRIG_MAX_RATIO)+s_center.x,
      .y = (int16_t)(cos_lookup(TRIG_MAX_ANGLE * i / 60)*(int32_t)(bounds.size.w/2)/TRIG_MAX_RATIO)+s_center.y,
    };
    
    GPoint End = (GPoint){
      .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * i / 60)*(int32_t)((bounds.size.w/2)*NotchLength)/TRIG_MAX_RATIO)+s_center.x,
      .y = (int16_t)(cos_lookup(TRIG_MAX_ANGLE * i / 60)*(int32_t)((bounds.size.w/2)*NotchLength)/TRIG_MAX_RATIO)+s_center.y,
    };
    
    if(i % 5 == 0)
    {
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_context_set_stroke_width(ctx, 2);
      End = (GPoint){
      .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * i / 60)*(int32_t)((bounds.size.w/2)*NotchLength-0.04)/TRIG_MAX_RATIO)+s_center.x,
      .y = (int16_t)(cos_lookup(TRIG_MAX_ANGLE * i / 60)*(int32_t)((bounds.size.w/2)*NotchLength-0.04)/TRIG_MAX_RATIO)+s_center.y,
    };
    }
    graphics_draw_line(ctx, Notch, End);
  }
  
}

static void window_load(Window *window)
{
  //Defines bounds
  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);
  s_center = grect_center_point(&bounds);
  
  //Set up the minute notches
  notch_layer = layer_create(bounds);
  layer_set_update_proc(notch_layer, NotchCreate);
  layer_add_child(window_layer, notch_layer);
  
  //create weather font
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_WEATHER_FONT_12));
  
  //create weather layer
  // Create temperature Layer
  weather_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(0, 4), PBL_IF_ROUND_ELSE((int16_t)(bounds.size.h*0.07), 0), bounds.size.w, 50));

  city_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(0, 4), PBL_IF_ROUND_ELSE((int16_t)(bounds.size.h*0.85), (int16_t)(bounds.size.h*0.91)), bounds.size.w, 50));

  // Style the text
  text_layer_set_background_color(weather_layer, GColorClear);
  text_layer_set_text_color(weather_layer, GColorBlack);
  text_layer_set_text_alignment(weather_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft) );
  text_layer_set_font(weather_layer, s_weather_font);  
  text_layer_set_text(weather_layer, "Loading...");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(weather_layer));
  
  text_layer_set_background_color(city_layer, GColorClear);
  text_layer_set_text_color(city_layer, GColorBlack);
  text_layer_set_text_alignment(city_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft) );
  text_layer_set_font(city_layer, s_weather_font);  
  text_layer_set_text(city_layer, "Loading...");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(city_layer));
  
  
  //Create date layer
  date_layer = text_layer_create(
    GRect(PBL_IF_ROUND_ELSE((int16_t)((bounds.size.w)/2*0.58),0), PBL_IF_ROUND_ELSE((int16_t)(bounds.size.h*0.42),0), bounds.size.w-4, 50));
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, GColorBlack);
  text_layer_set_text_alignment(date_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentRight));
  text_layer_set_font(date_layer, s_weather_font);    
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
  
  //create hand layer
  hand_layer = layer_create(bounds);
  layer_set_update_proc(hand_layer, hand_update);
  layer_add_child(window_layer, hand_layer);

  
}
static void window_unload(Window *window)
{
  layer_destroy(notch_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(weather_layer);
  layer_destroy(hand_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message received by Pebble");
  
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  static char city_buffer[32];
  
  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);
  Tuple *city_tuple = dict_find(iterator, KEY_CITY);
  
  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    
    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s\n%s", temperature_buffer, conditions_buffer);
    text_layer_set_text(weather_layer, weather_layer_buffer);    
  }
  else{
    APP_LOG(APP_LOG_LEVEL_ERROR, "Something was missing");
  }
  if(city_tuple)
  {
    snprintf(city_buffer, sizeof(city_buffer), "%s", city_tuple->value->cstring);
    text_layer_set_text(city_layer, city_buffer);
  }
  else
    APP_LOG(APP_LOG_LEVEL_ERROR, "City is missing");
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


static void init()
{
  main_window = window_create();
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  
  window_stack_push(main_window, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, ticktime_handler);
  
  window_set_background_color(main_window, GColorWhite);
  
  update_time();
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}
static void deinit()
{
  window_destroy(main_window);
}
int main(void)
{
  init();
  app_event_loop();
  deinit();
}