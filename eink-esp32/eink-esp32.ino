#include <stdlib.h>
#include <time.h>

#include "esp_sntp.h"
#include <ArduinoJson.h>
#include <HttpClient.h>
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

struct ArtData _data;

void setup() {
  DEV_ModuleInit();

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

  printf("Art Init\r\n");
  if (!art_init()) {
    printf("Not enough memory to initialize art.\r\n");
    printf("Available memory: %ld\n",
           heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
    return finish();
  }

  printf("Make the art\r\n");
  art_make(_data);

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

  // if (timeinfo.tm_hour < 6) {
  //   printf("Screen doesn't work during midnight!\r\n");
  //   return false;
  // }

  _data.minute = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  return true;
}

static enum WeatherType weather_id_to_type(uint16_t weather_id) {
  switch (weather_id / 100) {
  case 2:
    return WEATHER_THUNDERSTORM;
  case 3:
    return WEATHER_DRIZZLE;
  case 5:
    return weather_id < 520 ? WEATHER_RAIN : WEATHER_SHOWER_RAIN;
  case 6:
    return weather_id < 620 ? WEATHER_SNOW : WEATHER_SHOWER_SNOW;
  case 7:
    return WEATHER_FOG;
  case 8:
    switch (weather_id) {
    case 800:
      return WEATHER_CLEAR;
    case 801:
      return WEATHER_FEW_CLOUDS;
    default:
      return WEATHER_MANY_CLOUDS;
    }
  }
  return WEATHER_WTF;
}

static bool download_weather_data(void) {
  if (!connect_wifi()) {
    return false;
  }

  // https://randomnerdtutorials.com/esp32-http-get-open-weather-map-thingspeak-arduino/
  std::string uri = "/data/2.5/forecast?units=metric&cnt=4";
  uri = uri + "&lat=" + LATITUDE + "&lon=" + LONGITUDE;
  uri = uri + "&APPID=" + OPEN_WEATHER_API_KEY;

  DynamicJsonDocument response(2560);
  DeserializationError error;

  {
    WiFiClient client;
    HttpClient http(client);
    int conn_err = http.get("api.openweathermap.org", uri.c_str());
    if (conn_err != 0) {
      printf("Connection error: %d\n", conn_err);
    }
    int status = http.responseStatusCode();
    printf("HTTP CODE: %d\n", status);
    http.skipResponseHeaders();
    error = deserializeJson(response, http);
    http.stop();
  }

  if (error) {
    printf("Parsing weather data failed: %s \n", error.f_str());
    return false;
  }

  for (int i = 0; i < 4; i++) {
    const JsonVariant item = response["list"][i].as<JsonVariant>();
    struct Forecast *forecast = &_data.forecast[i];
    uint16_t weather_id = item["weather"][0]["id"];

    time_t dt = item["dt"];
    struct tm timeinfo;
    localtime_r(&dt, &timeinfo);

    forecast->type = weather_id_to_type(weather_id);
    forecast->hour = timeinfo.tm_hour;
    forecast->minute = timeinfo.tm_min;
    forecast->temperature = item["main"]["temp"];

    printf("Forecast %d id: %3d, timestamp: %ld time: %02d:%02d; temperature: "
           "%2d\n",
           i, weather_id, dt, forecast->hour, forecast->minute,
           forecast->temperature);
  }

  {
    JsonVariant item = response["list"][0].as<JsonVariant>();
    _data.rain_density = static_cast<double>(item["rain"]["3h"]) * 8.0;
    _data.snow_density = static_cast<double>(item["snow"]["3h"]) * 8.0;
    _data.clouds_count = static_cast<double>(item["clouds"]["all"]) / 4.0;
    printf("Rain density: %d; clouds: %d\n", _data.rain_density,
           _data.clouds_count);
  }

  return true;
}

static void draw_to_screen(void) {
  struct Image *image = image_create();
  if (!image) {
    printf("Not enough memory to draw! available memory: %ld; required: %ld\n",
           heap_caps_get_free_size(MALLOC_CAP_DEFAULT), sizeof(struct Image));
    return;
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

  image_destroy(image);
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
