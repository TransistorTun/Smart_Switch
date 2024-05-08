#ifndef _UART_DRIVER_H
#define _UART_DRIVER_H

#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include "esp_system.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_15)


#define UART_NUM UART_NUM_1

typedef enum _UART_frame_t
{
    UART_BUTTON_1,
    UART_BUTTON_2,
    UART_BUTTON_3,
    UART_BUTTON_4,
    UART_BUTTON_5_AUTO,
    UART_BUTTON_5_HAND,
    UART_DHT_HUMID,
    UART_DHT_TEMP,
    UART_PIR,
    UART_LM393,
    UART_STOP_FRAME = 10,
}UART_frame_t;

void uart_init(void);
void uart_tx_task(const char* data);
int sendData(const char* logName, const char* data);

#endif
