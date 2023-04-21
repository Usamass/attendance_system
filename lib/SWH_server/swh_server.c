#include <stdlib.h>
#include "esp_log.h"
#include "esp_http_server.h"
#include "freertos/event_groups.h"
#include "device_configs.h" // for getting mac address or setting device configs.
#include "cJSON.h"
#include "swh_server.h"
#include "../SWH_web_pages.h"
#include "user_login_data.h"
#include "device_configs.h"
#include "swh_client.h"
#include "../SWH_data_buffers.h"
#include "../SWH_eventGroups.h"
#include "../event_bits.h"

extern device_config_t dConfig;

const char stdData[] = "[{\"id\":12,\"name\":\"mahnoo\",\"user_name\":\"bc123456789\"},{\"id\":13,\"name\":\"hfkgdfgka\",\"user_name\":\"adlhglhlaghl\"},{\"id\":14,\"name\":\"dhfaghjhakv,\",\"user_name\":\"adlhglhhflhjvk\"},{\"id\":16,\"name\":\"Prof. Sterling Brakus\",\"user_name\":\"gino.hoeger\"},{\"id\":17,\"name\":\"Miss Naomi Kunze\",\"user_name\":\"treutel.sammy\"},{\"id\":22,\"name\":\"Victor Senger II\",\"user_name\":\"moshe.thompson\"}]";


static char* HTTP_SERVER_TAG = "server tag";
static char* JSON_TAG = "json tag";
static bool login_flag = false;
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
    cJSON* root2 = NULL;
    user_credentials* usr = usr_data_init();
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
        root2 = cJSON_Parse(buf);
        if (cJSON_GetObjectItem(root2, "username")) {
            username = cJSON_GetObjectItem(root2,"username")->valuestring;
            ESP_LOGI(JSON_TAG, "username=%s",username);
        }
        if (cJSON_GetObjectItem(root2, "password")) {
            password = cJSON_GetObjectItem(root2,"password")->valuestring;
            ESP_LOGI(JSON_TAG, "password=%s",password);
        }

        if (!(strcmp(username , usr->username) || strcmp(password , usr->password))){
            ESP_LOGI(HTTP_SERVER_TAG , "login sucessful!");
            login_flag = true;

            // Send the header with a NULL payload and length 0
            httpd_resp_send(req, NULL, 0);          

        }
        else {
            ESP_LOGI(HTTP_SERVER_TAG , "login unsucessful!");
            httpd_resp_send_404(req);
        }

    }

    cJSON_Delete(root2);

    return ESP_OK;
}
esp_err_t dashboard_handler(httpd_req_t *req)
{
    int response;
    const char* un_authMsg = "Please login first";
    ESP_LOGI("logs" , "login flag: %d" , login_flag);

    
    if (login_flag){
    response = httpd_resp_send(req, dashboard, HTTPD_RESP_USE_STRLEN);
    return response;
    }
    else {
    response = httpd_resp_send(req, un_authMsg, HTTPD_RESP_USE_STRLEN);
    return response;


    }

    return ESP_OK;

}

esp_err_t logout_handler(httpd_req_t *req){
    login_flag = false;
    ESP_LOGI("logs" , "login flag: %d" , login_flag);
    int response;
   
    response = httpd_resp_send(req, NULL , 0);
    return response;
}

esp_err_t get_network(httpd_req_t *req){
    int response;
    const char* un_authMsg = "Please login first";
    ESP_LOGI("logs" , "login flag: %d" , login_flag);
    if (login_flag){
    cJSON *root;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "ip", getIpAddr(&dConfig));
	cJSON_AddStringToObject(root, "mac", getMacAddr(&dConfig));
	const char *my_json_string = cJSON_Print(root);
    cJSON_Delete(root);
    response = httpd_resp_send(req , my_json_string , HTTPD_RESP_USE_STRLEN);
    return response;
    }
    else {
        response = httpd_resp_send(req, un_authMsg, HTTPD_RESP_USE_STRLEN);
        return response;

    }

}

