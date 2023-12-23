/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

#include "esp32/wifi.h"
#include "esp32/time.h"
#include "esp32/epd.h"
#include "esp32/weather.h"
#include "esp32/draw.h"

// #include "esp_chip_info.h"
// #include "esp_flash.h"
// #include "esp_system.h
#include "esp_heap_caps.h"
#include "esp_sleep.h"
#include "esp_random.h"

#include "art/art.h"
#include "keys.h"

#include "sdkconfig.h"

static void finish(void);
// static void debug(void);

uint32_t art_random() { return esp_random(); }

void app_main(void)
{
  struct ArtData data;
  printf("Available memory: %u\n",
       heap_caps_get_free_size(MALLOC_CAP_DEFAULT));

  printf("Screen init\r\n");
  EPD_12in48B_Init();

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
    printf("Available memory: %u\n",
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




// static void debug(void) {
//     /* Print chip information */
//     esp_chip_info_t chip_info;
//     uint32_t flash_size;
//     esp_chip_info(&chip_info);
//     printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
//            CONFIG_IDF_TARGET,
//            chip_info.cores,
//            (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
//            (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
//            (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
//            (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

//     unsigned major_rev = chip_info.revision / 100;
//     unsigned minor_rev = chip_info.revision % 100;
//     printf("silicon revision v%d.%d, ", major_rev, minor_rev);
//     if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
//         printf("Get flash size failed");
//         return;
//     }

//     printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
//            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

//     printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
// }