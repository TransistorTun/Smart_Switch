/*********************
 *      INCLUDES
 *********************/

#include "uartstdio.h"

extern TaskHandle_t Task_UART_RX_Handle;

//*****************************************************************************
//
// This global controls whether or not we are echoing characters back to the
// transmitter.  By default, echo is enabled but if using this module as a
// convenient method of implementing a buffered serial interface over which
// you will be running an application protocol, you are likely to want to
// disable echo by calling UARTEchoSet(0).
//
//*****************************************************************************
static uint8_t disable_echo;
static struct _uartstdio_device
{
    UART_HandleTypeDef* UART_handle;
    IRQn_Type		    UART_irqn;
    uint16_t            TX_buffer_size;
    uint16_t            RX_buffer_size;
    uint8_t			    disable_echo;
}uartstdio_device;

#define UART_TX_BUFFER_SIZE uartstdio_device.TX_buffer_size
#define UART_RX_BUFFER_SIZE uartstdio_device.RX_buffer_size

//*****************************************************************************
//
// A mapping from an integer between 0 and 15 to its ASCII character
// equivalent.
//
//*****************************************************************************
static const char * const HEX_reference = "0123456789abcdef";

//*****************************************************************************
//
// Output ring buffer.  Buffer is full if UART_TX_read_index is one ahead of
// UART_TX_write_index.  Buffer is empty if the two indices are the same.
//
//*****************************************************************************
static uint8_t* UART_TX_buffer;
static volatile uint16_t UART_TX_write_index = 0;
static volatile uint16_t UART_TX_read_index = 0;

//*****************************************************************************
//
// Input ring buffer.  Buffer is full if UART_TX_read_index is one ahead of
// UART_TX_write_index.  Buffer is empty if the two indices are the same.
//
//*****************************************************************************
static  uint8_t* UART_RX_buffer;
static  volatile uint16_t UART_RX_write_index = 0;
static  volatile uint16_t UART_RX_read_index = 0;
static  volatile char     RX_irq_char;
        volatile uint16_t UART_RX_frame_count = 0;

/******************************
 *  STATIC PROTOTYPE FUNCTION
 ******************************/
static uint8_t is_buffer_full(volatile uint16_t *pui16Read,
             volatile uint16_t *pui16Write, uint16_t ui16Size);

static uint8_t is_buffer_empty(volatile uint16_t *pui16Read,
              volatile uint16_t *pui16Write);

static uint16_t get_buffer_count(volatile uint16_t *pui16Read,
               volatile uint16_t *pui16Write, uint16_t ui16Size);

static void UART_prime_transmit();

//*****************************************************************************
//
// Macros to determine number of free and used bytes in the transmit buffer.
//
//*****************************************************************************
#define TX_BUFFER_USED          (get_buffer_count(&UART_TX_read_index,  \
                                                &UART_TX_write_index, \
                                                UART_TX_BUFFER_SIZE))

#define TX_BUFFER_FREE          (UART_TX_BUFFER_SIZE - TX_BUFFER_USED)

#define TX_BUFFER_EMPTY         (is_buffer_empty(&UART_TX_read_index,   \
                                               &UART_TX_write_index))

#define TX_BUFFER_FULL          (is_buffer_full(&UART_TX_read_index,  \
                                              &UART_TX_write_index, \
                                              UART_TX_BUFFER_SIZE))

#define ADVANCE_TX_BUFFER_INDEX(Index) \
                                (Index) = ((Index) + 1) % UART_TX_BUFFER_SIZE

//*****************************************************************************
//
// Macros to determine number of free and used bytes in the receive buffer.
//
//*****************************************************************************
#define RX_BUFFER_USED          (get_buffer_count(&UART_RX_read_index,  \
                                                &UART_RX_write_index, \
                                                UART_RX_BUFFER_SIZE))

#define RX_BUFFER_FREE          (UART_RX_BUFFER_SIZE - RX_BUFFER_USED)

#define RX_BUFFER_EMPTY         (is_buffer_empty(&UART_RX_read_index,   \
                                               &UART_RX_write_index))

#define RX_BUFFER_FULL          (is_buffer_full(&UART_RX_read_index,  \
                                              &UART_RX_write_index, \
                                              UART_RX_BUFFER_SIZE))

