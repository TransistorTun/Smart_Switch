idf_component_register(SRCS "http_server_app.c"
                    REQUIRES nvs_flash esp_wifi esp_eth esp_http_server 
                    INCLUDE_DIRS ".")