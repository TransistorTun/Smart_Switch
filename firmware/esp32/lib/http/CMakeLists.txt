idf_component_register(SRCS "http.c"
                    REQUIRES nvs_flash esp_wifi esp_eth esp_wifi esp_http_server driver "uart_driver"
                    INCLUDE_DIRS ".")