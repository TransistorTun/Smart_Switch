#ifndef DHT_11_DHT_11_H_
#define DHT_11_DHT_11_H_

#include <stdint.h>

#include "stm32f1xx_hal.h"
#include "delay.h"

/*
#define DATA_GPIO	GPIOB
#define DATA_PIN	GPIO_PIN_4
*/

//#define DHT_PORT    GPIOB
//#define DHT_PIN     GPIO_PIN_12

#define DHT_ERROR_RESPONSE  0
#define DHT_ERROR_40_BITS   1
#define DHT_ERROR_CHECKSUM  2
#define DHT_OK              3

typedef struct _dht11_device_t
{
	GPIO_TypeDef* port;
	uint16_t pin;
	TIM_HandleTypeDef* delay_handle_us;
}dht11_device_t;

void DHT_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TIM_HandleTypeDef* timer_handle_us);
uint8_t dht_get_data(uint8_t* data);

#endif /* DHT_11_DHT_11_H_ */
