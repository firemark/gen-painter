#include "esp32/wifi.h"

#include "esp_event.h"
#include "esp_wifi.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "keys.h"

static bool _is_connected = false;

static void _wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    esp_wifi_connect();
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    _is_connected = true;
  }
}

bool connect_wifi(void) {
  if (_is_connected) {
    return true;
  }
  printf("Init Wifi");

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = SSID,
              .password = PASSWORD,
              .threshold.authmode = WIFI_AUTH_WPA2_PSK,
          },
  };

  esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                      &_wifi_event_handler, NULL, NULL);
  esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                      &_wifi_event_handler, NULL, NULL);

  if (esp_netif_init() != ESP_OK) {
    return false;
  }
  if (esp_event_loop_create_default() != ESP_OK) {
    return false;
  }
  if (esp_wifi_init(&cfg) != ESP_OK) {
    return false;
  }
  if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK) {
    return false;
  }
  if (!esp_netif_create_default_wifi_sta()) {
    return false;
  }
  if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) {
    return false;
  }
  if (esp_wifi_set_config(WIFI_IF_STA, &wifi_config) != ESP_OK) {
    return false;
  }
  if (esp_wifi_start() != ESP_OK) {
    return false;
  }

  uint16_t count = 0;
  while (!_is_connected) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
    printf(".");
    if (count++ > 300) {
      printf(" failed.\r\n");
      return false;
    }
  }
  printf(" connected!\r\n");
  return true;
}