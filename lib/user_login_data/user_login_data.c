#include "user_login_data.h"
#include "esp_tls_crypto.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DST_BUF_SIZE 255
user_credentials usr = {
        .username = "osama",
        .password = "1234"
    };
const char* make_auth_token()
{
    size_t n = 0;
    char* usr_info = NULL;
    asprintf(&usr_info  , "%s:%s" , usr.username , usr.password);
    unsigned char* dst_buf = (unsigned char*)malloc(MAX_DST_BUF_SIZE * sizeof(unsigned char));

    esp_crypto_base64_encode(dst_buf , MAX_DST_BUF_SIZE , &n , (const unsigned char*)usr_info , strlen(usr_info));

    return (const char*)dst_buf;
}

user_credentials* usr_data_init()
{
    return &usr;

}