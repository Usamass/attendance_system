typedef struct {
    const char* username;
    const char* password;

}user_credentials;

user_credentials* usr_data_init();
const char* make_auth_token();