esp_err_t get_device_configs(httpd_req_t* req)
{
    char buf[200];
    cJSON* root2 = NULL;

    char* device_id = NULL;
    char* auth_token = NULL;
    char* device_location = NULL;
    char* location_id = NULL;

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
        root2 = cJSON_Parse(buf);
        if (cJSON_GetObjectItem(root2, "device_id")) {
            device_id = cJSON_GetObjectItem(root2,"device_id")->valuestring;
            ESP_LOGI(JSON_TAG, "device_id=%s",device_id);
        }
        if (cJSON_GetObjectItem(root2, "auth_token")) {
            auth_token = cJSON_GetObjectItem(root2,"auth_token")->valuestring;
            ESP_LOGI(JSON_TAG, "auth_token=%s",auth_token);
        }
        if (cJSON_GetObjectItem(root2, "device_location")) {
            device_location = cJSON_GetObjectItem(root2,"device_location")->valuestring;
            ESP_LOGI(JSON_TAG, "device_location=%s",device_location);
        }
        if (cJSON_GetObjectItem(root2, "location_id")) {
            location_id = cJSON_GetObjectItem(root2,"location_id")->valuestring;
            ESP_LOGI(JSON_TAG, "location_id=%s",location_id);
        }
        /* setting all the parameters of device configs*/
        setDeviceId(&dConfig , device_id);
        setAuthToken(&dConfig , auth_token);
        setDeviceLocation(&dConfig , device_location);
        setLocationId(&dConfig , atoi(location_id));
        xEventGroupSetBits(spiffs_event_group , DEVICE_CONFIG_BIT); // test it outside the while loop
    }

    cJSON_Delete(root2);
    return ESP_OK;




}

esp_err_t enrollment_page(httpd_req_t* req){
    int response;
    response = httpd_resp_send(req , enrollmentPage , HTTPD_RESP_USE_STRLEN);
    return response;



}
esp_err_t get_std_data(httpd_req_t* req){
    int response;
    getStudentsData(dConfig);
    // ESP_LOGI("inside server get_std_data" , "%s" , client_receive_buffer);  // bug is here!

    response = httpd_resp_send(req , client_receive_buffer , HTTPD_RESP_USE_STRLEN);
    return response;
}

esp_err_t get_date_time(httpd_req_t* req){
    char buf[50];
    int response;

    char* date = NULL;
    char* time = NULL;
    
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
        if (cJSON_GetObjectItem(root2, "date")) {
            date = cJSON_GetObjectItem(root2,"date")->valuestring;
            ESP_LOGI(JSON_TAG, "date=%s",date);
        }
        if (cJSON_GetObjectItem(root2, "time")) {
            time = cJSON_GetObjectItem(root2,"time")->valuestring;
            ESP_LOGI(JSON_TAG, "time=%s",time);
        }

    }
     


    response = httpd_resp_send(req , NULL , 0);
    return response;


}



void swh_server_init()
{
   
    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server_handle = NULL;
    ESP_LOGI(HTTP_SERVER_TAG, "Starting server on port: '%d'", server_config.server_port);

    if (httpd_start(&server_handle, &server_config) == ESP_OK){
        httpd_uri_t uri_login = {
            .uri = "/login",
            .method = HTTP_GET,
            .handler = login_page,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server_handle, &uri_login);

        httpd_uri_t uri_get_data = {
        .uri = "/getData",
        .method = HTTP_POST,
        .handler = echo_post_handler,
        .user_ctx = NULL
        };

        httpd_register_uri_handler(server_handle, &uri_get_data);


        httpd_uri_t uri_dashboard = {
            .uri = "/dashboard",
            .method = HTTP_GET,
            .handler = dashboard_handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server_handle , &uri_dashboard);


       httpd_uri_t uri_logout = {
            .uri = "/logout",
            .method = HTTP_GET,
            .handler = logout_handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server_handle , &uri_logout);

        httpd_uri_t uri_network = {
            .uri = "/network",
            .method = HTTP_GET,
            .handler = get_network,
            .user_ctx = NULL
        };

       httpd_register_uri_handler(server_handle , &uri_network);


       httpd_uri_t uri_get_std_data = {
            .uri = "/getStdData",
            .method = HTTP_GET,
            .handler = get_std_data,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server_handle , &uri_get_std_data); 


        httpd_uri_t uri_get_configs = {
            .uri = "/getDeviceConfigs",
            .method = HTTP_POST,
            .handler = get_device_configs,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server_handle , &uri_get_configs);

    //    httpd_uri_t uri_get_date_time = {
    //         .uri = "/dateTime",
    //         .method = HTTP_POST,
    //         .handler = get_date_time,
    //         .user_ctx = NULL
    //     };

    //    httpd_register_uri_handler(server_handle , &uri_get_date_time);

       httpd_uri_t uri_get_enrollment = {
            .uri = "/enrollment",
            .method = HTTP_GET,
            .handler = enrollment_page,
            .user_ctx = NULL
        };

       httpd_register_uri_handler(server_handle , &uri_get_enrollment);

        


    }
    else{
        ESP_LOGI(HTTP_SERVER_TAG, "Error starting server!");
    }

}