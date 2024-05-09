#include "app.h"

extern SPI_HandleTypeDef hspi2;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

extern UART_HandleTypeDef huart1;

typedef struct _DHT_data_t
{
  uint8_t DHT_data[5];
}DHT_data_t;

//TODO: is this necessary?
static relay_light_state_t relay_light_auto_state   = RELAY_LED_OFF;
static relay_light_state_t relay_light_wait_state   = RELAY_LED_OFF;


static relay_light_state_t relay_light_1_state      = RELAY_LED_AUTO;
static relay_light_state_t relay_light_2_state      = RELAY_LED_AUTO;

static relay_humid_fan_state_t relay_humid_state    = RELAY_HUMID_OFF_AUTO;
static relay_humid_fan_state_t relay_fan_state      = RELAY_FAN_OFF_AUTO;

        xQueueHandle queue_relay_light_data;
        xQueueHandle queue_UART_RX_data;
static  xQueueHandle queue_DHT_data;
static  xQueueHandle queue_relay_humid_fan_data;

static  xQueueHandle queue_UART_TX_state;
static  xQueueHandle queue_TFT_state;

        TaskHandle_t Task_Relay_Light_Handle;
        TaskHandle_t Task_UART_RX_Handle;
static  TaskHandle_t Task_UART_TX_Handle;
static  TaskHandle_t Task_TFT_Display_Handle;
static  TaskHandle_t Task_Relay_Humid_Fan_Handle;
static  TaskHandle_t Task_DHT_Get_Data_Handle;

static void Task_Relay_Light(void *pvParameters);
static void Task_UART_RX(void *pvParameters);
static void Task_UART_TX(void *pvParameters);
static void Task_TFT_Display(void *pvParameters);
static void Task_Relay_Humid_Fan(void *pvParameters);
static void Task_DHT_Get_Data(void *pvParameters);

static void UART_Frame_TX(UART_frame_t frame_data, uint8_t TX_data[], uint8_t TX_count);

void App_Main(void)
{
    Delay_Init(&htim3, &htim2);
    UART_Config(&huart1, 128, 64, 0);

    DHT_Init(GPIOB, GPIO_PIN_0, &htim3);
    PIR_AM312_Init(GPIOB, GPIO_PIN_1);
    LM393_Init(GPIOB, GPIO_PIN_2);

    ILI9341_Init();
    ILI9341_SetRotation(SCREEN_HORIZONTAL_1);

    ILI9341_FillScreen(WHITE);
    ILI9341_Draw_LVGL(40, 50, 48, 48, HUMID_ICON);
    ILI9341_Draw_LVGL(190, 50, 48, 48, TEMP_ICON);
    ILI9341_Draw_LVGL(40, 140, 48, 48, YES_PEOPLE_ICON);
    ILI9341_Draw_LVGL(190, 140, 48, 48, SUN_ICON);

    queue_relay_light_data      = xQueueCreate(10, sizeof(relay_light_data_t));
    queue_UART_RX_data          = xQueueCreate(16, sizeof(UART_frame_t));
    queue_DHT_data              = xQueueCreate(10, sizeof(DHT_data_t));
    queue_relay_humid_fan_data  = xQueueCreate(10, sizeof(relay_humid_fan_data_t));

    queue_UART_TX_state         = xQueueCreate(32, sizeof(UART_TX_state_t));
    queue_TFT_state             = xQueueCreate(10, sizeof(TFT_state_t));

    xTaskCreate(Task_Relay_Light, "Task_Relay_Light", 256, NULL, 7, &Task_Relay_Light_Handle);
    xTaskCreate(Task_UART_RX, "Task_UART_RX", 256, NULL, 6, &Task_UART_RX_Handle);
    xTaskCreate(Task_UART_TX, "Task_UART_TX", 512, NULL, 5, &Task_UART_TX_Handle);
    xTaskCreate(Task_TFT_Display, "Task_TFT_Display", 256, NULL, 4, &Task_TFT_Display_Handle);
    xTaskCreate(Task_Relay_Humid_Fan, "Task_Relay_Humid_Fan", 256, NULL,3, &Task_Relay_Humid_Fan_Handle);
    xTaskCreate(Task_DHT_Get_Data, "Task_DHT_Get_Data", 256, NULL, 2, &Task_DHT_Get_Data_Handle);

    vTaskStartScheduler();

    while (1)
    {
        ;
    }
}

