#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "esp32/draw.h"
#include "esp32/epd.h"
#include "esp32/time.h"
#include "esp32/weather.h"
#include "esp32/wifi.h"

#include "neopixel.h"
#include "esp_heap_caps.h"
#include "esp_random.h"
#include "esp_sleep.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "nvs_flash.h"

#include "art/art.h"
#include "keys.h"

#include "sdkconfig.h"

#define PRINT_TIME printf("Total time: %0.2fs\r\n", (float)(t1 - t0)*1e-6)
#define SET_PIXEL(R, G, B) { tNeopixel pixel = { 0, NP_RGB(R, G, B) }; neopixel_SetPixel(neopixel, &pixel, 1); }


static void finish(void);
// static void debug(void);

uint32_t art_random() { return esp_random(); }

void app_main(void) {
  int64_t t0 = 0;
  int64_t t1 = 0;
  esp_task_wdt_add(NULL);
  struct ArtData data;
  printf("Available memory: %u\n",
         heap_caps_get_free_size(MALLOC_CAP_DEFAULT));

  tNeopixelContext neopixel = neopixel_Init(1, 48);
  SET_PIXEL(127, 127, 127);

  printf("Screen init\r\n");
  EPD_12in48B_Init();

  if (nvs_flash_init() != ESP_OK) {
    printf("nvs_flash_init failed.\r\n");
    SET_PIXEL(127, 0, 0);
    return finish();
  }

  if (!check_time(&data.minute)) {
    SET_PIXEL(127, 0, 0);
    return finish();
  }

  printf("Download weather data\r\n");
  SET_PIXEL(0, 0, 127);
  t0 = esp_timer_get_time();
  if (!download_weather_data(&data)) {
    printf("Something with downloading is wrong...\r\n");
    return finish();
  }
  t1 = esp_timer_get_time();
  PRINT_TIME;

  printf("Art Init\r\n");
  SET_PIXEL(0, 127, 0);
  t0 = esp_timer_get_time();
  if (!art_init()) {
    printf("Not enough memory to initialize art.\r\n");
    printf("Available memory: %u\n",
           heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
    return finish();
  }
  t1 = esp_timer_get_time();
  PRINT_TIME;

  printf("Make the art\r\n");
  SET_PIXEL(127, 127, 0);
  t0 = esp_timer_get_time();
  art_make(data);
  t1 = esp_timer_get_time();
  PRINT_TIME;

  printf("Screen fill with the art\r\n");
  SET_PIXEL(0, 127, 127);
  t0 = esp_timer_get_time();
  draw_to_screen();
  t1 = esp_timer_get_time();
  PRINT_TIME;

  printf("Screen display\r\n");
  SET_PIXEL(0, 50, 127);
  t0 = esp_timer_get_time();
  EPD_12in48B_TurnOnDisplay();
  t1 = esp_timer_get_time();
  PRINT_TIME;

  SET_PIXEL(0, 0, 0);
  finish();
}

static void finish(void) {
  printf("Screen sleep\r\n");
  EPD_12in48B_Sleep();

  printf("CPU sleep\r\n");
  esp_sleep_enable_timer_wakeup(3600000000L); // 1 hour
  esp_deep_sleep_start();
}