#pragma once
#include <stdint.h>

enum WeatherType {
    THUNDERSTORM,
    DRIZZLE,
    RAIN,
    SHOWER_RAIN,
    SNOW,
    SHOWER_SNOW,
    FOG,
    CLEAR,
    CLOUDS,
    WTF,
};

struct Forecast {
    enum WeatherType type;
    uint8_t hour;
    uint8_t minute;
    int16_t temperature;
} forecast[4];

struct ArtData {
    struct Forecast forecast[4];
    
    uint16_t minute;
    uint16_t rain_density;
    uint16_t snow_density;
    uint8_t clouds_count;
};