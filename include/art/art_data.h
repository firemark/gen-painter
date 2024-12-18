#pragma once
#include <stdint.h>

#define FORECAST_SIZE 4

enum WeatherType {
  WEATHER_THUNDERSTORM = 0,
  WEATHER_DRIZZLE,
  WEATHER_RAIN,
  WEATHER_SHOWER_RAIN,
  WEATHER_SNOW,
  WEATHER_SHOWER_SNOW,
  WEATHER_FOG,
  WEATHER_CLEAR,
  WEATHER_FEW_CLOUDS,
  WEATHER_MANY_CLOUDS,
  WEATHER_WTF,
};

struct Forecast {
  enum WeatherType type;
  uint8_t hour;
  uint8_t minute;
  int16_t temperature;
};

struct ArtData {
  struct Forecast forecast[FORECAST_SIZE];

  uint16_t minute;
  uint16_t rain_density;
  uint16_t snow_density;
  uint8_t clouds_count;
};