#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
#include <stdarg.h>

static void SetWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);

/* imprecise small delay */
__STATIC_INLINE void DelayUs(volatile uint32_t us)
{
	us *= (SystemCoreClock / 1000000);
	while (us--);
}

void ILI9341_DrawHollowCircle(uint16_t X, uint16_t Y, uint16_t radius, uint16_t color)
{
	int x = radius-1;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (radius << 1);

	while (x >= y)
	{
		ILI9341_DrawPixel(X + x, Y + y, color);
		ILI9341_DrawPixel(X + y, Y + x, color);
		ILI9341_DrawPixel(X - y, Y + x, color);
		ILI9341_DrawPixel(X - x, Y + y, color);
		ILI9341_DrawPixel(X - x, Y - y, color);
		ILI9341_DrawPixel(X - y, Y - x, color);
		ILI9341_DrawPixel(X + y, Y - x, color);
		ILI9341_DrawPixel(X + x, Y - y, color);

		if (err <= 0)
		{
			y++;
			err += dy;
			dy += 2;
		}

		if (err > 0)
		{
			x--;
			dx += 2;
			err += (-radius << 1) + dx;
		}
	}
}

void ILI9341_DrawFilledCircle(uint16_t X, uint16_t Y, uint16_t radius, uint16_t color)
{

	int x = radius;
	int y = 0;
	int xChange = 1 - (radius << 1);
	int yChange = 0;
	int radiusError = 0;

	while (x >= y)
	{
		for (int i = X - x; i <= X + x; i++)
		{
			ILI9341_DrawPixel(i, Y + y,color);
			ILI9341_DrawPixel(i, Y - y,color);
		}

		for (int i = X - y; i <= X + y; i++)
		{
			ILI9341_DrawPixel(i, Y + x,color);
			ILI9341_DrawPixel(i, Y - x,color);
		}

		y++;
		radiusError += yChange;
		yChange += 2;

		if (((radiusError << 1) + xChange) > 0)
		{
			x--;
			radiusError += xChange;
			xChange += 2;
		}
	}
}

void ILI9341_DrawHollowRectangleCoord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color)
{
	uint16_t xLen = 0;
	uint16_t yLen = 0;
	uint8_t negX = 0;
	uint8_t negY = 0;
	float negCalc = 0;

	negCalc = X1 - X0;
	if(negCalc < 0) negX = 1;
	negCalc = 0;

	negCalc = Y1 - Y0;
	if(negCalc < 0) negY = 1;

	//DRAW HORIZONTAL!
	if(!negX)
	{
		xLen = X1 - X0;
	}
	else
	{
		xLen = X0 - X1;
	}
	ILI9341_DrawHLine(X0, Y0, xLen, color);
	ILI9341_DrawHLine(X0, Y1, xLen, color);

	//DRAW VERTICAL!
	if(!negY)
	{
		yLen = Y1 - Y0;
	}
	else
	{
		yLen = Y0 - Y1;
	}

	ILI9341_DrawVLine(X0, Y0, yLen, color);
	ILI9341_DrawVLine(X1, Y0, yLen, color);

	if((xLen > 0)||(yLen > 0))
	{
		ILI9341_DrawPixel(X1, Y1, color);
	}
}

void ILI9341_DrawFilledRectangleCoord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color)
{
	uint16_t xLen = 0;
	uint16_t yLen = 0;
	uint8_t negX = 0;
	uint8_t negY = 0;
	int32_t negCalc = 0;
	uint16_t X0True = 0;
	uint16_t Y0True = 0;

	negCalc = X1 - X0;
	if(negCalc < 0) negX = 1;
	negCalc = 0;

	negCalc = Y1 - Y0;
	if(negCalc < 0) negY = 1;

	if(!negX)
	{
		xLen = X1 - X0;
		X0True = X0;
	}
	else
	{
		xLen = X0 - X1;
		X0True = X1;
	}

	if(!negY)
	{
		yLen = Y1 - Y0;
		Y0True = Y0;
	}
	else
	{
		yLen = Y0 - Y1;
		Y0True = Y1;
	}

	ILI9341_DrawRectangle(X0True, Y0True, xLen, yLen, color);
}

