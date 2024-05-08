#include "http.h"
#include "uart_driver.h"
#include "driver/gpio.h"

static const char *TAG = "http_server";
static httpd_handle_t http_server = NULL;
static httpd_req_t *REQ;

static http_post_callback_t http_post_switch_callback = NULL;
static http_get_callback_t http_get_dht11_callback = NULL;

/*---404 NOT FOUND---*/
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if(strcmp("/dashboard", req->uri) == 0)
    {
        httpd_resp_send_err (req, HTTPD_404_NOT_FOUND, "DASHBOARD URI IS NOT AVAILABLE");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "404 NOT FOUND");
    return ESP_FAIL;
}

/*---DASHBOARD---*/
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]  asm("_binary_index_html_end");

static esp_err_t dashboard_display(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");  
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}

static const httpd_uri_t get_dashboard_display = 
{
    .uri = "/dashboard",
    .method = HTTP_GET,
    .handler = dashboard_display,
    .user_ctx = NULL
};

/*----------------------------------------SWITCH-----------------------------*/
// --------------Switch 1--------------//
static esp_err_t switch1_handler(httpd_req_t *req)
{
    char buf[5];
    char send_buffer[10];
    httpd_req_recv(req, buf, req->content_len);
    printf("SW1 change state: %s\n", buf);
    if (buf[0] == '1')
    {
        sprintf(send_buffer, "%d\n", UART_BUTTON_1);
        uart_tx_task(send_buffer);
    }
    else if (buf[0] == '0')
    {
        sprintf(send_buffer, "%d\n", UART_BUTTON_1);
        uart_tx_task(send_buffer);
    }
    
    return ESP_OK;
}

static const httpd_uri_t post_switch1 = 
{
    .uri = "/switch1",
    .method = HTTP_POST,
    .handler = switch1_handler,
    .user_ctx = NULL
};
// -------------------------------------//

// --------------Switch 2--------------//
static esp_err_t switch2_handler(httpd_req_t *req)
{
    char buf[5];
    char send_buffer[10];
    httpd_req_recv(req, buf, req->content_len);
    printf("SW2 change state: %s\n", buf);

    if (buf[0] == '1')
    {
        sprintf(send_buffer, "%d\n", UART_BUTTON_2);
        uart_tx_task(send_buffer);
    }
    else if (buf[0] == '0')
    {
        sprintf(send_buffer, "%d\n", UART_BUTTON_2);
        uart_tx_task(send_buffer);
    }

    return ESP_OK;
}

static const httpd_uri_t post_switch2 = 
{
    .uri = "/switch2",
    .method = HTTP_POST,
    .handler = switch2_handler,
    .user_ctx = NULL
};
// -------------------------------------//

// --------------Switch 3--------------//
static esp_err_t switch3_handler(httpd_req_t *req)
{
    char buf[5];
    char send_buffer[10];
    httpd_req_recv(req, buf, req->content_len);
    printf("SW3 change state: %s\n", buf);

    if (buf[0] == '1')
    {
        sprintf(send_buffer, "%d\n", UART_BUTTON_3);
        uart_tx_task(send_buffer);
    }
    else if (buf[0] == '0')
    {
        sprintf(send_buffer, "%d\n", UART_BUTTON_3);
        uart_tx_task(send_buffer);
    }

    return ESP_OK;
}

static const httpd_uri_t post_switch3 = {
    .uri = "/switch3",
    .method = HTTP_POST,
    .handler = switch3_handler,
    .user_ctx = NULL
};
// -------------------------------------//

// --------------Switch 4--------------//
static esp_err_t switch4_handler(httpd_req_t *req)
{
    char buf[5];
    char send_buffer[10];
    httpd_req_recv(req, buf, req->content_len);
    printf("SW4 change state: %s\n", buf);

    if (buf[0] == '1')
    {
        sprintf(send_buffer, "%d\n", UART_BUTTON_4);
        uart_tx_task(send_buffer);
    }
    else if (buf[0] == '0')
    {
        sprintf(send_buffer, "%d\n", UART_BUTTON_4);
        uart_tx_task(send_buffer);
    }

    return ESP_OK;
}

static const httpd_uri_t post_switch4 = {
    .uri = "/switch4",
    .method = HTTP_POST,
    .handler = switch4_handler,
    .user_ctx = NULL
};
// -------------------------------------//

// Switch 5
    static esp_err_t switch5_handler(httpd_req_t *req)
{
    char buf[5];
    char send_buffer[10];
    httpd_req_recv(req, buf, req->content_len);
    printf("SW5 change state: %s\n", buf);

    if (buf[0] == '1')
    {
        sprintf(send_buffer, "%d\n", UART_BUTTON_5_AUTO);
        uart_tx_task(send_buffer);
    }
    else if (buf[0] == '0')
    {
        sprintf(send_buffer, "%d\n", UART_BUTTON_5_HAND);
        uart_tx_task(send_buffer);
    }

    return ESP_OK;
}

static const httpd_uri_t post_switch5 = {
    .uri = "/switch5",
    .method = HTTP_POST,
    .handler = switch5_handler,
    .user_ctx = NULL
};

/*-------------------------------------SENSOR----------------------------------*/

// --------------TEMP & HUMID --------------//
static esp_err_t dht11_get_handler(httpd_req_t *req)
{
    REQ = req;
    http_get_dht11_callback();
    return ESP_OK;
}

void dht11_response(char *data, int len)
{
    httpd_resp_send(REQ, data, len);
}

static const httpd_uri_t get_data_dht11 = {
    .uri = "/get_data_dht11",
    .method = HTTP_GET,
    .handler = dht11_get_handler,
    .user_ctx = NULL
};
// -------------------------------------//

// -------------- PIR --------------//
// static esp_err_t pir_get_handler(httpd_req_t *req)
// {
//     // const char* resp_str = (const char*) "{\"temperature\": \"40\", \"humidity\": \"90\"}";
//     // httpd_resp_send(req, resp_str, strlen(resp_str));
//     // return ESP_OK;
// }

// static const httpd_uri_t get_data_dht11 = {
//     .uri = "/get_data_dht11",
//     .method = HTTP_GET,
//     .handler = pir_get_handler,
//     .user_ctx = NULL
// };
// // -------------------------------------//


void http_start_webserver(void)
{
    httpd_config_t http_conf = HTTPD_DEFAULT_CONFIG();
    http_conf.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting server on port: '%d'", http_conf.server_port);  
    if(httpd_start(&http_server, &http_conf) == ESP_OK)
    {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_err_handler(http_server, HTTPD_404_NOT_FOUND, http_404_error_handler);
        httpd_register_uri_handler(http_server, &get_dashboard_display);
        httpd_register_uri_handler(http_server, &post_switch1);
        httpd_register_uri_handler(http_server, &post_switch2);
        httpd_register_uri_handler(http_server, &post_switch3);
        httpd_register_uri_handler(http_server, &post_switch4);
        httpd_register_uri_handler(http_server, &post_switch5);
        httpd_register_uri_handler(http_server, &get_data_dht11);
        ESP_LOGI(TAG, "Finish Regist URI handlers");
    }else {
        ESP_LOGI(TAG, "Error starting server!");
    }
}

void http_stop_webserver(void)
{
    httpd_stop(http_server);
}

void http_set_callback_switch(void *callback)
{
    http_post_switch_callback = callback;
}

void http_set_callback_dht11(void *callback)
{
    http_get_dht11_callback = callback;
}