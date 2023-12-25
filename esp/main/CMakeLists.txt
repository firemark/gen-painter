file(GLOB_RECURSE SRC_FILES "src/**/*.c")

idf_component_register(
    SRCS src/esp32_main.c ${SRC_FILES}
    INCLUDE_DIRS include
    REQUIRES art esp_driver_gpio esp_wifi json esp_http_client esp_event nvs_flash
)
