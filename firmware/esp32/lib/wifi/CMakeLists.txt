idf_component_register(SRCS "wifi.c" 
                    REQUIRES nvs_flash esp_wifi esp_eth esp_wifi esp_http_server
                    INCLUDE_DIRS ".")