#define ADVANCE_RX_BUFFER_INDEX(Index) \
                                (Index) = ((Index) + 1) % UART_RX_BUFFER_SIZE

/*********************
 * STATIC FUNCTION
 *********************/

//*****************************************************************************
//
//! Determines whether the ring buffer whose pointers and size are provided
//! is full or not.
//!
//! \param pui16Read points to the read index for the buffer.
//! \param pui16Write points to the write index for the buffer.
//! \param ui16Size is the size of the buffer in bytes.
//!
//! This function is used to determine whether or not a given ring buffer is
//! full.  The structure of the code is specifically to ensure that we do not
//! see warnings from the compiler related to the order of volatile accesses
//! being undefined.
//!
//! \return Returns \b 1 if the buffer is full or \b 0 otherwise.
//
//*****************************************************************************

static uint8_t is_buffer_full(volatile uint16_t *pui16Read,
             volatile uint16_t *pui16Write, uint16_t ui16Size)
{
    uint16_t ui16Write;
    uint16_t ui16Read;

    ui16Write = *pui16Write;
    ui16Read = *pui16Read;

    //TODO: Sua lai khuc nay
    return((((ui16Write + 1) % ui16Size) == ui16Read) ? 1 : 0);
    //return((ui16Write == (ui16Size - 1)) ? 1 : 0);
}


//*****************************************************************************
//
//! Determines whether the ring buffer whose pointers and size are provided
//! is empty or not.
//!
//! \param pui16Read points to the read index for the buffer.
//! \param pui16Write points to the write index for the buffer.
//!
//! This function is used to determine whether or not a given ring buffer is
//! empty.  The structure of the code is specifically to ensure that we do not
//! see warnings from the compiler related to the order of volatile accesses
//! being undefined.
//!
//! \return Returns \b 1 if the buffer is empty or \b 0 otherwise.
//
//*****************************************************************************

static uint8_t is_buffer_empty(volatile uint16_t *pui16Read,
              volatile uint16_t *pui16Write)
{
    uint16_t ui16Write;
    uint16_t ui16Read;

    ui16Write = *pui16Write;
    ui16Read = *pui16Read;

    return((ui16Write == ui16Read) ? 1 : 0);
}


//*****************************************************************************
//
//! Determines the number of bytes of data contained in a ring buffer.
//!
//! \param pui16Read points to the read index for the buffer.
//! \param pui16Write points to the write index for the buffer.
//! \param ui16Size is the size of the buffer in bytes.
//!
//! This function is used to determine how many bytes of data a given ring
//! buffer currently contains.  The structure of the code is specifically to
//! ensure that we do not see warnings from the compiler related to the order
//! of volatile accesses being undefined.
//!
//! \return Returns the number of bytes of data currently in the buffer.
//
//*****************************************************************************

static uint16_t get_buffer_count(volatile uint16_t *pui16Read,
               volatile uint16_t *pui16Write, uint16_t ui16Size)
{
    uint16_t ui16Write;
    uint16_t ui16Read;

    ui16Write = *pui16Write;
    ui16Read = *pui16Read;

    return((ui16Write >= ui16Read) ? (ui16Write - ui16Read) :
           (ui16Size - (ui16Read - ui16Write)));
}

//*****************************************************************************
//
// Take as many bytes from the transmit buffer as we have space for and move
// them into the UART transmit FIFO.
//
//*****************************************************************************

