#define ETHERNET_CONNECT_FLAG 12
#define ETHERNET_DISCONNECT_FLAG 13
#define GOT_IP_FLAG 14
#define CLIENT_WRITE_FLAG (0x09)
#define CLIENT_READ_FLAG (0x08)
#define DEVICE_CONFIG_WRITE_FLAG (0x04)
#define LOAD_MAPPING_FLAG (0x05)
#define HTML_FILE_READ (0x15)
#define HTML_FILE_WRITE (0x16)

// Fingerprint flags
#define FINGERPRINT_SUCCESS 0x01
#define FINGERPRINT_NOT_MACHING 0x02
#define FINGERPRINT_RELEASE_MSG 0x03
#define FINGERPRINT_RELEASE_CODE 0x04
#define FINGERPRINT_ENROLL_ERROR 0x05
#define FINGERPRINT_SENCOND_PRINT 0x06
#define FINGERPRINT_STORE_SUCCESS 0x07
#define FINGERPRINT_ENROLL_CODE 0x08
#define FINGERPRINT_MAX_TAMP 0x09
#define FINGERPRINT_ALREADY_THERE 0x10