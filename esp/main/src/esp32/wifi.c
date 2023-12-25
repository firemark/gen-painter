#include "esp32/wifi.h"

#include "esp_event.h"
#include "esp_wifi.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "keys.h"

static bool _is_connected = false;
static int _retry_num = 0;
static EventGroupHandle_t _wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static void _wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (_retry_num < 5) {
      esp_wifi_connect();
      _retry_num++;
    } else {
      xEventGroupSetBits(_wifi_event_group, WIFI_FAIL_BIT);
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    _retry_num = 0;
    xEventGroupSetBits(_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

bool connect_wifi(void) {
  if (_is_connected) {
    return true;
  }
  printf("Init Wifi");
  _wifi_event_group = xEventGroupCreate();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = SSID,
              .password = PASSWORD,
              .threshold.authmode = WIFI_AUTH_WPA2_PSK,
          },
  };

  if (esp_netif_init() != ESP_OK) {
    printf("esp_netif_init failed.\r\n");
    return false;
  }
  if (esp_event_loop_create_default() != ESP_OK) {
    printf("esp_event_loop_create_default failed.\r\n");
    return false;
  }
  if (!esp_netif_create_default_wifi_sta()) {
    printf("esp_netif_create_default_wifi_sta failed.\r\n");
    return false;
  }
  if (esp_wifi_init(&cfg) != ESP_OK) {
    printf("esp_wifi_init failed.\r\n");
    return false;
  }

  esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                      &_wifi_event_handler, NULL, NULL);
  esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                      &_wifi_event_handler, NULL, NULL);

  if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK) {
    printf("esp_wifi_set_storage failed.\r\n");
    return false;
  }
  if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) {
    printf("esp_wifi_set_mode failed.\r\n");
    return false;
  }
  if (esp_wifi_set_config(WIFI_IF_STA, &wifi_config) != ESP_OK) {
    printf("esp_wifi_set_config failed.\r\n");
    return false;
  }
  if (esp_wifi_start() != ESP_OK) {
    printf("esp_wifi_start failed.\r\n");
    return false;
  }

  EventBits_t bits =
      xEventGroupWaitBits(_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                          pdFALSE, pdFALSE, portMAX_DELAY);
  if (bits & WIFI_CONNECTED_BIT) {
    printf(" connected!\r\n");
    _is_connected = true;
    return true;
  } else {
    printf(" failed.\r\n");
  }
  return false;
}