static void UART_prime_transmit()
{
    HAL_StatusTypeDef UART_ret;
    //
    // Do we have any data to transmit?
    //
    if(!TX_BUFFER_EMPTY)
    //while(!TX_BUFFER_EMPTY)
    {
        //
        // Disable the UART interrupt.  If we don't do this there is a race
        // condition which can cause the read index to be corrupted.
        //
    	//NVIC_DisableIRQ(uartstdio_device.uart_irqn);


        //
        // Yes - take some characters out of the transmit buffer and feed
        // them to the UART transmit FIFO.
        //


        //LL_USART_TransmitData8(uartstdio_device.uart, UART_TX_buffer[UART_TX_read_index]);
        //HAL_UART_Transmit_IT(uartstdio_device.UART_handle, &UART_TX_buffer[UART_TX_read_index], 1);
        //while (!is_finished_sending);
    
        //is_finished_sending = 0;
        //HAL_UART_Transmit_DMA(uartstdio_device.UART_handle, &UART_TX_buffer[UART_TX_read_index], TX_count);
    	UART_ret = HAL_UART_Transmit_IT(uartstdio_device.UART_handle, &UART_TX_buffer[UART_TX_read_index], 1);
        if(UART_ret == HAL_OK)
            ADVANCE_TX_BUFFER_INDEX(UART_TX_read_index);
        //UART_TX_read_index = UART_TX_write_index;
        //ADVANCE_TX_BUFFER_INDEX(UART_TX_read_index);
        //UART_TX_read_index = UART_TX_write_index;


        //
        // Reenable the UART interrupt.
        //
        //NVIC_EnableIRQ(uartstdio_device.uart_irqn);
    }
}

/*********************
 * INTERRUPT FUNCTION
 *********************/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == uartstdio_device.UART_handle->Instance)
    {
         if (((READ_REG(huart->Instance->SR) & USART_SR_TC) != RESET))
         {
            if(TX_BUFFER_EMPTY)
            {
                // Buffer empty, so disable interrupts
                __HAL_UART_DISABLE_IT(huart, UART_IT_TC);
            }
            else
            {
                // There is more data in the output buffer. Send the next byte
                HAL_UART_Transmit_IT(uartstdio_device.UART_handle, &UART_TX_buffer[UART_TX_read_index], 1);
                ADVANCE_TX_BUFFER_INDEX(UART_TX_read_index);
            }
            
            //if (__HAL_DMA_GET_FLAG(uartstdio_device.DMA_handle, DMA_FLAG_TC4))
                //is_finished_sending = 1;
            /*
            if(TX_BUFFER_EMPTY)
                __HAL_UART_DISABLE_IT(uartstdio_device.UART_handle, UART_IT_TC);
            */
         }
            
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == uartstdio_device.UART_handle->Instance)
    {
        if(!RX_BUFFER_FULL)
        {
            if (RX_irq_char != '\n')
            {
                RX_irq_char -= 48;
                xQueueSendFromISR(queue_UART_RX_data, &RX_irq_char, NULL);
                BaseType_t higher_task_awoken = pdFALSE;
                vTaskNotifyGiveFromISR(Task_UART_RX_Handle, &higher_task_awoken);
                portYIELD_FROM_ISR(higher_task_awoken);
            }

            /*
            UART_RX_buffer[UART_RX_write_index] = RX_irq_char;
            ADVANCE_RX_BUFFER_INDEX(UART_RX_write_index);

            if (RX_irq_char = '\n')
            {
                UART_RX_frame_count++;
                BaseType_t higher_task_awoken = pdFALSE;
                vTaskNotifyGiveFromISR(Task_UART_RX_Handle, &higher_task_awoken);
                portYIELD_FROM_ISR(higher_task_awoken);
            }
            */
        }
        HAL_UART_Receive_IT(uartstdio_device.UART_handle, &RX_irq_char, 1);
    }
}

/*********************
 *   GLOBAL FUNCTION
 *********************/

/**
 * The function `UARTConfig` initializes a UART configuration with a specified receive buffer size and
 * sets up interrupt-driven reception.
 * 
 * @param uart_p The `uart_p` parameter is a pointer to a structure of type `UART_HandleTypeDef`, which
 * contains configuration settings for a UART (Universal Asynchronous Receiver-Transmitter) peripheral.
 * @param rx_buffer_size The `rx_buffer_size` parameter specifies the size of the receive buffer used
 * for storing incoming data in the UARTConfig function. This buffer will be initialized using the
 * QUEUE_Init function to manage incoming data efficiently.
 */
void UART_Config(UART_HandleTypeDef* p_UART_handle, uint16_t _TX_buffer_size ,uint16_t _RX_buffer_size, uint8_t _disable_echo)
{
    uartstdio_device.UART_handle    = p_UART_handle;
    uartstdio_device.TX_buffer_size = _TX_buffer_size;
    uartstdio_device.RX_buffer_size = _RX_buffer_size;
    uartstdio_device.disable_echo   = _disable_echo;

    UART_TX_buffer = (uint8_t *)malloc(UART_TX_BUFFER_SIZE * sizeof(uint8_t));
    //UART_RX_buffer = (uint8_t *)malloc(UART_RX_BUFFER_SIZE * sizeof(uint8_t));

    HAL_UART_Receive_IT(uartstdio_device.UART_handle, &RX_irq_char, 1);
    //QUEUE_Init(&rxBuffer, rx_buffer_size);
}

