
#include "wifi.h"
#include "http.h"
#include "uart_driver.h"
#include "string.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUFFER_SIZE 100

static const int RX_BUF_SIZE = 1024;
char buffer_uart_rx[BUFFER_SIZE];
char humidity_buffer [10];
char temperature_buffer [10];
char pir_buffer [10];
char lm393_buffer [10];

static const char *TAG_5 = "uart_task";

// void uart_get_pir_data()
// {
//     uart_send_command();
// }


// void uart_get_lm393_data()
// {

// }


// void uart_send_command(UART_frame_t header,uint8_t data, UART_frame_t stopflag)
// {
//     uart_tx_task('header');

// }

// void switch_data_callback()
// {

// }
/*---------------------------------------*/

void dht11_data_callback ()
{
    char resp[100];
    sprintf(resp, "{\"temperature\": \"%s\", \"humidity\": \"%s\"}", temperature_buffer, humidity_buffer);
    //dht11_response(resp, strlen(resp));
}

void uart_rx_task(void *arg)
{
    uint8_t buffer_temp[RX_BUF_SIZE + 1];
    uint16_t pos_buffer_uart_rx = 0;
    uint8_t enable_bit = 0;
    int8_t data_heading = -1;
    uint16_t i;

    memset((void *)buffer_uart_rx, '\0', sizeof(buffer_uart_rx));

    while (1)
    {
        uint16_t rxBytes = uart_read_bytes(UART_NUM_1, &buffer_temp,
                                           RX_BUF_SIZE, 10 / portTICK_PERIOD_MS);
        if (rxBytes > 0)
        {
        printf("read header\n");
            for (i = 0; i < rxBytes; i++)
            {
                buffer_uart_rx[pos_buffer_uart_rx] = buffer_temp[i];
                if (buffer_uart_rx[pos_buffer_uart_rx] == '\n')
                {
                    enable_bit = 1;
                    buffer_uart_rx[pos_buffer_uart_rx] = '\0';
                    pos_buffer_uart_rx = 0;
                    break;
                }
                pos_buffer_uart_rx++;
            }

            if (enable_bit == 0)
            {
                continue;
            }

            data_heading = buffer_uart_rx[0];
            uint8_t size_buffer = strlen(buffer_uart_rx);

            for (i = 0; i < size_buffer; i++)
            {
                buffer_uart_rx[i] = buffer_uart_rx[i + 1];
            }

            switch (data_heading)
            {
            case UART_BUTTON_1:
                ESP_LOGI(TAG_5,"UART_BUTTON_1");
                if (memcmp(buffer_uart_rx, "0", strlen(buffer_uart_rx) + 1) == 0)
                {
                    //button chuyển về 0
                }
                else if (memcmp(buffer_uart_rx, "1", strlen(buffer_uart_rx) + 1) == 0)
                {
                    //button chuyển về 1
                }
                break;

            case UART_BUTTON_2:
                ESP_LOGI(TAG_5,"UART_BUTTON_2");
                if (memcmp(buffer_uart_rx, "0", strlen(buffer_uart_rx) + 1) == 0)
                {
                    //button chuyển về 0
                }
                else if (memcmp(buffer_uart_rx, "1", strlen(buffer_uart_rx) + 1) == 0)
                {
                    //button chuyển về 1  
                }
                break;

            case UART_BUTTON_3:
                ESP_LOGI(TAG_5,"UART_BUTTON_3");
                if (memcmp(buffer_uart_rx, "0", strlen(buffer_uart_rx) + 1) == 0)
                {
                    
                }
                else if (memcmp(buffer_uart_rx, "1", strlen(buffer_uart_rx) + 1) == 0)
                {
                    
                }
                break;

            case UART_BUTTON_4:
                ESP_LOGI(TAG_5,"UART_BUTTON_4");
                if (memcmp(buffer_uart_rx, "0", strlen(buffer_uart_rx) + 1) == 0)
                {
            
                }
                else if (memcmp(buffer_uart_rx, "1", strlen(buffer_uart_rx) + 1) == 0)
                {
            
                }
                break;

            case UART_DHT_HUMID:
                memcpy(humidity_buffer, buffer_uart_rx, strlen(buffer_uart_rx) + 1);
                ESP_LOGI(TAG_5,"UART_DHT_HUMID");
                ESP_LOGI(TAG_5,"%s",humidity_buffer);
                break;

            case UART_DHT_TEMP:
                memcpy(temperature_buffer, buffer_uart_rx, strlen(buffer_uart_rx) + 1);
                ESP_LOGI(TAG_5,"UART_DHT_TEMP");
                ESP_LOGI(TAG_5,"%s",temperature_buffer);
                break;

            case UART_PIR:
                memcpy(pir_buffer, buffer_uart_rx, strlen(buffer_uart_rx) + 1);
                break;

            case UART_LM393:
                memcpy(lm393_buffer, buffer_uart_rx, strlen(buffer_uart_rx) + 1);
                break;

            default:
                enable_bit = 0;
                memset((void *)buffer_uart_rx, '\0', sizeof(buffer_uart_rx));
                break;

                memset((void *)buffer_uart_rx, '\0', sizeof(buffer_uart_rx));
            }

            enable_bit = 0;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    http_set_callback_dht11(dht11_data_callback);

    uart_init();
    wifi_init_sta();

    http_start_webserver();
    xTaskCreate(uart_rx_task, "uarttask", 1024*10, NULL, 2, NULL);
}
