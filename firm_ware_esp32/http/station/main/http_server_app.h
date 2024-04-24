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
#include "gpio_driver.h"
#include "relay_control_driver.h"
#include "user_interface_driver.h"

void http_start_webserver(void);
void http_stop_webserver(void);

#endif