//*****************************************************************************
//
//! Writes a string of characters to the UART output.
//!
//! \param pcBuf points to a buffer containing the string to transmit.
//! \param ui16Len is the length of the string to transmit.
//!
//! This function will transmit the string to the UART output.  The number of
//! characters transmitted is determined by the \e ui16Len parameter.  This
//! function does no interpretation or translation of any characters.  Since
//! the output is sent to a UART, any LF (/n) characters encountered will be
//! replaced with a CRLF pair.
//!
//! Besides using the \e ui16Len parameter to stop transmitting the string, if
//! a null character (0) is encountered, then no more characters will be
//! transmitted and the function will return.
//!
//! In non-buffered mode, this function is blocking and will not return until
//! all the characters have been written to the output FIFO.  In buffered mode,
//! the characters are written to the UART transmit buffer and the call returns
//! immediately.  If insufficient space remains in the transmit buffer,
//! additional characters are discarded.
//!
//! \return Returns the count of characters written.
//
//*****************************************************************************
uint16_t UART_write(const char *pcBuf, uint16_t ui16Len)
{

    unsigned int uIdx;

    //
    // Check for valid arguments.
    //
    //
    // Send the characters
    //
    for(uIdx = 0; uIdx < ui16Len; uIdx++)
    {
        //
        // If the character to the UART is \n, then add a \r before it so that
        // \n is translated to \n\r in the output.
        //
        /*
        if(pcBuf[uIdx] == '\n')
        {
            if(!TX_BUFFER_FULL)
            {
                UART_TX_buffer[UART_TX_write_index] = '\r';
                ADVANCE_TX_BUFFER_INDEX(UART_TX_write_index);
            }
            else
            {
                //
                // Buffer is full - discard remaining characters and return.
                //
                break;
            }
        }
        */

        //
        // Send the character to the UART output.
        //
        if(!TX_BUFFER_FULL)
        {
            UART_TX_buffer[UART_TX_write_index] = pcBuf[uIdx];
            ADVANCE_TX_BUFFER_INDEX(UART_TX_write_index);
        }
        else
        {
            //
            // Buffer is full - discard remaining characters and return.
            //
            break;
        }
    }

    //
    // If we have anything in the buffer, make sure that the UART is set
    // up to transmit it.
    //
    /*
    if(!TX_BUFFER_EMPTY)
    {
        //__HAL_UART_ENABLE_IT(uartstdio_device.UART_handle, UART_IT_TC);
        //is_finished_sending = 0;
        if(pcBuf[uIdx - 1] == '\n')
            UART_prime_transmit(uIdx+1);
        else
            UART_prime_transmit(uIdx);
    }
    */
    if(__HAL_UART_GET_IT_SOURCE(uartstdio_device.UART_handle, UART_IT_TC) == 0)
        __HAL_UART_ENABLE_IT(uartstdio_device.UART_handle, UART_IT_TC);
    UART_prime_transmit();

    //
    // Return the number of characters written.
    //
    return(uIdx);
}

