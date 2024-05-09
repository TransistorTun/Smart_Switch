#include "lm_393.h"

static struct _lm393_device
{
	GPIO_TypeDef* port;
	uint16_t pin;
}lm393_device;

void LM393_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    lm393_device.port = GPIOx;
    lm393_device.pin = GPIO_Pin;
}

uint8_t lm393_get_data(void)
{
    // Đọc giá trị từ chân kết nối cảm biến
    if (HAL_GPIO_ReadPin(lm393_device.port, lm393_device.pin) == 0)
        return 1;
    else
        return 0;
}