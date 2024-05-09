#ifndef APP_H_
#define APP_H_

#include <stdint.h>

#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_usart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
#include "dht_11.h"
#include "pir_am312.h"
#include "lm_393.h"

#include "delay.h"
//#include "uart_stdio.h"
#include "uartstdio.h"

extern xQueueHandle queue_relay_light_data;
extern xQueueHandle queue_UART_RX_data;
extern TaskHandle_t Task_Relay_Light_Handle;

typedef enum _relay_light_state_t
{
    RELAY_LED_OFF,
    RELAY_LED_ON_TIMER,
    RELAY_LED_ON_BUTTON,
    RELAY_LED_AUTO,
    RELAY_LED_HAND,
}relay_light_state_t;

typedef enum _relay_light_data_t
{
    BUTTON_1_PRESSED,
    BUTTON_2_PRESSED,
    WEB_BUTTON_1_PRESSED,
    WEB_BUTTON_2_PRESSED,
    RELAY_LIGHT_BUTTON_5_AUTO,
    RELAY_LIGHT_BUTTON_5_HAND,
    PIR_ON,
    PIR_OFF,
    LM393_ON,
    LM393_OFF,
    TIMER_GO_OFF,
    RELAY_LIGHT_DATA_NULL,
}relay_light_data_t;

typedef enum _UART_TX_state_t
{
    UART_TX_RELAY_1_ON,
    UART_TX_RELAY_1_OFF,
    UART_TX_RELAY_2_ON,
    UART_TX_RELAY_2_OFF,
    UART_TX_RELAY_3_ON,
    UART_TX_RELAY_3_OFF,
    UART_TX_RELAY_4_ON,
    UART_TX_RELAY_4_OFF,
    UART_TX_RELAY_5_ON,
    UART_TX_RELAY_5_OFF,
    UART_TX_DHT,
    UART_TX_PIR_ON,
    UART_TX_PIR_OFF,
    UART_TX_LM_ON,
    UART_TX_LM_OFF,
    UART_TX_AUTO_OFF,
    UART_TX_IDLE,
}UART_TX_state_t;

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

typedef enum _TFT_state_t
{
    TFT_IDLE,
    TFT_DHT,
    TFT_PIR_ON,
    TFT_PIR_OFF,
    TFT_LM393_ON,
    TFT_LM393_OFF,
}TFT_state_t;

typedef enum _relay_humid_fan_state_t
{
    RELAY_HUMID_ON_BUTTON,
    RELAY_HUMID_ON_COMPARE,
    RELAY_HUMID_OFF_AUTO,
    RELAY_HUMID_OFF_HAND,
    RELAY_FAN_ON_BUTTON,
    RELAY_FAN_ON_COMPARE,
    RELAY_FAN_OFF_AUTO,
    RELAY_FAN_OFF_HAND,
}relay_humid_fan_state_t;

typedef enum _relay_humid_fan_data_t
{
    BUTTON_3_PRESSED,
    BUTTON_4_PRESSED,
    WEB_BUTTON_3_PRESSED,
    WEB_BUTTON_4_PRESSED,
    RELAY_HUMID_FAN_5_AUTO,
    RELAY_HUMID_FAN_5_HAND,
    RELAY_HUMID_FAN_BUTTON_NULL,
} relay_humid_fan_data_t;

typedef enum _relay_status_data_t
{
    RELAY_1_ON,
    RELAY_1_OFF,
    RELAY_2_ON,
    RELAY_2_OFF,
    RELAY_3_ON,
    RELAY_3_OFF,
    RELAY_4_ON,
    RELAY_4_OFF,
}relay_status_data_t;

#define BUTTON_1_PIN    GPIO_PIN_3
#define BUTTON_2_PIN    GPIO_PIN_2
#define BUTTON_3_PIN    GPIO_PIN_1
#define BUTTON_4_PIN    GPIO_PIN_0

#define RELAY_1_PORT    GPIOA
#define RELAY_1_PIN     GPIO_PIN_4
#define RELAY_2_PORT    GPIOA
#define RELAY_2_PIN     GPIO_PIN_5
#define RELAY_3_PORT    GPIOA
#define RELAY_3_PIN     GPIO_PIN_6
#define RELAY_4_PORT    GPIOA
#define RELAY_4_PIN     GPIO_PIN_7

void App_Main(void);

#endif /* APP_H_ */