static void Task_Relay_Light(void *pvParameters)
{
    while (1)
    {
        ulTaskNotifyTake(pdFALSE,portMAX_DELAY);

        UART_TX_state_t    UART_TX_state_1_give     = UART_TX_IDLE;
        UART_TX_state_t    UART_TX_state_2_give     = UART_TX_IDLE;
        UART_TX_state_t    UART_TX_state_auto_give  = UART_TX_IDLE;

        relay_light_data_t relay_light_data_take = RELAY_LIGHT_DATA_NULL;
        xQueueReceive(queue_relay_light_data, &relay_light_data_take, 0);

        switch (relay_light_auto_state)
        {
        case RELAY_LED_OFF:
            if (relay_light_data_take == PIR_ON)
            {
                if (lm393_get_data() == 1)
                    break;
                
                relay_light_auto_state = RELAY_LED_ON_TIMER;

                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_RESET);

                UART_TX_state_1_give = UART_TX_RELAY_1_ON;
                UART_TX_state_2_give = UART_TX_RELAY_2_ON;

                __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
                HAL_TIM_Base_Start_IT(&htim1);
            }
            else if ((relay_light_data_take == RELAY_LIGHT_BUTTON_5_HAND) || 
                    (relay_light_data_take == BUTTON_1_PRESSED) || 
                    (relay_light_data_take == BUTTON_2_PRESSED))
            {
                relay_light_auto_state = RELAY_LED_HAND;
                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_SET);
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_SET);
            }
            break;
        case RELAY_LED_ON_TIMER:
            if (relay_light_data_take == TIMER_GO_OFF && pir_am312_get_data() == 0)
            {   
                relay_light_auto_state = RELAY_LED_OFF;

                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_SET);
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_SET);

                UART_TX_state_1_give = UART_TX_RELAY_1_OFF;
                UART_TX_state_2_give = UART_TX_RELAY_2_OFF;
            }
            else if (relay_light_data_take == TIMER_GO_OFF)
            {
                __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
                HAL_TIM_Base_Start_IT(&htim1);
            }
            else if ((relay_light_data_take == RELAY_LIGHT_BUTTON_5_HAND) || 
                    (relay_light_data_take == BUTTON_1_PRESSED) || 
                    (relay_light_data_take == BUTTON_2_PRESSED))
            {
                relay_light_auto_state = RELAY_LED_HAND;
                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_SET);
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_SET);
            }
            break;
        case RELAY_LED_HAND:
            if (relay_light_data_take == RELAY_LIGHT_BUTTON_5_AUTO)
            {
                relay_light_auto_state = RELAY_LED_OFF;
            }
            break;
        default:
            break;
        }

        switch (relay_light_1_state)
        {
        case RELAY_LED_OFF:
            if ((relay_light_data_take == BUTTON_1_PRESSED))
            {
                relay_light_1_state = RELAY_LED_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_RESET);
                UART_TX_state_1_give = UART_TX_RELAY_1_ON;
            }
            else if ((relay_light_data_take == WEB_BUTTON_1_PRESSED))
            {
                relay_light_1_state = RELAY_LED_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_RESET);
            }
            else if (relay_light_data_take == RELAY_LIGHT_BUTTON_5_AUTO)
            {
                relay_light_1_state = RELAY_LED_AUTO;
                relay_light_2_state = RELAY_LED_AUTO;
                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_SET);
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_SET);
            }
            break;

        case RELAY_LED_ON_BUTTON:
            if ((relay_light_data_take == BUTTON_1_PRESSED))
            {
                relay_light_1_state = RELAY_LED_OFF;
                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_SET);
                UART_TX_state_1_give = UART_TX_RELAY_1_OFF;
            }
            else if ((relay_light_data_take == WEB_BUTTON_1_PRESSED))
            {
                relay_light_1_state = RELAY_LED_OFF;
                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_SET);
            }
            else if (relay_light_data_take == RELAY_LIGHT_BUTTON_5_AUTO)
            {
                relay_light_1_state = RELAY_LED_AUTO;
                relay_light_2_state = RELAY_LED_AUTO;
                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_SET);
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_SET);
            }
            break;
        case RELAY_LED_AUTO:
            if (relay_light_data_take == RELAY_LIGHT_BUTTON_5_HAND)
            {
                relay_light_1_state = RELAY_LED_OFF;
                relay_light_2_state = RELAY_LED_OFF;
            }
            else if (relay_light_data_take == BUTTON_1_PRESSED)
            {
                relay_light_1_state = RELAY_LED_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_RESET);
                UART_TX_state_1_give = UART_TX_RELAY_1_ON;
                UART_TX_state_auto_give = UART_TX_AUTO_OFF;
            }
            else if (relay_light_data_take == WEB_BUTTON_1_PRESSED)
            {
                relay_light_1_state = RELAY_LED_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_RESET);
            }
            break;
        default:
            break;
        }

        switch (relay_light_2_state)
        {
        case RELAY_LED_OFF:
            if ((relay_light_data_take == BUTTON_2_PRESSED))
            {
                relay_light_2_state = RELAY_LED_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_RESET);
                UART_TX_state_2_give = UART_TX_RELAY_2_ON;
            }
            else if ((relay_light_data_take == WEB_BUTTON_2_PRESSED))
            {
                relay_light_2_state = RELAY_LED_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_RESET);
            }
            else if (relay_light_data_take == RELAY_LIGHT_BUTTON_5_AUTO)
            {
                relay_light_1_state = RELAY_LED_AUTO;
                relay_light_2_state = RELAY_LED_AUTO;
                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_SET);
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_SET);
            }
            break;

        case RELAY_LED_ON_BUTTON:
            if ((relay_light_data_take == BUTTON_2_PRESSED))
            {
                relay_light_2_state = RELAY_LED_OFF;
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_SET);
                UART_TX_state_2_give = UART_TX_RELAY_2_OFF;
            }
            else if ((relay_light_data_take == WEB_BUTTON_2_PRESSED))
            {
                relay_light_2_state = RELAY_LED_OFF;
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_SET);
            }
            else if (relay_light_data_take == RELAY_LIGHT_BUTTON_5_AUTO)
            {
                relay_light_1_state = RELAY_LED_AUTO;
                relay_light_2_state = RELAY_LED_AUTO;
                HAL_GPIO_WritePin(RELAY_1_PORT, RELAY_1_PIN, GPIO_PIN_SET);
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_SET);
            }
            break;
        case RELAY_LED_AUTO:
            if (relay_light_data_take == RELAY_LIGHT_BUTTON_5_HAND)
            {
                relay_light_1_state = RELAY_LED_OFF;
                relay_light_2_state = RELAY_LED_OFF;
            }
            else if ((relay_light_data_take == BUTTON_2_PRESSED))
            {
                relay_light_2_state = RELAY_LED_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_RESET);
                UART_TX_state_2_give = UART_TX_RELAY_2_ON;
                UART_TX_state_auto_give = UART_TX_AUTO_OFF;
            }
            else if ((relay_light_data_take == WEB_BUTTON_2_PRESSED))
            {
                relay_light_2_state = RELAY_LED_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_2_PORT, RELAY_2_PIN, GPIO_PIN_RESET);
            }
            break;
        default:
            break;
        }

        xQueueSend(queue_UART_TX_state, &UART_TX_state_1_give, 0);
        xQueueSend(queue_UART_TX_state, &UART_TX_state_2_give, 0);
        xQueueSend(queue_UART_TX_state, &UART_TX_state_auto_give, 0);
        xTaskNotifyGive(Task_UART_TX_Handle);
        xTaskNotifyGive(Task_UART_TX_Handle);
        xTaskNotifyGive(Task_UART_TX_Handle);
    }
}

