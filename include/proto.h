#ifndef _PROTO_H
#define _PROTO_H

#include "user.h"

/* THIS PORT is used to connect, register. restore and configurate */
#define SERVER_UDP_CONNECT_PORT 60720
#define SERVER_UDP_CONNECT_IPv4 INADDR_ANY

/* THIS PORT is used to distrubite feature to user */
#define FEATURE_DISTRIBUTE_PORT 60721
#define FEATURE_DISTRIBUTE_IPv4 INADDR_ANY

#define MAX_UDP_MSG 4096

struct feature_option{
    char code;
    struct{
        short length; // the length field only include the length of the value field
        char *value;
    }ft_data;
};

struct user_register{
    char *username;
    char *passwd;
};
/* udp connect protocol
 *    0        8        16               32
 *    +--------+--------+----------------+
 *    |  type  | code   |  proto length  |
 *    +--------+--------+----------------+
 * 
 */

#define CHEADER_LEN 4
// udp connect type
#define CONNECT 0x01
#define PAUSE   0x03
#define RESTORE 0x04
#define CONFIG  0x05
#define RESPONSE 0x06

#define CONNECT_RSP 0x11
#define PAUSE_RSP   0x12
#define RESTORE_RSP 0x13
#define CONFIG_RSP  0x14

// response code
#define CMRSP_OK  0x01
#define CMRSP_ERR 0x02


/* monitor protocol and feature transport protocol
 *    0        8        16               32
 *    +--------+--------+----------------+
 *    |  type  | code   |  proto length  |
 *    +--------+--------+----------------+
 * 
 */

#define TICK    0X01
#define TICK_RSP    0x02

// tick response code
#define CTICK_RSP_OK 0x01

#define FEATURE 0x03
#define MESSAGE 0x04

#define GENERAL_CODE  0x01
/* message code */
// finish processing a pcap file
#define PROTO_CODE_MSG_PCAP_FIN 0x2

// a error was encountered while processing pcap file
#define PROTO_CODE_MSG_PCAP_ERR 0x3

struct protocol{
    char type;
    char code;
    short proto_length;
    char *data;
    
};

typedef struct protocol udp_connect_protocol;

int init_udp_connect();


#endif