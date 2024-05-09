#include "pir_am312.h"

static struct _pir_am312_device
{
	GPIO_TypeDef* port;
	uint16_t pin;
}pir_am312_device;

void PIR_AM312_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    pir_am312_device.port = GPIOx;
    pir_am312_device.pin = GPIO_Pin;
}

uint8_t pir_am312_get_data(void)
{
    // Đọc giá trị từ chân kết nối cảm biến
    return HAL_GPIO_ReadPin(pir_am312_device.port, pir_am312_device.pin);
}
