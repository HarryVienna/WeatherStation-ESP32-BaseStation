FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/*.*)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "."
                    REQUIRES lvgl esp_lcd esp_lcd_touch esp_lcd_touch_gt911 bh1750 json sen0610 sensirion_sensor
                             esp_wifi nvs_flash esp_event esp_netif esp_http_client esp-tls)
