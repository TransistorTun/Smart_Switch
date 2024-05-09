#ifndef DELAY_H_
#define DELAY_H_

#include <stdint.h>

#include "stm32f1xx_hal.h"
#include "core_cm3.h"

/*
* UNCOMMENT THE DEFINE YOU WANT TO USE
*/
#define DELAY_USE_TIMER     
//#define DELAY_USE_SYSTICK

#ifdef DELAY_USE_TIMER
    extern TIM_HandleTypeDef* delay_timebase_ms;
    extern TIM_HandleTypeDef* delay_timebase_us;
#endif

void Delay_Init(TIM_HandleTypeDef* delay_us_handle, TIM_HandleTypeDef* delay_ms_handle);
void delay_us(uint32_t delay_time_us);
void delay_ms(uint32_t delay_time_ms);
void util_delay_handle(void);

#endif /* DELAY_H_ */
