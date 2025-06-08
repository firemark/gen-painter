#include "esp32/weather.h"

#include <stdio.h>
#include <cJSON.h>

#include "esp32/wifi.h"

#include "esp_http_client.h"
#include "keys.h"

#define MAX_HTTP_OUTPUT_BUFFER (4 * 1024)
const char *PATH = "/data/2.5/forecast?units=metric&cnt=4" //
                   "&lat=" LATITUDE                        //
                   "&lon=" LONGITUDE                       //
                   "&APPID=" OPEN_WEATHER_API_KEY          //
    ;

struct Buffer {
  char buffer[MAX_HTTP_OUTPUT_BUFFER + 1];
  int len;
};

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
  struct Buffer *buffer = evt->user_data;
  switch (evt->event_id) {
  case HTTP_EVENT_ON_DATA:
    if (buffer && buffer->len == 0) {
      memset(buffer->buffer, 0, MAX_HTTP_OUTPUT_BUFFER + 1);
    }
    if (esp_http_client_is_chunked_response(evt->client)) {
      break; // Not supported
    }
    // If user_data buffer is configured, copy the response into the buffer
    int copy_len = 0;
    if (buffer) {
      // The last byte in evt->user_data is kept for the NULL character in case
      // of out-of-bound access.
      copy_len = MAX_HTTP_OUTPUT_BUFFER - buffer->len;
      copy_len = evt->data_len < copy_len ? evt->data_len : copy_len;
      if (copy_len) {
        memcpy(&buffer->buffer[buffer->len], evt->data, copy_len);
      }
      buffer->len += copy_len;
    }
    break;
  default:
    break;
  }
  return ESP_OK;
}

bool http_response(struct Buffer *buffer) {
  if (!connect_wifi()) {
    return false;
  }

  esp_http_client_config_t config = {
      .host = "api.openweathermap.org",
      .path = PATH,
      .event_handler = _http_event_handler,
      .user_data = buffer,
      .disable_auto_redirect = true,
      .timeout_ms = 10000,
  };
  esp_http_client_handle_t client = esp_http_client_init(&config);

  esp_err_t err = esp_http_client_perform(client);
  if (err != ESP_OK) {
    printf("Connection error: %d\r\n", err);
    esp_http_client_cleanup(client);
    return false;
  }

  int status = esp_http_client_get_status_code(client);

  printf("HTTP CODE: %d\n", status);

  esp_http_client_cleanup(client);

  return true;
}

static enum WeatherType weather_id_to_type(int weather_id) {
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

bool download_weather_data(struct ArtData *data) {
  struct Buffer * buffer = malloc(sizeof(struct Buffer));
  buffer->len = 0;

  if (!http_response(buffer)) {
    free(buffer);
    return false;
  }

  cJSON *json = cJSON_ParseWithLength(buffer->buffer, buffer->len);
  free(buffer);

  if (!json) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr) {
      printf("Parsing weather data failed: %s\r\n", error_ptr);
    } else {
      printf("Parsing weather data failed (Unknown error)\r\n");
    }
    return false;
  }

  char found_rain = 0;
  char found_snow = 0;

  const cJSON *json_list = cJSON_GetObjectItem(json, "list");
  for (int i = 0; i < 4; i++) {
    const cJSON *json_item = cJSON_GetArrayItem(json_list, i);
    if (cJSON_IsInvalid(json_item)) {
      continue;
    }

    int weather_id = 0;
    time_t dt = 0;
    struct Forecast *forecast = &data->forecast[i];

    {
      const cJSON *json_weather_list =
          cJSON_GetObjectItem(json_item, "weather");
      const cJSON *json_weather_item = cJSON_GetArrayItem(json_weather_list, 0);
      const cJSON *json_weather_id =
          cJSON_GetObjectItem(json_weather_item, "id");
      weather_id = cJSON_GetNumberValue(json_weather_id);
      forecast->type = weather_id_to_type(weather_id);
    }

    {
      const cJSON *json_weather_main = cJSON_GetObjectItem(json_item, "main");
      const cJSON *json_weather_temp =
          cJSON_GetObjectItem(json_weather_main, "temp");
      forecast->temperature = cJSON_GetNumberValue(json_weather_temp);
    }

    {
      const cJSON *json_dt = cJSON_GetObjectItem(json_item, "dt");
      time_t dt = (time_t)cJSON_GetNumberValue(json_dt);
      struct tm timeinfo;
      localtime_r(&dt, &timeinfo);
      forecast->hour = timeinfo.tm_hour;
      forecast->minute = timeinfo.tm_min;
    }

    if (!found_rain) {
      const cJSON *json_rain = cJSON_GetObjectItem(json_item, "rain");
      const cJSON *json_rain_item;
      json_rain_item = cJSON_GetObjectItem(json_rain, "1h");
      if (json_rain_item) {
        found_rain = 1;
        double rain = cJSON_GetNumberValue(json_rain_item);
        data->rain_density = rain > 0 ? rain * 80.0 : 0;
      }
      json_rain_item = cJSON_GetObjectItem(json_rain, "3h");
      if (json_rain_item) {
        found_rain = 1;
        double rain = cJSON_GetNumberValue(json_rain_item);
        data->rain_density = rain > 0 ? rain / 3.0 * 80.0 : 0;
      }
    }

    if (!found_snow) {
      const cJSON *json_snow = cJSON_GetObjectItem(json_item, "snow");
      const cJSON *json_snow_item;
      json_snow_item = cJSON_GetObjectItem(json_snow, "1h");
      if (json_snow_item) {
        found_snow = 1;
        double snow = cJSON_GetNumberValue(json_snow_item);
        data->snow_density = snow > 0 ? snow * 8.0 : 0;
      }
      json_snow_item = cJSON_GetObjectItem(json_snow, "3h");
      if (json_snow_item) {
        found_snow = 1;
        double snow = cJSON_GetNumberValue(json_snow_item);
        data->snow_density = snow > 0 ? snow / 3.0 * 8.0 : 0;
      }
    }

    printf("Forecast %d id: %3d, timestamp: %lld time: %02d:%02d; temperature: "
           "%2d\n",
           i, weather_id, dt, forecast->hour, forecast->minute,
           forecast->temperature);
  }

  if (!found_rain) {
     data->rain_density = 0;
  }

  if (!found_snow) {
     data->snow_density = 0;
  }

  {
    const cJSON *json_item = cJSON_GetArrayItem(json_list, 0);
    {
      const cJSON *json_clouds = cJSON_GetObjectItem(json_item, "clouds");
      const cJSON *json_clouds_all = cJSON_GetObjectItem(json_clouds, "all");
      double clouds = cJSON_GetNumberValue(json_clouds_all);
      data->clouds_count = clouds > 0 ? clouds / 4.0 : 0;
    }
    printf("Rain density: %d; Snow density: %d; clouds: %d\n", //
           data->rain_density, data->snow_density, data->clouds_count);
  }

  cJSON_Delete(json);
  return true;
}
