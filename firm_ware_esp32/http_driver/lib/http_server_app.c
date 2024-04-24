#include "http_server_app.h"

uint8_t relay1_state;
uint8_t relay2_state;
uint8_t relay3_state;
uint8_t relay4_state;

static const char *TAG = "http_server";
static httpd_handle_t http_server = NULL;

// static xQueueHandle xQueue_SwitchPost;

enum SwitchID_State{SW1_ON, SW1_OFF, SW2_ON, SW2_OFF, SW3_ON, SW3_OFF};
enum SwitchID_State SWID_St;

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

/*---SWITCH---*/
// Switch 1
static esp_err_t switch1_handler(httpd_req_t *req)
{
    char buf[5];
    httpd_req_recv(req, buf, req->content_len);
    printf("SW1 change state: %s\n", buf);
    if (buf[0] == '1')
    {
        if(relay1_state == RELAY_OFF)
        {
            relay_change_state(RELAY_1);
            main_display_update();
        }
    }else if (buf[0] == '0')
    {
        if(relay1_state == RELAY_ON)
        {
            relay_change_state(RELAY_1);
            main_display_update();
        }
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

// Switch 2
static esp_err_t switch2_handler(httpd_req_t *req)
{
    char buf[5];
    httpd_req_recv(req, buf, req->content_len);
    printf("SW2 change state: %s\n", buf);

    if (buf[0] == '1')
    {
        if(relay2_state == RELAY_OFF)
        {
            relay_change_state(RELAY_2);
            main_display_update();
        }
    }else if (buf[0] == '0')
    {
        if(relay2_state == RELAY_ON)
        {
            relay_change_state(RELAY_2);
            main_display_update();
        }
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

// Switch 3
static esp_err_t switch3_handler(httpd_req_t *req)
{
    char buf[5];
    httpd_req_recv(req, buf, req->content_len);
    printf("SW3 change state: %s\n", buf);

    if (buf[0] == '1')
    {
        if(relay3_state == RELAY_OFF)
        {
            relay_change_state(RELAY_3);
            main_display_update();
        }
    }else if (buf[0] == '0')
    {
        if(relay3_state == RELAY_ON)
        {
            relay_change_state(RELAY_3);
            main_display_update();
        }
    }

    return ESP_OK;
}

static const httpd_uri_t post_switch3 = {
    .uri = "/switch3",
    .method = HTTP_POST,
    .handler = switch3_handler,
    .user_ctx = NULL
};

void http_start_webserver(void)
{
    httpd_config_t http_conf = HTTPD_DEFAULT_CONFIG();
    // allow the http server to close our socket if needed
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
    }else {
        ESP_LOGI(TAG, "Error starting server!");
    }
}

void http_stop_webserver(void)
{
    //Stop the http server
    httpd_stop(http_server);
}