//*****************************************************************************
//
//! A simple UART based vprintf function supporting \%c, \%d, \%p, \%s, \%u,
//! \%x, and \%X.
//!
//! \param pc_string is the format string.
//! \param vaArgP is a variable argument list pointer whose content will depend
//! upon the format string passed in \e pc_string.
//!
//! This function is very similar to the C library <tt>vprintf()</tt> function.
//! All of its output will be sent to the UART.  Only the following formatting
//! characters are supported:
//!
//! - \%c to print a character
//! - \%d or \%i to print a decimal value
//! - \%s to print a string
//! - \%u to print an unsigned decimal value
//! - \%x to print a hexadecimal value using lower case letters
//! - \%X to print a hexadecimal value using lower case letters (not upper case
//! letters as would typically be used)
//! - \%p to print a pointer as a hexadecimal value
//! - \%\% to print out a \% character
//!
//! For \%s, \%d, \%i, \%u, \%p, \%x, and \%X, an optional number may reside
//! between the \% and the format character, which specifies the minimum number
//! of characters to use for that value; if preceded by a 0 then the extra
//! characters will be filled with zeros instead of spaces.  For example,
//! ``\%8d'' will use eight characters to print the decimal value with spaces
//! added to reach eight; ``\%08d'' will use eight characters as well but will
//! add zeroes instead of spaces.
//!
//! The type of the arguments in the variable arguments list must match the
//! requirements of the format string.  For example, if an integer was passed
//! where a string was expected, an error of some kind will most likely occur.
//!
//! \return None.
//
//*****************************************************************************
void UART_printf(const char *pc_string, ...)
{
    uint16_t ui16Idx, ui16Value, ui16Pos, ui16Count, ui16Base, ui16Neg;
    char *pcStr, pcBuf[16], cFill;

    //
    // Check the arguments.
    //

    va_list vaArgP;

    va_start(vaArgP, pc_string);

    //
    // Loop while there are more characters in the string.
    //
    while(*pc_string)
    {
        //
        // Find the first non-% character, or the end of the string.
        //
        for(ui16Idx = 0;
            (pc_string[ui16Idx] != '%') && (pc_string[ui16Idx] != '\0');
            ui16Idx++)
        {
        }

        //
        // Write this portion of the string.
        //
        UART_write(pc_string, ui16Idx);

        //
        // Skip the portion of the string that was written.
        //
        pc_string += ui16Idx;

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
            ui16Count = 0;
            cFill = ' ';

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
                    if((pc_string[-1] == '0') && (ui16Count == 0))
                    {
                        cFill = '0';
                    }

                    //
                    // Update the digit count.
                    //
                    ui16Count *= 10;
                    ui16Count += pc_string[-1] - '0';

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
                    ui16Value = va_arg(vaArgP, int);

                    //
                    // Print out the character.
                    //
                    UART_write((char *)&ui16Value, 1);

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
                    ui16Value = va_arg(vaArgP, int);

                    //
                    // Reset the buffer position.
                    //
                    ui16Pos = 0;

                    //
                    // If the value is negative, make it positive and indicate
                    // that a minus sign is needed.
                    //
                    if((int16_t)ui16Value < 0)
                    {
                        //
                        // Make the value positive.
                        //
                        ui16Value = -(int16_t)ui16Value;

                        //
                        // Indicate that the value is negative.
                        //
                        ui16Neg = 1;
                    }
                    else
                    {
                        //
                        // Indicate that the value is positive so that a minus
                        // sign isn't inserted.
                        //
                        ui16Neg = 0;
                    }

                    //
                    // Set the base to 10.
                    //
                    ui16Base = 10;

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
                    pcStr = va_arg(vaArgP, char *);

                    //
                    // Determine the length of the string.
                    //
                    for(ui16Idx = 0; pcStr[ui16Idx] != '\0'; ui16Idx++)
                    {
                    }

                    //
                    // Write the string.
                    //
                    UART_write(pcStr, ui16Idx);

                    //
                    // Write any required padding spaces
                    //
                    if(ui16Count > ui16Idx)
                    {
                        ui16Count -= ui16Idx;
                        while(ui16Count--)
                        {
                            UART_write(" ", 1);
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
                    ui16Value = va_arg(vaArgP, int);

                    //
                    // Reset the buffer position.
                    //
                    ui16Pos = 0;

                    //
                    // Set the base to 10.
                    //
                    ui16Base = 10;

                    //
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    //
                    ui16Neg = 0;

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
                    ui16Value = va_arg(vaArgP, int);

                    //
                    // Reset the buffer position.
                    //
                    ui16Pos = 0;

                    //
                    // Set the base to 16.
                    //
                    ui16Base = 16;

                    //
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    //
                    ui16Neg = 0;

                    //
                    // Determine the number of digits in the string version of
                    // the value.
                    //
convert:
                    for(ui16Idx = 1;
                        (((ui16Idx * ui16Base) <= ui16Value) &&
                         (((ui16Idx * ui16Base) / ui16Base) == ui16Idx));
                        ui16Idx *= ui16Base, ui16Count--)
                    {
                    }

                    //
                    // If the value is negative, reduce the count of padding
                    // characters needed.
                    //
                    if(ui16Neg)
                    {
                        ui16Count--;
                    }

                    //
                    // If the value is negative and the value is padded with
                    // zeros, then place the minus sign before the padding.
                    //
                    if(ui16Neg && (cFill == '0'))
                    {
                        //
                        // Place the minus sign in the output buffer.
                        //
                        pcBuf[ui16Pos++] = '-';

                        //
                        // The minus sign has been placed, so turn off the
                        // negative flag.
                        //
                        ui16Neg = 0;
                    }

                    //
                    // Provide additional padding at the beginning of the
                    // string conversion if needed.
                    //
                    if((ui16Count > 1) && (ui16Count < 16))
                    {
                        for(ui16Count--; ui16Count; ui16Count--)
                        {
                            pcBuf[ui16Pos++] = cFill;
                        }
                    }

                    //
                    // If the value is negative, then place the minus sign
                    // before the number.
                    //
                    if(ui16Neg)
                    {
                        //
                        // Place the minus sign in the output buffer.
                        //
                        pcBuf[ui16Pos++] = '-';
                    }

                    //
                    // Convert the value into a string.
                    //
                    for(; ui16Idx; ui16Idx /= ui16Base)
                    {
                        pcBuf[ui16Pos++] =
                            HEX_reference[(ui16Value / ui16Idx) % ui16Base];
                    }

                    //
                    // Write the string.
                    //
                    UART_write(pcBuf, ui16Pos);

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
                    UART_write(pc_string - 1, 1);

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
                    UART_write("ERROR", 5);

                    //
                    // This command has been handled.
                    //
                    break;
                }
            }
        }
    }
    va_end(vaArgP);
}

