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

  printf("Screen init\r\n");
  EPD_12in48B_Init();

  printf("Wifi Init");
  DEV_TestLED();
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    printf(".");
  }
  printf("\r\n");

  printf("Art Init\r\n");
  DEV_TestLED();
  art_init();
}

void loop() {
  printf("Settings time\r\n");
  setenv("TZ", TIMEZONE, 1);
  tzset();
  sntp_setservername(0, "pool.ntp.org");
  sntp_init();

  sntp_sync_status_t sntp_sync;
  uint8_t count = 0;
  do {
    delay(500);
    sntp_sync = sntp_get_sync_status();
    if (count++ < 5) {
      break;
    }
  } while (sntp_sync != SNTP_SYNC_STATUS_COMPLETED);

  if (sntp_sync != SNTP_SYNC_STATUS_COMPLETED) {
    printf("Failed to update system time\r\n");
    EPD_12in48B_Clear();
    finish();
  }

  printf("Check day\r\n");
  time_t now;
  struct tm timeinfo;

  time(&now);
  localtime_r(&now, &timeinfo);
  if (timeinfo.tm_hour < 6) {
    printf("Screen doesn't work during midnight!\r\n");
    finish();
  }

  printf("Download weather data\r\n");
  if (!download_weather_data()) {
    printf("Something with downloading is wrong...\r\n");
    EPD_12in48B_Clear();
    finish();
  }

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
  esp_sleep_enable_timer_wakeup(3600L * 1000000L); // 1 hour
  esp_deep_sleep_start();
}

static String http_get(const std::string &uri) {
  // https://randomnerdtutorials.com/esp32-http-get-open-weather-map-thingspeak-arduino/
  WiFiClient client;
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin(client, uri.c_str());

  // Send HTT GET request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    printf("HTTP Response code: %d", httpResponseCode);
    payload = http.getString();
  } else {
    printf("Error code: %d\n", httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

static bool download_weather_data(void) {
  return false;
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
