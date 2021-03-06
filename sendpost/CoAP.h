#define COAP_TYPE_CON 0x00
#define COAP_TYPE_NON 0x01
#define COAP_TYPE_ACK 0x02
#define COAP_TYPE_RST 0x03

#define COAP_METHOD_GET 0x01
#define COAP_METHOD_POST 0x02
#define COAP_METHOD_PUT 0x03
#define COAP_METHOD_DELETE 0x04

#define COAP_FORMAT_TEXT 0
#define COAP_FORMAT_XML  41
#define COAP_FORMAT_JSON 50

#define COAP_OPTION_IF_MATCH        1
#define COAP_OPTION_URI_HOST        3
#define COAP_OPTION_ETAG            4
#define COAP_OPTION_OBSERVE         6
#define COAP_OPTION_URI_PORT        7
#define COAP_OPTION_LOCATION_PATH   8
#define COAP_OPTION_URI_PATH       11 
#define COAP_OPTION_CONTENT        12
#define COAP_OPTION_MAX_AGE        14
#define COAP_OPTION_URI_QUERY      15
#define COAP_OPTION_ACCEPT         17
#define COAP_OPTION_LOCATION_QUERY 20
#define COAP_OPTION_BLOCK2         23
#define COAP_OPTION_PROXY_URI      35
#define COAP_OPTION_PROXY_SCHEME   39
#define COAP_OPTION_SIZE1          60
#define COAP_END_OPTION          0xFF 