static void Task_UART_RX(void *pvParameters)
{
    while (1)
    {
        ulTaskNotifyTake(pdFALSE,portMAX_DELAY);

        relay_light_data_t          relay_light_UART_RX_give = RELAY_LIGHT_DATA_NULL;
        relay_humid_fan_data_t      relay_humid_fan_data_UART_RX_give = RELAY_HUMID_FAN_BUTTON_NULL;
        UART_frame_t                UART_RX_data_take;

        xQueueReceive(queue_UART_RX_data, &UART_RX_data_take, 0);

        switch (UART_RX_data_take)
        {
        case UART_BUTTON_1:
            relay_light_UART_RX_give = WEB_BUTTON_1_PRESSED;
            break;
        case UART_BUTTON_2:
            relay_light_UART_RX_give = WEB_BUTTON_2_PRESSED;
            break;
        case UART_BUTTON_3:
            relay_humid_fan_data_UART_RX_give = WEB_BUTTON_3_PRESSED;
            break;
        case UART_BUTTON_4:
            relay_humid_fan_data_UART_RX_give = WEB_BUTTON_4_PRESSED;
            break;
        case UART_BUTTON_5_AUTO:
            relay_light_UART_RX_give = RELAY_LIGHT_BUTTON_5_AUTO;
            relay_humid_fan_data_UART_RX_give = RELAY_HUMID_FAN_5_AUTO;
            break;
        case UART_BUTTON_5_HAND:
            relay_light_UART_RX_give = RELAY_LIGHT_BUTTON_5_HAND;
            relay_humid_fan_data_UART_RX_give = RELAY_HUMID_FAN_5_HAND;
            break;
        default:
            break;
        }

        xQueueSend(queue_relay_light_data, &relay_light_UART_RX_give, 0);
        xQueueSend(queue_relay_humid_fan_data, &relay_humid_fan_data_UART_RX_give, 0);
        xTaskNotifyGive(Task_Relay_Light_Handle);
        xTaskNotifyGive(Task_Relay_Humid_Fan_Handle);
    }
}

