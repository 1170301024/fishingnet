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
#define IPv4_ANYADDR    "anyaddr"

// "auto" configuration for interface
#define AUTO_IFF    "auto"

// connection service configuration
struct connect_service_config{
    char *address;      // IPv4 protocol
    uint16_t port;
};

// distribution service configuration
struct distribute_service_config{
    char *address;      // IPv4 protocol
    uint16_t port;
};

/* each attribute represents a feature within the feature extraction and whether 
   or not it is turned on.
   the struct is used to feature extraction mainly.
*/
struct data_feature_config{

    /* 1. feature extraction related configuration*/
    uint16_t num_pkts;
    uint16_t inact_timeout;
    uint16_t idp;       // idp = 0 for close idp feation extraction option non-0 for open
    
    /* 2. for feature options*/
    bool bidir;
    bool zeroes;
    bool retains;
    bool byte_distribution;
    bool entropy;
    bool exe;
    bool hd;

    // protocol related feature options
    bool dns;
    bool ssh;
    bool tls;
    bool dhcp;
    bool http;
    bool ike;
    bool ppi;
    bool salt;
    bool payload;
};


typedef struct fnet_configuration{
    
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
} fnet_configuration_t;


int config_data_feature_from_file (struct data_feature_config *config, const char *fname);

int config_from_xml(fnet_configuration_t *config, const char *fname);

void fnet_config_print (FILE *f, const fnet_configuration_t *c);
#endif