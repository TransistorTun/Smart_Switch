#ifndef ILI9341_GFX_H
#define ILI9341_GFX_H

#include <stdarg.h>

#include "stm32f1xx_hal.h"
#include "fonts.h"

#define HORIZONTAL_IMAGE	0
#define VERTICAL_IMAGE		1

typedef struct _tft_print_pos_t
{
	const uint8_t* font;
	uint16_t X;
	uint16_t Y;
	uint16_t print_color;
	uint16_t bg_color;
}tft_print_pos_t;

void ILI9341_DrawHollowCircle(uint16_t X, uint16_t Y, uint16_t radius, uint16_t color);
void ILI9341_DrawFilledCircle(uint16_t X, uint16_t Y, uint16_t radius, uint16_t color);
void ILI9341_DrawHollowRectangleCoord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color);
void ILI9341_DrawFilledRectangleCoord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color);
void ILI9341_Draw_BitMap(uint16_t x, uint16_t y, uint8_t *bitmap, uint16_t width, uint16_t height, uint16_t color);

void ILI9341_ConfigPrint(	const uint8_t* _font,
							uint16_t _X, uint16_t _Y,
							uint16_t _print_color, uint16_t _bg_color);

void ILI9341_PrintChar(const char Char);
void ILI9341_PrintBuffer(const char* pc_string, uint16_t length);
uint8_t ILI9341_Print(const char* pc_string, ...);

#endif
