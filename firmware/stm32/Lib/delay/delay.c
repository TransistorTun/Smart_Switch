#include "delay.h"

#ifdef DELAY_USE_SYSTICK
    volatile uint8_t delay_update_flag = 0;
#endif

#ifdef DELAY_USE_TIMER
    TIM_HandleTypeDef* delay_timer_us_handle;
    TIM_HandleTypeDef* delay_timer_ms_handle;

    void Delay_Init(TIM_HandleTypeDef* us_handle, TIM_HandleTypeDef* ms_handle)
    {
        delay_timer_us_handle = us_handle;
        delay_timer_ms_handle = ms_handle;
    }
#endif

void delay_us(uint32_t delay_time_us)
{
    // Util use SysTick
    #ifdef DELAY_USE_SYSTICK
        delay_update_flag = 0;
        /*
        * Do ham HAL_SYSTICK_config luon mac dinh lay clock la AHB khong chia cho 8
        * nen, T = 1 / 8*10^6 = 1.25*10^-7, de co tick cho 1us thi SysTick_load = 1*10^-6/1.25*10-7 = 8
        */
        uint32_t systick_load = delay_time_us * 8;
        HAL_SYSTICK_Config(systick_load);
        while (delay_update_flag == 0);
    #endif

    #ifdef DELAY_USE_TIMER
        delay_timer_us_handle->Instance->CNT = 0;
        __HAL_TIM_ENABLE(delay_timer_us_handle);
        while (delay_timer_us_handle->Instance->CNT < delay_time_us);
        __HAL_TIM_DISABLE(delay_timer_us_handle);

    #endif
}

void delay_ms(uint32_t delay_time_ms)
{   
    // Util use SysTick
    #ifdef DELAY_USE_SYSTICK
        delay_update_flag = 0;
        /*
        * Do ham HAL_SYSTICK_config luon mac dinh lay clock la AHB khong chia cho 8
        * nen, T = 1 / 8*10^6 = 1.25*10^-7, de co tick cho 1ms thi SysTick_load = 1*10^-3/1.25*10-7 = 8000
        */
        uint32_t systick_load = delay_time_ms * 8000;
        HAL_SYSTICK_Config(systick_load);
        while (delay_update_flag == 0);
    #endif

    #ifdef DELAY_USE_TIMER
        delay_timer_ms_handle->Instance->CNT = 0;
        __HAL_TIM_ENABLE(delay_timer_ms_handle);
        while (delay_timer_ms_handle->Instance->CNT < delay_time_ms);
        __HAL_TIM_DISABLE(delay_timer_ms_handle);

    #endif
}

#ifdef DELAY_USE_SYSTICK
void util_delay_handle(void)
{
    delay_update_flag = 1;
    SysTick->CTRL &= !SysTick_CTRL_ENABLE_Msk;
}
#endif