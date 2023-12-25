#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "esp32/draw.h"
#include "esp32/epd.h"
#include "esp32/time.h"
#include "esp32/weather.h"
#include "esp32/wifi.h"

#include "esp_heap_caps.h"
#include "esp_random.h"
#include "esp_sleep.h"
#include "nvs_flash.h"

#include "art/art.h"
#include "keys.h"

#include "sdkconfig.h"

static void finish(void);
// static void debug(void);

uint32_t art_random() { return esp_random(); }

void app_main(void) {
  struct ArtData data;
  printf("Available memory: %lu\n",
         heap_caps_get_free_size(MALLOC_CAP_DEFAULT));

  printf("Screen init\r\n");
  EPD_12in48B_Init();

  if (nvs_flash_init() != ESP_OK) {
    printf("nvs_flash_init failed.\r\n");
    return finish();
  }

  if (!check_time(&data.minute)) {
    return finish();
  }

  printf("Download weather data\r\n");
  if (!download_weather_data(&data)) {
    printf("Something with downloading is wrong...\r\n");
    return finish();
  }

  printf("Art Init\r\n");
  if (!art_init()) {
    printf("Not enough memory to initialize art.\r\n");
    printf("Available memory: %lu\n",
           heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
    return finish();
  }

  printf("Make the art\r\n");
  art_make(data);

  printf("Screen fill with the art\r\n");
  draw_to_screen();

  printf("Screen display\r\n");
  EPD_12in48B_TurnOnDisplay();

  finish();
}

static void finish(void) {
  printf("Screen sleep\r\n");
  EPD_12in48B_Sleep();

  printf("CPU sleep\r\n");
  esp_sleep_enable_timer_wakeup(3600000000L); // 1 hour
  esp_deep_sleep_start();
}