static void UART_Frame_TX(UART_frame_t frame_data, uint8_t TX_data[], uint8_t TX_count)
{
    UART_printf("%c", frame_data);

    if(frame_data == UART_DHT_HUMID || frame_data == UART_DHT_TEMP)
    {
        UART_printf("%d", TX_data[0]);
        UART_write(".", 1);
        UART_printf("%d", TX_data[1]);
    }
    else
        UART_printf("%d", TX_data[0]);

    UART_printf("%c", UART_STOP_FRAME);
}

static void Task_UART_TX(void *pvParameters)
{
    while (1)
    {
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

        DHT_data_t          UART_TX_DHT_data_take = {0};
        UART_TX_state_t     UART_TX_state_take = UART_TX_IDLE;

        uint8_t relay_1_data, relay_2_data, relay_3_data, relay_4_data, PIR_data, LM_data;

        xQueueReceive(queue_UART_TX_state, &UART_TX_state_take, 0);
        xQueuePeek(queue_DHT_data, &UART_TX_DHT_data_take, 0);

        switch (UART_TX_state_take)
        {
        case UART_TX_IDLE:
            break;
        case UART_TX_RELAY_1_ON:
            relay_1_data = 1;
            UART_Frame_TX(UART_BUTTON_1, &relay_1_data, 1);
            UART_TX_state_take = UART_TX_IDLE;
            break;
        case UART_TX_RELAY_1_OFF:
            relay_1_data = 0;
            UART_Frame_TX(UART_BUTTON_1, &relay_1_data, 1);
            UART_TX_state_take = UART_TX_IDLE;
            break;
        case UART_TX_RELAY_2_ON:
            relay_2_data = 1;
            UART_Frame_TX(UART_BUTTON_2, &relay_2_data, 1);
            UART_TX_state_take = UART_TX_IDLE;
            break;
        case UART_TX_RELAY_2_OFF:
            relay_2_data = 0;
            UART_Frame_TX(UART_BUTTON_2, &relay_2_data, 1);
            UART_TX_state_take = UART_TX_IDLE;
            break;
        case UART_TX_RELAY_3_ON:
            relay_3_data = 1;
            UART_Frame_TX(UART_BUTTON_3, &relay_3_data, 1);
            UART_TX_state_take = UART_TX_IDLE;
            break;
        case UART_TX_RELAY_3_OFF:
            relay_3_data = 0;
            UART_Frame_TX(UART_BUTTON_3, &relay_3_data, 1);
            UART_TX_state_take = UART_TX_IDLE;
            break;
        case UART_TX_RELAY_4_ON:
            relay_4_data = 1;
            UART_Frame_TX(UART_BUTTON_4, &relay_4_data, 1);
            UART_TX_state_take = UART_TX_IDLE;
            break;
        case UART_TX_RELAY_4_OFF:
            relay_4_data = 0;
            UART_Frame_TX(UART_BUTTON_4, &relay_4_data, 1);
            UART_TX_state_take = UART_TX_IDLE;
            break;
        case UART_TX_DHT:
            UART_Frame_TX(UART_DHT_HUMID, &UART_TX_DHT_data_take.DHT_data[0], 2);
            vTaskDelay(60);
            UART_Frame_TX(UART_DHT_TEMP, &UART_TX_DHT_data_take.DHT_data[2], 2);
            UART_TX_state_take = UART_TX_IDLE;
            break;
        case UART_TX_PIR_ON:
            PIR_data = 1;
            UART_Frame_TX(UART_PIR, &PIR_data, 1);
            UART_TX_state_take = UART_TX_IDLE;
            break;
        case UART_TX_PIR_OFF:
            PIR_data = 0;
            UART_Frame_TX(UART_PIR, &PIR_data, 1);
            UART_TX_state_take = UART_TX_IDLE;
            break;
        case UART_TX_LM_ON:
            LM_data = 1;
            UART_Frame_TX(UART_PIR, &LM_data, 1);
            UART_TX_state_take = UART_TX_IDLE;
            break;
        case UART_TX_LM_OFF:
            LM_data = 0;
            UART_Frame_TX(UART_PIR, &LM_data, 1);
            UART_TX_state_take = UART_TX_IDLE;
            break;
        default:
            break;
        }
    }
}

