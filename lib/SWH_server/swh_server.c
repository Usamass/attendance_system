#include "esp_log.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "swh_server.h"
#include "../SWH_web_pages.h"
#include "../user_login_data.h"
static char* HTTP_SERVER_TAG = "server tag";
static char* JSON_TAG = "json tag";
#define MIN(a,b) (((a)<(b))?(a):(b))

esp_err_t login_page(httpd_req_t *req)
{
    int response;
   
    response = httpd_resp_send(req, loginPage, HTTPD_RESP_USE_STRLEN);
    return response;
}
static esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[100];
    char* username = NULL;
    char* password = NULL;
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }
        ESP_LOGI(JSON_TAG, "Deserialize the reponse");
        cJSON *root2 = cJSON_Parse(buf);
        if (cJSON_GetObjectItem(root2, "username")) {
            username = cJSON_GetObjectItem(root2,"username")->valuestring;
            ESP_LOGI(JSON_TAG, "username=%s",username);
        }
        if (cJSON_GetObjectItem(root2, "password")) {
            password = cJSON_GetObjectItem(root2,"password")->valuestring;
            ESP_LOGI(JSON_TAG, "password=%s",password);
        }

        if (!(strcmp(username , usr.username) || strcmp(password , usr.password))){
            ESP_LOGI(HTTP_SERVER_TAG , "login sucessful!");
            httpd_resp_set_status(req, HTTPD_200);
            httpd_resp_set_hdr(req, "Content-Type", "text/plain");

            // Send the header with a NULL payload and length 0
            httpd_resp_send(req, NULL, 0);          

        }
        else {
            ESP_LOGI(HTTP_SERVER_TAG , "login unsucessful!");
            httpd_resp_send_404(req);
        }

    }


    return ESP_OK;
}
esp_err_t dashboard_handler(httpd_req_t *req)
{
    int response;
   
    response = httpd_resp_send(req, dashboard, HTTPD_RESP_USE_STRLEN);
    return response;
}

void swh_server_init()
{
   
    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server_handle = NULL;
    ESP_LOGI(HTTP_SERVER_TAG, "Starting server on port: '%d'", server_config.server_port);

    if (httpd_start(&server_handle, &server_config) == ESP_OK){
        httpd_uri_t uri_led_color = {
            .uri = "/login",
            .method = HTTP_GET,
            .handler = login_page,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server_handle, &uri_led_color);

        httpd_uri_t uri_off = {
        .uri = "/getData",
        .method = HTTP_POST,
        .handler = echo_post_handler,
        .user_ctx = NULL
        };

        httpd_register_uri_handler(server_handle, &uri_off);


        httpd_uri_t uri_buzzer_on = {
            .uri = "/dashboard",
            .method = HTTP_GET,
            .handler = dashboard_handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server_handle , &uri_buzzer_on);


        // httpd_uri_t uri_buzzer_off = {
        //     .uri = "/buzzer/off",
        //     .method = HTTP_GET,
        //     .handler = buzzer_off,
        //     .user_ctx = NULL
        // };

        // httpd_register_uri_handler(server_handle , &uri_buzzer_off);

        // httpd_uri_t uri_system_info = {
        //     .uri = "/GetSystemInfo",
        //     .method = HTTP_GET,
        //     .handler = get_system_info,
        //     .user_ctx = NULL
        // };

       // httpd_register_uri_handler(server_handle , &uri_system_info);
    }
    else{
        ESP_LOGI(HTTP_SERVER_TAG, "Error starting server!");
    }

}