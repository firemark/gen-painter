#include "esp32/time.h"

#include "esp32/wifi.h"

#include "esp_sntp.h"

#include "keys.h"

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

bool check_time(uint16_t *minute) {
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

  *minute = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  return true;
}