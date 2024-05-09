#ifndef UARTSTDIO_H
#define UARTSTDIO_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include "stm32f1xx_hal.h"

#include "app.h"

extern volatile uint16_t UART_RX_frame_count;

void UART_Config(UART_HandleTypeDef* p_UART_handle, uint16_t _TX_buffer_size ,uint16_t _RX_buffer_size, uint8_t _disable_echo);
uint16_t UART_write(const char *pcBuf, uint16_t ui16Len);
void UART_printf(const char *pc_string, ...);

/*
uint16_t UART_RX_bytes_avail(void);
uint16_t UART_get_string(char *pcBuf, uint16_t ui16Len);
uint16_t UART_read_till_empty(char* pcBuf, uint16_t _RX_count);
uint16_t UART_frame_avail();
void UART_frame_process(char* number_array, uint16_t frame_to_count);
*/
//void UART_frame_process(char* command_buffer);

#endif // UARTSTDIO_H