/*
//*****************************************************************************
//
//! Returns the number of bytes available in the receive buffer.
//!
//! This function, available only when the module is built to operate in
//! buffered mode using \b UART_BUFFERED, may be used to determine the number
//! of bytes of data currently available in the receive buffer.
//!
//! \return Returns the number of available bytes.
//
//*****************************************************************************
uint16_t UART_RX_bytes_avail(void)
{
    return(RX_BUFFER_USED);
}

//*****************************************************************************
//
//! A simple UART based get string function, with some line processing.
//!
//! \param pcBuf points to a buffer for the incoming string from the UART.
//! \param ui16Len is the length of the buffer for storage of the string,
//! including the trailing 0.
//!
//! This function will receive a string from the UART input and store the
//! characters in the buffer pointed to by \e pcBuf.  The characters will
//! continue to be stored until a termination character is received.  The
//! termination characters are CR, LF, or ESC.  A CRLF pair is treated as a
//! single termination character.  The termination characters are not stored in
//! the string.  The string will be terminated with a 0 and the function will
//! return.
//!
//! In both buffered and unbuffered modes, this function will block until
//! a termination character is received.  If non-blocking operation is required
//! in buffered mode, a call to UARTPeek() may be made to determine whether
//! a termination character already exists in the receive buffer prior to
//! calling UARTgets().
//!
//! Since the string will be null terminated, the user must ensure that the
//! buffer is sized to allow for the additional null character.
//!
//! \return Returns the count of characters that were stored, not including
//! the trailing 0.
//
//*****************************************************************************
uint16_t UART_get_string(char *pcBuf, uint16_t ui16Len)
{

    uint16_t ui16Count = 0;
    int8_t cChar;



    //
    // Adjust the length back by 1 to leave space for the trailing
    // null terminator.
    //
    ui16Len--;

    //
    // Process characters until a newline is received.
    //
    while(1)
    //while(!RX_BUFFER_EMPTY)
    {
        //
        // Read the next character from the receive buffer.
        //
        if(!RX_BUFFER_EMPTY)
        {
            cChar = UART_RX_buffer[UART_RX_read_index];
            ADVANCE_RX_BUFFER_INDEX(UART_RX_read_index);

            //
            // See if a newline or escape character was received.
            //
            if((cChar == '\r') || (cChar == '\n') || (cChar == 0x1b))
            {
                //
                // Stop processing the input and end the line.
                //
                break;
            }

            //
            // Process the received character as long as we are not at the end
            // of the buffer.  If the end of the buffer has been reached then
            // all additional characters are ignored until a newline is
            // received.
            //
            if(ui16Count < ui16Len)
            {
                //
                // Store the character in the caller supplied buffer.
                //
                pcBuf[ui16Count] = cChar;

                //
                // Increment the count of characters received.
                //
                ui16Count++;
            }
        }
    }

    //
    // Add a null termination to the string.
    //
    pcBuf[ui16Count] = 0;

    //
    // Return the count of int8_ts in the buffer, not counting the trailing 0.
    //
    return(ui16Count);
}
*/