static void Task_TFT_Display( void *pvParameters )
{
    DHT_data_t      TFT_DHT_data_take = {0};
    TFT_state_t     TFT_state_take = TFT_IDLE;

    while (1)
    {
        ulTaskNotifyTake(pdFALSE,portMAX_DELAY);
        
        xQueuePeek(queue_DHT_data, &TFT_DHT_data_take, 0);
        xQueueReceive(queue_TFT_state, &TFT_state_take, 0);

        switch (TFT_state_take)
        {
        case TFT_IDLE:
            break;
        case TFT_DHT:
            ILI9341_ConfigPrint(FONT4, 100, 70, BLACK, WHITE);
            ILI9341_Print("%d.%d", TFT_DHT_data_take.DHT_data[0], TFT_DHT_data_take.DHT_data[1]);
            
            ILI9341_ConfigPrint(FONT4, 250, 70, BLACK, WHITE);
            ILI9341_Print("%d.%d", TFT_DHT_data_take.DHT_data[2], TFT_DHT_data_take.DHT_data[3]);
            TFT_state_take = TFT_IDLE;
            break;
        case TFT_PIR_ON:
            ILI9341_ConfigPrint(FONT4, 0, 38, WHITE, BLACK);
            ILI9341_DrawRectangle(0, 38, 20*15, 20, BLACK);

            ILI9341_Print("Phong co nguoi");

            TFT_state_take = TFT_IDLE;
            break;
        case TFT_PIR_OFF:
            ILI9341_ConfigPrint(FONT4, 0, 38, WHITE, BLACK);
            ILI9341_DrawRectangle(0, 38, 20*15, 20, BLACK);

            ILI9341_Print("Phong khong co nguoi");
            
            TFT_state_take = TFT_IDLE;
            break;
        case TFT_LM393_ON:
            ILI9341_ConfigPrint(FONT4, 0, 57, WHITE, BLACK);
            ILI9341_DrawRectangle(0, 57, 10*15, 20, BLACK);

            ILI9341_Print("Phong sang");

            TFT_state_take = TFT_IDLE;
            break;
        case TFT_LM393_OFF:
            ILI9341_ConfigPrint(FONT4, 0, 57, WHITE, BLACK);
            ILI9341_DrawRectangle(0, 57, 10*15, 20, BLACK);

            ILI9341_Print("Phong toi");

            TFT_state_take = TFT_IDLE;
            break;
        default:
            break;
        }
    }
}

