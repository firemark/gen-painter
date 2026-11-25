#include "esp32/time.h"

#include "esp32/wifi.h"

#include "esp_sntp.h"

#include "keys.h"

#include <math.h>

bool sync_time(void) {
  sntp_sync_status_t sntp_sync = sntp_get_sync_status();
  if (sntp_sync == SNTP_SYNC_STATUS_COMPLETED) {
    return true;
  }

  if (!connect_wifi()) {
    return false;
  }

  printf("Synchronize time\r\n");

  esp_sntp_init();
  // 24 hours, I don't need smooth synchronization because
  // I need only is to get current hour.
  esp_sntp_set_sync_interval(86400000ULL);
  esp_sntp_setservername(0, "pool.ntp.org");
  esp_sntp_restart();

  uint8_t count = 0;
  do {
    vTaskDelay(100 / portTICK_PERIOD_MS);
    sntp_sync = esp_sntp_get_sync_status();
    if (count++ > 250) {
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

static float _fmod(float x, float y) {
  x = fmod(x, y);
  if (x < 0) {
    x += y;
  }
  if (x >= y || x == 0.0) {
    x = 0;
  }
  return x;
}

static float _time(float N, float lngHour, bool rising) {
  float latitude = atof(LATITUDE);
  float zenith = 108;

  float t;
  if (rising) {
    t = N + ((6 - lngHour) / 24);
  } else {
    t = N + ((18 - lngHour) / 24);
  }

  float M = (0.9856 * t) - 3.289;
  float L = M + (1.916 * sin(M)) + (0.020 * sin(2 * M)) + 282.634;
  L = _fmod(L, 360);
  float RA = atan(0.91764 * tan(L));
  float Lquadrant = (floor(L / 90)) * 90;
  float RAquadrant = (floor(RA / 90)) * 90;
  RA = _fmod(RA, 360);
  RA = RA + (Lquadrant - RAquadrant);
  RA = RA / 15;
  float sinDec = 0.39782 * sin(L);
  float cosDec = cos(asin(sinDec));
  float cosH =
      (cos(zenith) - (sinDec * sin(latitude))) / (cosDec * cos(latitude));

  if (cosH < -1 || cosH > 1) {
    return 0;
  }

  float H;
  if (rising) {
    H = 360 - acos(cosH);
  } else {
    H = acos(cosH);
  }
  H = H / 15;

  float T = H + RA - (0.06571 * t) - 6.622;
  float UT = _fmod(T - lngHour, 24);
  // float localT = _fmod(UT + 2, 24);
  return UT;
}

static uint16_t compute_real_time(struct tm* timeinfo) {
  uint16_t N = timeinfo->tm_yday;
  float longitude = atof(LONGITUDE);
  float lngHour = longitude / 15;

  float t = (float)timeinfo->tm_hour + (float)timeinfo->tm_min / 60;
  float t_rising = _time(N, lngHour, true);
  float t_setting = _time(N, lngHour, false);

  printf("UTC    : %02.2f\r\n", t);
  printf("UTC ris: %02.2f\r\n", t_rising);
  printf("UTC set: %02.2f\r\n", t_setting);

  // 0 to 360 -> nothing
  // 360 to 1080 -> sun
  // 1080 to 1440 -> moon

  if (t < t_rising) {
    float tn = t / 12.0 + 0.5;
    float diff = t_rising;
    return (18 + diff * tn) * 60;
  } else if (t < t_setting) {
    float tn = (t - 6) / 12.0;
    float diff = t_setting - t_rising;
    return (6 + diff * tn) * 60;
  } else {
    float tn = (t - 18) / 12.0;
    float diff = 24 - t_setting;
    return (18 + diff * tn) * 60;
  }
}

bool check_time(uint16_t* minute) {
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

  gmtime_r(&now, &timeinfo);
  *minute = compute_real_time(&timeinfo);
  printf("Local time: %d\r\n", *minute);
  return true;
}