/*
uint16_t UART_read_till_empty(char* pcBuf, uint16_t _RX_count)
{
    uint16_t RX_count  = 0;
    for (RX_count = 0; RX_count < _RX_count; RX_count++)
    {
        if (RX_BUFFER_EMPTY)
            break;

        *(pcBuf + RX_count) = UART_RX_buffer[UART_RX_read_index];
        ADVANCE_RX_BUFFER_INDEX(UART_RX_read_index);
        RX_count++;
    }

    return RX_count;
}
*/

/*
uint16_t UART_frame_avail()
{
    uint16_t frame_counted = 0;
    uint16_t buffer_index = 0;

     while (!RX_BUFFER_EMPTY)
    {
        if (UART_RX_buffer[UART_RX_read_index + buffer_index] == "\n")
        {
            ADVANCE_RX_BUFFER_INDEX(buffer_index);
            frame_counted++;
        }

        ADVANCE_RX_BUFFER_INDEX(buffer_index);
    }

    for()

    return frame_counted;
}
*/

/*
void UART_frame_process(char* command_buffer, uint16_t frame_to_process)
//void UART_frame_process(char* command_buffer)
{
    uint16_t frame_counted = 0;
    char char_buffer[5 * frame_to_count];

    for (frame_counted = 0; frame_counted < frame_to_count; frame_counted++)
    {
        for (uint8_t buffer_index = 0;; buffer_index++)
        {
            char_buffer[frame_counted * buffer_index] = UART_RX_buffer[UART_RX_read_index];
            ADVANCE_RX_BUFFER_INDEX(UART_RX_read_index);

            if (UART_RX_buffer[UART_RX_read_index] == "\n")
            {
                break;
            }
        }
    }

    for (uint16_t frame_to_process = 0; frame_to_process < frame_counted; frame_to_process++)
    {
        for (uint8_t buffer_index = 0;; buffer_index++)
        {
            if(char_buffer[frame_to_process * buffer_index] == "\n")
            {
                break;
            }

            switch (char_buffer[frame_to_process * buffer_index])
            {
            case UART_BUTTON_1:
                break;
            case UART_BUTTON_2:
                break;
            case UART_BUTTON_3:
                break;
            case UART_BUTTON_4:
                break;
            default:
                break;
            }

            command_buffer[frame_to_process * buffer_index] = char_buffer[frame_to_process * buffer_index] - "0";
        }
    }

    //uint8_t test;

    for(uint16_t frame_processed = 0; frame_processed < frame_to_process; frame_processed++)
    {
        //test = UART_RX_buffer[UART_RX_read_index];
        switch (UART_RX_buffer[UART_RX_read_index]) // 1<LF>
        {
        case '1'://UART_BUTTON_1:
            command_buffer[frame_processed] = 1;//BUTTON_1_PRESSED;
            break;
        case '2'://UART_BUTTON_2:
            command_buffer[frame_processed] = 2;//BUTTON_2_PRESSED;
            break;
        case '3'://UART_BUTTON_3:
            command_buffer[frame_processed] = 3;//BUTTON_3_PRESSED;
            break;
        case '4'://UART_BUTTON_4:
            command_buffer[frame_processed] = 4;//BUTTON_4_PRESSED;
            break;
        default:
            command_buffer[frame_processed] = 5;
            break;
        }

        ADVANCE_RX_BUFFER_INDEX(UART_RX_read_index);
        ADVANCE_RX_BUFFER_INDEX(UART_RX_read_index);
    }

    //return frame_counted;
}
*/