static void Task_Relay_Humid_Fan(void *pvParameters)
{
    while (1)
    {
        ulTaskNotifyTake(pdFALSE,portMAX_DELAY);

        UART_TX_state_t    UART_TX_state_3_give     = UART_TX_IDLE;
        UART_TX_state_t    UART_TX_state_4_give     = UART_TX_IDLE;
        UART_TX_state_t    UART_TX_state_auto_give  = UART_TX_IDLE;

        DHT_data_t relay_humid_fan_DHT_data_take = {0};
        xQueueReceive(queue_DHT_data, &relay_humid_fan_DHT_data_take, 0);

        relay_humid_fan_data_t relay_hummid_fan_button_data_take = RELAY_HUMID_FAN_BUTTON_NULL;
        xQueueReceive(queue_relay_humid_fan_data, &relay_hummid_fan_button_data_take, 0);

        switch (relay_humid_state)
        {
        case RELAY_HUMID_OFF_AUTO:
            //if (relay_humid_fan_DHT_data_take.DHT_data[0] > 75)
            if (relay_humid_fan_DHT_data_take.DHT_data[0] > 75)
            {
                relay_humid_state = RELAY_HUMID_ON_COMPARE;
                HAL_GPIO_WritePin(RELAY_3_PORT, RELAY_3_PIN, GPIO_PIN_RESET);
                UART_TX_state_3_give = UART_TX_RELAY_3_ON;
            }
            else if (relay_hummid_fan_button_data_take == BUTTON_3_PRESSED)
            {
                relay_humid_state = RELAY_HUMID_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_3_PORT, RELAY_3_PIN, GPIO_PIN_RESET);
                UART_TX_state_3_give = UART_TX_RELAY_3_ON;
                UART_TX_state_auto_give = UART_TX_AUTO_OFF;
            }
            else if (relay_hummid_fan_button_data_take == RELAY_HUMID_FAN_5_HAND)
            {
                relay_humid_state = RELAY_HUMID_OFF_HAND;
                HAL_GPIO_WritePin(RELAY_3_PORT, RELAY_3_PIN, GPIO_PIN_SET);
            }
            break;
        case RELAY_HUMID_OFF_HAND:
            if (relay_hummid_fan_button_data_take == BUTTON_3_PRESSED)
            {
                relay_humid_state = RELAY_HUMID_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_3_PORT, RELAY_3_PIN, GPIO_PIN_RESET);
                UART_TX_state_3_give = UART_TX_RELAY_3_ON;
            }
            else if (relay_hummid_fan_button_data_take == WEB_BUTTON_3_PRESSED)
            {
                relay_humid_state = RELAY_HUMID_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_3_PORT, RELAY_3_PIN, GPIO_PIN_RESET);
            }
            else if (relay_hummid_fan_button_data_take == RELAY_HUMID_FAN_5_AUTO)
            {
                relay_humid_state = RELAY_HUMID_OFF_AUTO;
                HAL_GPIO_WritePin(RELAY_3_PORT, RELAY_3_PIN, GPIO_PIN_SET);
            }
            break;
        case RELAY_HUMID_ON_COMPARE:
            if (relay_humid_fan_DHT_data_take.DHT_data[0] <= 75)
            {
                relay_humid_state = RELAY_HUMID_OFF_AUTO;
                HAL_GPIO_WritePin(RELAY_3_PORT, RELAY_3_PIN, GPIO_PIN_SET);
                UART_TX_state_3_give = UART_TX_RELAY_3_OFF;
            }
            else if (relay_hummid_fan_button_data_take == RELAY_HUMID_FAN_5_HAND)
            {
                relay_humid_state = RELAY_HUMID_OFF_HAND;
                HAL_GPIO_WritePin(RELAY_3_PORT, RELAY_3_PIN, GPIO_PIN_SET);
            }
            break;
        case RELAY_HUMID_ON_BUTTON:
            if (relay_hummid_fan_button_data_take == BUTTON_3_PRESSED)
            {
                relay_humid_state = RELAY_HUMID_OFF_HAND;
                HAL_GPIO_WritePin(RELAY_3_PORT, RELAY_3_PIN, GPIO_PIN_SET);
                UART_TX_state_3_give = UART_TX_RELAY_3_OFF;
            }
            else if (relay_hummid_fan_button_data_take == WEB_BUTTON_3_PRESSED)
            {
                relay_humid_state = RELAY_HUMID_OFF_HAND;
                HAL_GPIO_WritePin(RELAY_3_PORT, RELAY_3_PIN, GPIO_PIN_SET);
            }
            else if (relay_hummid_fan_button_data_take == RELAY_HUMID_FAN_5_AUTO)
            {
                relay_humid_state = RELAY_HUMID_OFF_AUTO;
                HAL_GPIO_WritePin(RELAY_3_PORT, RELAY_3_PIN, GPIO_PIN_SET);
            }
            break;
        default:
            break;
        }

        switch (relay_fan_state)
        {
        case RELAY_FAN_OFF_AUTO:
            //if (relay_humid_fan_DHT_data_take.DHT_data[2] > 60)
            if (relay_humid_fan_DHT_data_take.DHT_data[2] > 32)
            {
                relay_fan_state = RELAY_FAN_ON_COMPARE;
                HAL_GPIO_WritePin(RELAY_4_PORT, RELAY_4_PIN, GPIO_PIN_RESET);
                UART_TX_state_4_give = UART_TX_RELAY_4_ON;
            }
            else if (relay_hummid_fan_button_data_take == BUTTON_4_PRESSED)
            {
                relay_fan_state = RELAY_FAN_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_4_PORT, RELAY_4_PIN, GPIO_PIN_RESET);
                UART_TX_state_4_give = UART_TX_RELAY_4_ON;
                UART_TX_state_auto_give = UART_TX_AUTO_OFF;
            }
            else if (relay_hummid_fan_button_data_take == RELAY_HUMID_FAN_5_HAND)
            {
                relay_fan_state = RELAY_FAN_OFF_HAND;
                HAL_GPIO_WritePin(RELAY_4_PORT, RELAY_4_PIN, GPIO_PIN_SET);
            }
            break;
        case RELAY_FAN_OFF_HAND:
            if (relay_hummid_fan_button_data_take == BUTTON_4_PRESSED)
            {
                relay_fan_state = RELAY_FAN_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_4_PORT, RELAY_4_PIN, GPIO_PIN_RESET);
                UART_TX_state_4_give = UART_TX_RELAY_4_ON;
            }
            else if (relay_hummid_fan_button_data_take == WEB_BUTTON_4_PRESSED)
            {
                relay_fan_state = RELAY_FAN_ON_BUTTON;
                HAL_GPIO_WritePin(RELAY_4_PORT, RELAY_4_PIN, GPIO_PIN_RESET);
            }
            else if (relay_hummid_fan_button_data_take == RELAY_HUMID_FAN_5_AUTO)
            {
                relay_fan_state = RELAY_FAN_OFF_AUTO;
                HAL_GPIO_WritePin(RELAY_4_PORT, RELAY_4_PIN, GPIO_PIN_SET);
            }
            break;
        case RELAY_FAN_ON_COMPARE:
            if (relay_humid_fan_DHT_data_take.DHT_data[2] <= 32)
            {
                relay_fan_state = RELAY_FAN_OFF_AUTO;
                HAL_GPIO_WritePin(RELAY_4_PORT, RELAY_4_PIN, GPIO_PIN_SET);
                UART_TX_state_4_give = UART_TX_RELAY_4_OFF;
            }
            else if (relay_hummid_fan_button_data_take == RELAY_HUMID_FAN_5_HAND)
            {
                relay_fan_state = RELAY_FAN_OFF_HAND;
                HAL_GPIO_WritePin(RELAY_4_PORT, RELAY_4_PIN, GPIO_PIN_SET);
            }
            break;
        case RELAY_FAN_ON_BUTTON:
            if (relay_hummid_fan_button_data_take == BUTTON_4_PRESSED)
            {
                relay_fan_state = RELAY_FAN_OFF_HAND;
                HAL_GPIO_WritePin(RELAY_4_PORT, RELAY_4_PIN, GPIO_PIN_SET);
                UART_TX_state_4_give = UART_TX_RELAY_4_OFF;
            }
            else if (relay_hummid_fan_button_data_take == WEB_BUTTON_4_PRESSED)
            {
                relay_fan_state = RELAY_FAN_OFF_HAND;
                HAL_GPIO_WritePin(RELAY_4_PORT, RELAY_4_PIN, GPIO_PIN_SET);
            }
            else if (relay_hummid_fan_button_data_take == RELAY_HUMID_FAN_5_AUTO)
            {
                relay_fan_state = RELAY_FAN_OFF_AUTO;
                HAL_GPIO_WritePin(RELAY_4_PORT, RELAY_4_PIN, GPIO_PIN_SET);
            }
            break;
        default:
            break;
        }

        xQueueSend(queue_UART_TX_state, &UART_TX_state_3_give, 0);
        xQueueSend(queue_UART_TX_state, &UART_TX_state_4_give, 0);
        xQueueSend(queue_UART_TX_state, &UART_TX_state_auto_give, 0);
        xTaskNotifyGive(Task_UART_TX_Handle);
        xTaskNotifyGive(Task_UART_TX_Handle);
        xTaskNotifyGive(Task_UART_TX_Handle);
    }
}

