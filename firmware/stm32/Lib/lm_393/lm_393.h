#ifndef LM_393_H
#define LM_393_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

typedef struct _lm393_device_t
{
	GPIO_TypeDef* port;
	uint16_t pin;
}lm393_device_t;

void LM393_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t lm393_get_data(void);

#endif /* LM393_H */
