#include "swh_client.h"
char* client_receive_buffer = NULL;
int content_len = 0;

static const char *HTTP_CLIENT_TAG = "http client";

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer; // Buffer to store response of http request from event handler
    static int output_len;      // Stores number of bytes read
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        // content_len = evt->data_len;
        // ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        // ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_RESPONSE_DATA: %s \n", (char *)evt->data);
        /*
         *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
         *  However, event handler can also be used in case chunked encoding is used.
         */
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            
            // If user_data buffer is configured, copy the response into the buffer
            int copy_len = 0;
            if (evt->user_data)
            {
                
                copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                if (copy_len)
                {
                    memcpy(evt->user_data + output_len, evt->data, copy_len);
                }
            }
            else
            {
                

                const int buffer_len = esp_http_client_get_content_length(evt->client);
                if (output_buffer == NULL)
                {
                    output_buffer = (char *)malloc(buffer_len);
                    output_len = 0;
                    if (output_buffer == NULL)
                    {
                        ESP_LOGE(HTTP_CLIENT_TAG, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                copy_len = MIN(evt->data_len, (buffer_len - output_len));
                if (copy_len)
                {
                    memcpy(output_buffer + output_len, evt->data, copy_len);
                }
            }
            output_len += copy_len;
        }
        else {
            if (evt->user_data) {
                int copy_len = 0;
                if (evt->user_data)
                {
                    
                    copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                    if (copy_len)
                    {
                        memcpy(evt->user_data + output_len, evt->data, copy_len);
                    }

                }
            }

        }

        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != NULL)
        {
            // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
            // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_DISCONNECTED");

        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_REDIRECT");
        esp_http_client_set_header(evt->client, "From", "user@example.com");
        esp_http_client_set_header(evt->client, "Accept", "text/html");
        esp_http_client_set_redirection(evt->client);
        break;
    }
    return ESP_OK;
}


esp_err_t getStudentsData(device_config_t dConfig)
{
    const int locationID = getLocationId(&dConfig);
    const char *auth = getAuthToken(&dConfig);

   // client_receive_buffer[MAX_HTTP_OUTPUT_BUFFER];
    client_receive_buffer = (char *)malloc(MAX_HTTP_OUTPUT_BUFFER * sizeof(char));
    char url[70] = {0};

    esp_err_t err;
    
    snprintf(url, sizeof(url), "http://192.168.50.209:8000/api/students/location/%d", locationID);

    esp_http_client_config_t config = {
        .host = "192.168.50.209",
        // .host = "2a85849f-67d6-40e7-a2cc-87c61ef2ac71.mock.pstmn.io",
        // .path = "/getStudentData",
        .url = url,
        .method = HTTP_METHOD_GET,
        .event_handler = _http_event_handler,
        .user_data = client_receive_buffer, // Pass address of local buffer to get response
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "token", auth);

    // GET
    err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(client);

        ESP_LOGI(HTTP_CLIENT_TAG, "HTTP GET Status = %d", status_code);
        if (status_code == 200)
        {
            client_receive_buffer[content_len] = '\0';
            ESP_LOGI(HTTP_CLIENT_TAG , "client_recieve -> %s" , client_receive_buffer);
            //xEventGroupSetBits(spiffs_event_group, CLIENT_RECIEVED_BIT);
        }
    }
    else
    {
        ESP_LOGE(HTTP_CLIENT_TAG, "HTTP GET request failed: %s", esp_err_to_name(err));

        return ESP_FAIL;
    }
    ESP_LOG_BUFFER_HEX(HTTP_CLIENT_TAG, client_receive_buffer, strlen(client_receive_buffer));

    return ESP_OK;
}
