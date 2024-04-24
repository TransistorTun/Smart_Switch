#include "http.h"

static const char *TAG = "http_server";
static httpd_handle_t http_server = NULL;

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
    .uri = "/",
    .method = HTTP_GET,
    .handler = dashboard_display,
    .user_ctx = NULL
};

void http_start_webserver(void)
{
    //httpd_handle_t http_server = NULL;
    httpd_config_t http_conf = HTTPD_DEFAULT_CONFIG();
    // allow the http server to close our socket if needed
    // http_conf.server_port = 8001;
    http_conf.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting server on port: '%d'", http_conf.server_port);
    if(httpd_start(&http_server, &http_conf) == ESP_OK)
    {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_err_handler(http_server, HTTPD_404_NOT_FOUND, http_404_error_handler);
        httpd_register_uri_handler(http_server, &get_dashboard_display);
        // httpd_register_uri_handler(http_server, &post_switch1);
        // httpd_register_uri_handler(http_server, &post_switch2);
        // httpd_register_uri_handler(http_server, &post_switch3);
    }else {
        ESP_LOGI(TAG, "Error starting server!");
    }
}

void http_stop_webserver(void)
{
    //Stop the http server
    httpd_stop(http_server);
}