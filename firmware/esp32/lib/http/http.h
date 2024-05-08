#ifndef _HTTP_SERVER_APP_H
#define _HTTP_SERVER_APP_H

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
// #include "esp_tls_crypto.h"
#include "esp_http_server.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef void (*http_post_callback_t) (uint8_t* data, uint16_t len);
typedef void (*http_get_callback_t) (void);

void http_start_webserver(void);
void http_stop_webserver(void);
void http_set_callback_switch(void *callback);
void http_set_callback_dht11(void *callback);

void dht11_response(char *data, int len);

#endif