static void Task_DHT_Get_Data(void *pvParameters)
{
    UART_TX_state_t     UART_TX_state_give  = UART_TX_DHT;
    TFT_state_t         TFT_state_give      = TFT_DHT;

    while (1)
    {
        //taskENTER_CRITICAL();
        DHT_data_t DHT_data_give = {0};
        dht_get_data(DHT_data_give.DHT_data);
        //taskEXIT_CRITICAL();

        if (DHT_data_give.DHT_data[4] == DHT_OK)
        {
            xQueueSend(queue_DHT_data, &DHT_data_give, 0);
            xQueueSend(queue_UART_TX_state, &UART_TX_state_give, 0);
            xQueueSend(queue_TFT_state, &TFT_state_give, 0);
            xTaskNotifyGive(Task_UART_TX_Handle);
            xTaskNotifyGive(Task_TFT_Display_Handle);
            xTaskNotifyGive(Task_Relay_Humid_Fan_Handle);
            vTaskDelay(10000);
        }
        else
        {
            vTaskDelay(10000);
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    relay_light_data_t          relay_light_data_exti_give = RELAY_LIGHT_DATA_NULL;
    relay_humid_fan_data_t      relay_humid_fan_data_exti_give = RELAY_HUMID_FAN_BUTTON_NULL;

    UART_TX_state_t             UART_TX_state_give = UART_TX_IDLE;
    TFT_state_t                 TFT_state_give = TFT_IDLE;

    BaseType_t higher_task_awoken = pdFALSE;

    switch (GPIO_Pin)
    {
    case BUTTON_1_PIN:
        relay_light_data_exti_give = BUTTON_1_PRESSED;
        break;
    case BUTTON_2_PIN:
        relay_light_data_exti_give = BUTTON_2_PRESSED;
        break;
    case BUTTON_3_PIN:
        relay_humid_fan_data_exti_give = BUTTON_3_PRESSED;
        break;
    case BUTTON_4_PIN:
        relay_humid_fan_data_exti_give = BUTTON_4_PRESSED;
        break;
    default:
        break;
    }
    
    xQueueSendFromISR(queue_relay_light_data, &relay_light_data_exti_give, NULL);
    xQueueSendFromISR(queue_relay_humid_fan_data, &relay_humid_fan_data_exti_give, NULL);
    xQueueSendFromISR(queue_TFT_state, &TFT_state_give, NULL);
    xQueueSendFromISR(queue_UART_TX_state, &UART_TX_state_give, NULL);
    vTaskNotifyGiveFromISR(Task_Relay_Light_Handle, &higher_task_awoken);
    vTaskNotifyGiveFromISR(Task_UART_TX_Handle, &higher_task_awoken);
    vTaskNotifyGiveFromISR(Task_TFT_Display_Handle, &higher_task_awoken);
    vTaskNotifyGiveFromISR(Task_Relay_Humid_Fan_Handle, &higher_task_awoken);

    portYIELD_FROM_ISR(higher_task_awoken);
}
