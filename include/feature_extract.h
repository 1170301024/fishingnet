#ifndef _FEATURE_EXTRACT_H
#define _FEATURE_EXTRACT_H
#include "pcap.h"  



#define IP_OR_VLAN  "ip"
#define IFL_MAX 16
#define INTFACENAMESIZE 64


#define MAC_ADDR_STR_LEN 32

#define NO_PACKETS_IN_LOOP 20

//
// Possible flags for the  iiFlags - bitmask.
//
#define IFF_UP              0x00000001 // Interface is up.
#define IFF_BROADCAST       0x00000002 // Broadcast is  supported.
#define IFF_LOOPBACK        0x00000004 // This is loopback interface.
#define IFF_POINTTOPOINT    0x00000008 // This is point-to-point interface.
#define IFF_MULTICAST       0x00000010 // Multicast is supported.

struct intrface {
    unsigned char name [INTFACENAMESIZE];
    unsigned char mac_addr[MAC_ADDR_STR_LEN];
    unsigned char ip_addr4[INET_ADDRSTRLEN];
    unsigned char ip_addr6[INET6_ADDRSTRLEN];
    unsigned char active;
};

int init_feature_extract_service();
static int feature_extract(pcap_t *handle, unsigned int ctx_idx);
int feature_extract_from_interface(char *device);
int feature_extract_from_pcap(char * f_pcap);

static int find_interface_in_list(char *name);
void print_interfaces(FILE *f_info, int num_ifs);
unsigned int interface_list_get();
pcap_t * open_pcap_device(char *device);
pcap_t * open_pcap_file(char *file_name);


#if (DEBUG_MEASURE_TIME == 1)
void test_joy_libpcap_process_packet(unsigned char *ctx_index, const struct pcap_pkthdr *header, const unsigned char *packet);
void task_joy_libpcap_process_packet(unsigned char *ctx_index, const struct pcap_pkthdr *header, const unsigned char *packet);

#endif

#endif