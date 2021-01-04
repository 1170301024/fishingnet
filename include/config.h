#ifndef _CONFIG_H
#define _CONFIG_H

#include    <stdbool.h>
#include    "type.h"

/** maximum line length */
#define LINEMAX 512

#define NULL_KEYWORD "none"
#define NULL_KEYWORD_LEN 4

/** Header description length */
#define HDR_DSC_LEN 32

#define MAX_IDP 1500

// "anyaddr" IPv4 address denotes server can bind all of the address on the mechine 
#define IPv4_ANYADDR    "anyaddr";

// "auto" configuration for interface
#define AUTO_IFF    "auto";

// connection service configuration
struct connect_service_config{
    char *address;      // IPv4 protocol
    uint8_t port;
};

// distribution service configuration
struct distribute_service_config{
    char *address;      // IPv4 protocol
    uint8_t port;
};

/* each attribute represents a feature within the feature extraction and whether 
   or not it is turned on
*/
struct data_feature_config{
    bool bidir;
    bool zeroes;
    bool retrans;
    bool byte_distribution;
    bool entropy;
    bool exe;
    bool idp;
    bool hd;

    bool dns;
    bool ssh;
    bool tls;
    bool dhcp;
    bool dhcpv6;
    bool http;
    bool ike;
    bool ppi;
    bool payload;
};


typedef struct configuration{
    
    // include connection service configuration
    struct connect_service_config connect_s_cfg;

    // include distribution service configuration 
    struct distribute_service_config distribute_s_cfg;

    // include data feature configuration
    struct data_feature_config data_feature_cfg;

    // other configuration
    char *interface;
    char *logfile;

    uint8_t verbosity;
    bool show_config;
    bool show_interface;
} configuration_t;
#endif