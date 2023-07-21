#include <stdlib.h>
#include <time.h>

#include "esp_sntp.h"
#include <Arduino_JSON.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "DEV_Config.h"
#include "keys.h"
#include "utility/EPD_12in48b.h"

extern "C" {
#include "art.h"
}

int Version = 1;

static bool download_weather_data(void);
static void draw_to_screen(void);
static void finish(void);
static bool connect_wifi(void);
static bool sync_time(void);
static bool check_time(void);
static void draw_S1(struct Image *image);
static void draw_M1(struct Image *image);
static void draw_S2(struct Image *image);
static void draw_M2(struct Image *image);

uint32_t art_random() { return esp_random(); }

struct WeatherData {
  uint16_t temperature;
} weather_data;

void setup() {
  DEV_ModuleInit();

  printf("Art Init\r\n");
  art_init();

  printf("Screen init\r\n");
  EPD_12in48B_Init();
}

void loop() {
  if (!check_time()) {
    return finish();
  }

  printf("Download weather data\r\n");
  if (!download_weather_data()) {
    printf("Something with downloading is wrong...\r\n");
    return finish();
  }

  printf("Current Weather: %dC\r\n", weather_data.temperature);

  printf("Make the art\r\n");
  art_make(weather_data.temperature, 32);

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

static bool connect_wifi(void) {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }
  printf("Init Wifi");
  WiFi.begin(SSID, PASSWORD);
  uint16_t count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    printf(".");
    if (count++ > 300) {
      printf(" failed.\r\n");
      return false;
    }
  }
  printf(" connected!\r\n");
  return true;
}

static bool sync_time(void) {
  sntp_sync_status_t sntp_sync = sntp_get_sync_status();
  if (sntp_sync == SNTP_SYNC_STATUS_COMPLETED) {
    return true;
  }

  if (!connect_wifi()) {
    return false;
  }

  printf("Synchronize time\r\n");

  sntp_init();
  // 24 hours, I don't need smooth synchronization because 
  // I need only is to get current hour.
  sntp_set_sync_interval(86400000L);
  sntp_restart();
  sntp_setservername(0, "pool.ntp.org");

  uint8_t count = 0;
  do {
    delay(100);
    sntp_sync = sntp_get_sync_status();
    if (count++ > 50) {
      break;
    }
  } while (sntp_sync != SNTP_SYNC_STATUS_COMPLETED);

  if (sntp_sync != SNTP_SYNC_STATUS_COMPLETED) {
    printf("Failed to update system time, ");
    switch (sntp_sync) {
    case SNTP_SYNC_STATUS_RESET:
      printf("State: SNTP_SYNC_STATUS_RESET\r\n");
      break;
    case SNTP_SYNC_STATUS_IN_PROGRESS:
      printf("State: SNTP_SYNC_STATUS_IN_PROGRESS\r\n");
      break;
    default:
      printf("State: ???\r\n");
      break;
    }
    return false;
  }

  printf("Set timezone: %s\r\n", TIMEZONE);
  setenv("TZ", TIMEZONE, 1);
  tzset();
  return true;
};

static bool check_time() {
  if (!sync_time()) {
    return false;
  }

  printf("Check time: ");
  time_t now;
  struct tm timeinfo;

  time(&now);
  localtime_r(&now, &timeinfo);
  printf("%02d:%02d:%02d\r\n", timeinfo.tm_hour, timeinfo.tm_min,
         timeinfo.tm_sec);

  if (timeinfo.tm_hour < 6) {
    printf("Screen doesn't work during midnight!\r\n");
    return false;
  }

  return true;
}

