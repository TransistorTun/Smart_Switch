#ifndef PIR_AM312_H
#define PIR_AM312_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

typedef struct _pir_am312_device_t
{
	GPIO_TypeDef* port;
	uint16_t pin;
}pir_am312_device_t;

void PIR_AM312_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t pir_am312_get_data(void);

#endif /*PIR_AM312_H*/
