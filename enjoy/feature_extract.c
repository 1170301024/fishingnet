#include    <stdlib.h>  
#include    <stdio.h>
#include    <unistd.h>
#include    <string.h>
#include    <pthread.h>
#include    "../include/enjoy.h"
#include    "../include/joy/joy_api.h"


#define IP_OR_VLAN  "ip or ip6 or vlan"

int snaplen = 65535;

pcap_t * open_pcap_device(char *device){
    pcap_t *pd;
    struct bpf_program fp;
    bpf_u_int32 localnet, netmask;
    char filter_exp[PCAP_ERRBUF_SIZE], errbuf[PCAP_BUF_SIZE];
    if(NULL == device){
        err_quit("the device is null");
    }
    printf("device %s is being opened", device);
    if((pd = pcap_open_live(device, snaplen, 0, snaplen, errbuf) == NULL)){
        err_quit("pcap_open_live: %s", errbuf);
    }
    // more actions
    if(pcap_lookupnet(device, &localnet, &netmask, errbuf) < 0){
        err_quit("pcap_lookupnet: %s", errbuf);
    }
    memset(&fp,  0x00, sizeof(struct bpf_program));
    strncpy(filter_exp, IP_OR_VLAN, strnlen(IP_OR_VLAN));
    if(pcap_compile(pd, &fp, filter_exp, 0, netmask) < 0){
        err_quit("pcap_compile: %s", pcap_geterr(pd));
    }
    if(pcap_setfilter(pd, &fp) < 0){
        err_quit("pcap_setfilter: %s", pcap_geterr(pd));
    }

    return pd;
}

int init_joy(){
    joy_init_t init_data;

    memset(&init_data, 0x00, sizeof init_data);

#ifdef __ENJOY_DEBUG
    init_data.verbosity = 1;
#else
    init_data.verbosity = 4;
#endif

    init_data.max_records = 1;
    init_data.num_pkts = 50;
    init_data.contexts = 1;
    init_data.idp = 1400;

    // turn on all bitmask value except JOY_IPFIX_EXPORT_ON
    init_data.bitmask = JOY_ALL_ON ^ JOY_IPFIX_EXPORT_ON;

    if(joy_initialize(&init_data, NULL, NULL, NULL) != 0){
        err_quit("joy initialized failed");
    }
    for(int n=0; n < init_data.contexts; n++){
        joy_print_config(n, JOY_JSON_FORMAT);
    }

}
// should i use thread? one thread for one device? or any other way?
/*
 * one context work for a device or one joy for a device?
 *   
 *
 */