static String http_get(const std::string &uri) {
  // https://randomnerdtutorials.com/esp32-http-get-open-weather-map-thingspeak-arduino/
  if (!connect_wifi()) {
    return "{}";
  }

  WiFiClient client;
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin(client, uri.c_str());

  // Send HTT GET request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode >= 200 && httpResponseCode < 300) {
    printf("HTTP Response code: %d\r\n", httpResponseCode);
    payload = http.getString();
  } else {
    printf("Error code: %d\n", httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

static bool download_weather_data(void) {
  // https://randomnerdtutorials.com/esp32-http-get-open-weather-map-thingspeak-arduino/
  std::string uri =
      "http://api.openweathermap.org/data/2.5/weather?units=metric&q=";
  String jsonBuffer =
      http_get(uri + CITY + "," + COUNTRY + "&APPID=" + OPEN_WEATHER_API_KEY);
  // Serial.println(jsonBuffer);
  JSONVar myObject = JSON.parse(jsonBuffer.c_str());

  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    printf("Parsing weather data failed!");
    return false;
  }

  weather_data.temperature = myObject["main"]["temp"];
  return true;
}

static void draw_to_screen(void) {
  static struct Image *image = NULL;
  if (!image) {
    image = (struct Image *)malloc(sizeof(struct Image));
  }

  image->offset.x = 0;
  image->offset.y = 0;
  art_draw(image);
  draw_S2(image);

  image->offset.x = EPD_12in48B_S2_WIDTH;
  art_draw(image);
  draw_M2(image);

  image->offset.x = 0;
  image->offset.y = EPD_12in48B_S2_HEIGHT;
  art_draw(image);
  draw_M1(image);

  image->offset.x = EPD_12in48B_S2_WIDTH;
  art_draw(image);
  draw_S1(image);
}

static void draw_S1(struct Image *image) {
  uint16_t x, y;

  // BLACK
  EPD_S1_SendCommand(0x10);
  for (y = 0; y < EPD_12in48B_S1_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_S1_WIDTH; x += 8) {
      uint8_t data = 0xFF;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == BLACK) {
          data &= ~(1 << (7 - i));
        }
      }
      EPD_S1_SendData(data);
    }
  }

  // RED
  EPD_S1_SendCommand(0x13);
  for (y = 0; y < EPD_12in48B_S1_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_S1_WIDTH; x += 8) {
      uint8_t data = 0x00;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == RED) {
          data |= 1 << (7 - i);
        }
      }
      EPD_S1_SendData(data);
    }
  }
}

static void draw_M1(struct Image *image) {
  uint16_t x, y;

  // BLACK
  EPD_M1_SendCommand(0x10);
  for (y = 0; y < EPD_12in48B_M1_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_M1_WIDTH; x += 8) {
      uint8_t data = 0xFF;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == BLACK) {
          data &= ~(1 << (7 - i));
        }
      }
      EPD_M1_SendData(data);
    }
  }

  // RED
  EPD_M1_SendCommand(0x13);
  for (y = 0; y < EPD_12in48B_M1_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_M1_WIDTH; x += 8) {
      uint8_t data = 0x00;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == RED) {
          data |= 1 << (7 - i);
        }
      }
      EPD_M1_SendData(data);
    }
  }
}

static void draw_S2(struct Image *image) {
  uint16_t x, y;

  // BLACK
  EPD_S2_SendCommand(0x10);
  for (y = 0; y < EPD_12in48B_S2_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_S2_WIDTH; x += 8) {
      uint8_t data = 0xFF;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == BLACK) {
          data &= ~(1 << (7 - i));
        }
      }
      EPD_S2_SendData(data);
    }
  }

  // RED
  EPD_S2_SendCommand(0x13);
  for (y = 0; y < EPD_12in48B_S2_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_S2_WIDTH; x += 8) {
      uint8_t data = 0x00;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == RED) {
          data |= 1 << (7 - i);
        }
      }
      EPD_S2_SendData(data);
    }
  }
}

static void draw_M2(struct Image *image) {
  uint16_t x, y;

  // BLACK
  EPD_M2_SendCommand(0x10);
  for (y = 0; y < EPD_12in48B_M2_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_M2_WIDTH; x += 8) {
      uint8_t data = 0xFF;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == BLACK) {
          data &= ~(1 << (7 - i));
        }
      }
      EPD_M2_SendData(data);
    }
  }

  // RED
  EPD_M2_SendCommand(0x13);
  for (y = 0; y < EPD_12in48B_M2_HEIGHT; y++) {
    for (x = 0; x < EPD_12in48B_M2_WIDTH; x += 8) {
      uint8_t data = 0x00;
      for (uint16_t i = 0; i < 8; i++) {
        if (image_get(image, x + i, y) == RED) {
          data |= 1 << (7 - i);
        }
      }
      EPD_M2_SendData(data);
    }
  }
}
