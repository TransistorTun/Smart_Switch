#include "dht_11.h"

static struct _dht_device
{
	GPIO_TypeDef* port;
	uint16_t pin;
	TIM_HandleTypeDef* delay_handle_us;
}dht_device;

static GPIO_InitTypeDef GPIO_output =
{
    .Pin	= 0,
    .Mode	= GPIO_MODE_OUTPUT_OD,
    .Pull	= GPIO_PULLUP,
    .Speed	= GPIO_SPEED_FREQ_LOW,
};

static GPIO_InitTypeDef GPIO_input = 
{
    .Pin	= 0,
    .Mode	= GPIO_MODE_INPUT,
    .Pull	= GPIO_PULLUP,
};


static inline uint8_t capture_bit_0_1_for(uint16_t bit_0_time, uint16_t bit_1_time, uint8_t return_data);

static inline uint8_t capture_bit_0_1_for(uint16_t bit_0_time_us, uint16_t bit_1_time_us, uint8_t return_data)
{
	dht_device.delay_handle_us->Instance->CNT = 0;
	__HAL_TIM_ENABLE(dht_device.delay_handle_us);
	while ((dht_device.port->IDR & dht_device.pin) == 0)
	{
		if (dht_device.delay_handle_us->Instance->CNT > bit_0_time_us)
		{
			__HAL_TIM_DISABLE(dht_device.delay_handle_us);
			HAL_GPIO_Init(dht_device.port, &GPIO_output);
			HAL_GPIO_WritePin(dht_device.port, dht_device.pin, GPIO_PIN_SET);
			return return_data;
		}
	}

	while ((dht_device.port->IDR & dht_device.pin) != 0)
	{
		if (dht_device.delay_handle_us->Instance->CNT > bit_1_time_us)
		{
			__HAL_TIM_DISABLE(dht_device.delay_handle_us);
			HAL_GPIO_Init(dht_device.port, &GPIO_output);
			HAL_GPIO_WritePin(dht_device.port, dht_device.pin, GPIO_PIN_SET);
			return return_data;	
		}	
	}
	__HAL_TIM_DISABLE(dht_device.delay_handle_us);
}

void DHT_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TIM_HandleTypeDef* timer_handle_us)
{
	dht_device.port 			= GPIOx;
	dht_device.pin 				= GPIO_Pin;
	dht_device.delay_handle_us 	= timer_handle_us;

	GPIO_output.Pin = GPIO_Pin;
	GPIO_input.Pin = GPIO_Pin;
}

uint8_t dht_get_data(uint8_t* data)
{
	uint8_t checksum = 0;
	uint8_t i = 0;
	int bit_pos = 0;
	
	//MCU send request to DHT
	HAL_GPIO_Init(dht_device.port, &GPIO_output);
	HAL_GPIO_WritePin(dht_device.port, dht_device.pin, GPIO_PIN_RESET);
	delay_ms(20);
	HAL_GPIO_WritePin(dht_device.port, dht_device.pin, GPIO_PIN_SET);
	delay_us(30);
	HAL_GPIO_Init(dht_device.port, &GPIO_input);

	//DHT send respond to MCU
	capture_bit_0_1_for(90, 170, DHT_ERROR_RESPONSE);

	//DHT send 40 bits to MCU
	for (i = 0; i<= 4; i++)
	{
		for (bit_pos = 7; bit_pos >= 0; bit_pos--)
		{
			capture_bit_0_1_for(50, 250, DHT_ERROR_40_BITS);

			if (dht_device.delay_handle_us->Instance->CNT < 90)
				data[i] &= ~(1 << bit_pos);
			else
				data[i] |= 1 << bit_pos;
		}
	}

	for (i = 0; i <= 3; i++)
	{
		checksum += data[i];
	}

	if (checksum != data[4])
	{
		HAL_GPIO_Init(dht_device.port, &GPIO_output);
		HAL_GPIO_WritePin(dht_device.port, dht_device.pin, GPIO_PIN_SET);
		data[4] = DHT_ERROR_CHECKSUM;
	}

	if(data[3] > 9)
		data[3] /= 10;
	
	HAL_GPIO_Init(dht_device.port, &GPIO_output);
	HAL_GPIO_WritePin(dht_device.port, dht_device.pin, GPIO_PIN_SET);
	data[4] = DHT_OK;
}