void ILI9341_Draw_BitMap(uint16_t x, uint16_t y, uint8_t *bitmap, uint16_t width, uint16_t height, uint16_t color)
{
	uint16_t byteWidth = (width + 7) / 8;
	uint8_t b = 0;
	for (int16_t j = 0; j < height; j++, y++)
	{
		for (int16_t i = 0; i < width; i++)
		{
			if (i & 7)
			{
				b <<= 1;
			}
			else
				b = bitmap[j * byteWidth + i/8];
			if (b & 0x80)
			{
                ILI9341_DrawPixel(x + i, y, color);
                delay_ms(1);
				//lcd_Draw_Pixel(tft, x + i, y, color);
			}
		}
		
	}	
}

static struct _print_config
{
	const uint8_t* font;
	uint16_t X;
	uint16_t Y;
    uint16_t X_index;
    uint16_t Y_index;
	uint16_t print_color;
	uint16_t bg_color;
}print_config;

static const char * const g_pc_Hex = "0123456789abcdef";

void ILI9341_ConfigPrint(	const uint8_t* _font,
							uint16_t _X, uint16_t _Y,
							uint16_t _print_color, uint16_t _bg_color)
{
	print_config.font = _font;
	print_config.X = _X;
	print_config.Y = _Y;
	print_config.print_color = _print_color;
	print_config.bg_color = _bg_color;
}

void ILI9341_PrintChar(const char Char)
{
    if (Char == '\n')
    {
        print_config.Y += 19;
        return;
    }

	if ((Char < 31) || (Char > 127))
        return;

	uint8_t font_offset, font_width, font_height, font_BPL;
	uint8_t *temp_char;

	font_offset = print_config.font[0];
	font_width = print_config.font[1];
	font_height = print_config.font[2];
	font_BPL = print_config.font[3];

    /* Current Character = Meta + (Character Index * Offset)
    *
    * temp_char:        Is a uint8_t type pointer
    *
    * Offset = 46:      46 bytes of data is needed to represent a character
    *
    * (Char - 0x20):    In ascii table, the readable letter start at index 32 (0x20)
    * for letter "space", so to get the character index, we take the ascii index
    * of that letter minus the number 32.
    * Example: Letter "!" has ascii index of 33, we (33 - 32) = 1, since the letter
    * "space" take up 46 index in our font array, we need to 1 * 46, so that the
    * first array index of the first byte for letter "!" is 46, while the array index
    * of the last byte of letter "space" is 45.
    * 
    * Meta = 4:         Inside the font array consist of 4 bytes for the meta of the
    * fonts such as: font_offset, font_width, font_height, font_BPL, so to get the
    * correct index of the font array, you need to offset by 4.
    */
	temp_char = (uint8_t*)&print_config.font[((Char - 0x20) * font_offset) + 4];

	/* Clear background first */
	ILI9341_DrawRectangle(  print_config.X, print_config.Y, 
                            font_width, font_height, 
                            print_config.bg_color);

	for (int height = 0; height < font_height; height++)
	{
		for (int width=0; width < font_width; width++)
		{
            /* (height & 0xF8) >> 3, increase one by 8-bits */
			uint8_t z =  temp_char[font_BPL * width + ((height & 0xF8) >> 3) + 1];

			uint8_t b = 1 << (height & 0x07);
			if (( z & b ) != 0x00)
			{
				ILI9341_DrawPixel(  print_config.X + width, 
                                    print_config.Y + height, 
                                    print_config.print_color);
			}
		}
	}
}

void ILI9341_PrintBuffer(const char* pc_string, uint16_t length)
{
    /* Width of character */
	uint8_t char_width;
    /* Offset of character */
	uint8_t font_offset = print_config.font[0];
    /* Width of font */
	uint8_t font_width = print_config.font[1];

	uint16_t print_index;

	for(print_index = 0; print_index < length; print_index++)
	{
		ILI9341_PrintChar(*pc_string);

		/* Check character width and calculate proper position */
		uint8_t *temp_char = (uint8_t*)&print_config.font[((*pc_string - 0x20) * font_offset) + 4];
		char_width = temp_char[0];

		if(char_width + 2 < font_width)
		{
			/* If character width is smaller than font width */
			print_config.X += (char_width + 2);
		}
		else
		{
			print_config.X += font_width;
		}

		pc_string++;
	}
}

uint8_t ILI9341_Print(const char* pc_string, ...)
{
	uint32_t ui32_index, ui32_value, ui32_pos, ui32_count, ui32_base, ui32_is_neg;
    char *pc_temp_string, pc_buffer[16], c_fill;

    va_list vaArgP;

    va_start(vaArgP, pc_string);

	//
    // Loop while there are more characters in the string.
    //
	while (*pc_string)
	{
		 //
        // Find the first non-% character, or the end of the string.
        //
        for(ui32_index = 0;
            (pc_string[ui32_index] != '%') && (pc_string[ui32_index] != '\0');
            ui32_index++)
        {
        }

		//
        // Write this portion of the string.
        //
        ILI9341_PrintBuffer(pc_string, ui32_index);

		//
        // Skip the portion of the string that was written.
        //
        pc_string += ui32_index;

        //
        // See if the next character is a %.
        //
        if(*pc_string == '%')
        {
            //
            // Skip the %.
            //
            pc_string++;

            //
            // Set the digit count to zero, and the fill character to space
            // (in other words, to the defaults).
            //
            ui32_count = 0;
            c_fill = ' ';

            //
            // It may be necessary to get back here to process more characters.
            // Goto's aren't pretty, but effective.  I feel extremely dirty for
            // using not one but two of the beasts.
            //

			again:

            //
            // Determine how to handle the next character.
            //
            switch(*pc_string++)
            {
                //
                // Handle the digit characters.
                //
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    //
                    // If this is a zero, and it is the first digit, then the
                    // fill character is a zero instead of a space.
                    //
                    if((pc_string[-1] == '0') && (ui32_count == 0))
                    {
                        c_fill = '0';
                    }

                    //
                    // Update the digit count.
                    //
                    ui32_count *= 10;
                    ui32_count += pc_string[-1] - '0';

                    //
                    // Get the next character.
                    //
                    goto again;
                }

                //
                // Handle the %c command.
                //
                case 'c':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ui32_value = va_arg(vaArgP, uint32_t);

                    //
                    // Print out the character.
                    //
                    ILI9341_PrintBuffer((char *)&ui32_value, 1);

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %d and %i commands.
                //
                case 'd':
                case 'i':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ui32_value = va_arg(vaArgP, uint32_t);

                    //
                    // Reset the buffer position.
                    //
                    ui32_pos = 0;

                    //
                    // If the value is negative, make it positive and indicate
                    // that a minus sign is needed.
                    //
                    if((int32_t)ui32_value < 0)
                    {
                        //
                        // Make the value positive.
                        //
                        ui32_value = -(int32_t)ui32_value;

                        //
                        // Indicate that the value is negative.
                        //
                        ui32_is_neg = 1;
                    }
                    else
                    {
                        //
                        // Indicate that the value is positive so that a minus
                        // sign isn't inserted.
                        //
                        ui32_is_neg = 0;
                    }

                    //
                    // Set the base to 10.
                    //
                    ui32_base = 10;

                    //
                    // Convert the value to ASCII.
                    //
                    goto convert;
                }

                //
                // Handle the %s command.
                //
                case 's':
                {
                    //
                    // Get the string pointer from the varargs.
                    //
                    pc_temp_string = va_arg(vaArgP, char *);

                    //
                    // Determine the length of the string.
                    //
                    for(ui32_index = 0; pc_temp_string[ui32_index] != '\0'; ui32_index++)
                    {
                    }

                    //
                    // Write the string.
                    //
                    ILI9341_PrintBuffer(pc_temp_string, ui32_index);

                    //
                    // Write any required padding spaces
                    //
                    if(ui32_count > ui32_index)
                    {
                        ui32_count -= ui32_index;
                        while(ui32_count--)
                        {
                            ILI9341_PrintBuffer(" ", 1);
                        }
                    }

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %u command.
                //
                case 'u':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ui32_value = va_arg(vaArgP, uint32_t);

                    //
                    // Reset the buffer position.
                    //
                    ui32_pos = 0;

                    //
                    // Set the base to 10.
                    //
                    ui32_base = 10;

                    //
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    //
                    ui32_is_neg = 0;

                    //
                    // Convert the value to ASCII.
                    //
                    goto convert;
                }

                //
                // Handle the %x and %X commands.  Note that they are treated
                // identically; in other words, %X will use lower case letters
                // for a-f instead of the upper case letters it should use.  We
                // also alias %p to %x.
                //
                case 'x':
                case 'X':
                case 'p':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ui32_value = va_arg(vaArgP, uint32_t);

                    //
                    // Reset the buffer position.
                    //
                    ui32_pos = 0;

                    //
                    // Set the base to 16.
                    //
                    ui32_base = 16;

                    //
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    //
                    ui32_is_neg = 0;

                    //
                    // Determine the number of digits in the string version of
                    // the value.
                    //
convert:
                    for(ui32_index = 1;
                        (((ui32_index * ui32_base) <= ui32_value) &&
                         (((ui32_index * ui32_base) / ui32_base) == ui32_index));
                        ui32_index *= ui32_base, ui32_count--)
                    {
                    }

                    //
                    // If the value is negative, reduce the count of padding
                    // characters needed.
                    //
                    if(ui32_is_neg)
                    {
                        ui32_count--;
                    }

                    //
                    // If the value is negative and the value is padded with
                    // zeros, then place the minus sign before the padding.
                    //
                    if(ui32_is_neg && (c_fill == '0'))
                    {
                        //
                        // Place the minus sign in the output buffer.
                        //
                        pc_buffer[ui32_pos++] = '-';

                        //
                        // The minus sign has been placed, so turn off the
                        // negative flag.
                        //
                        ui32_is_neg = 0;
                    }

                    //
                    // Provide additional padding at the beginning of the
                    // string conversion if needed.
                    //
                    if((ui32_count > 1) && (ui32_count < 16))
                    {
                        for(ui32_count--; ui32_count; ui32_count--)
                        {
                            pc_buffer[ui32_pos++] = c_fill;
                        }
                    }

                    //
                    // If the value is negative, then place the minus sign
                    // before the number.
                    //
                    if(ui32_is_neg)
                    {
                        //
                        // Place the minus sign in the output buffer.
                        //
                        pc_buffer[ui32_pos++] = '-';
                    }

                    //
                    // Convert the value into a string.
                    //
                    for(; ui32_index; ui32_index /= ui32_base)
                    {
                        pc_buffer[ui32_pos++] =
                            g_pc_Hex[(ui32_value / ui32_index) % ui32_base];
                    }

                    //
                    // Write the string.
                    //
                    ILI9341_PrintBuffer(pc_buffer, ui32_pos);

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %% command.
                //
                case '%':
                {
                    //
                    // Simply write a single %.
                    //
                    ILI9341_PrintBuffer(pc_string - 1, 1);

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle all other commands.
                //
                default:
                {
                    //
                    // Indicate an error.
                    //
                    ILI9341_PrintBuffer("ERROR", 5);

                    //
                    // This command has been handled.
                    //
                    break;
                }
            }
        }
	}

    print_config.X = 0;
	va_end(vaArgP);
    return 1;
}
