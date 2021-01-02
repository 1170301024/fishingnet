
#include    "../include/nflog.h"
#include    <netinet/in.h>
#include    <string.h>
#include    <time.h>
#include    <stdio.h>
#include    <unistd.h>
#include    <stdlib.h>

#include    "../include/enjoy.h"
#include    "../include/joy/joy_api.h"
#include    "../include/joy/joy_api_private.h"
#include    "../include/feature_extract.h"   

int 
parser_nflog(const unsigned char *packet, struct nflog *nflog, int * payload_offset){
    int offset = 0;
    unsigned short tlv_len, tlv_type;

    memset(nflog, 0x00, sizeof (struct nflog));
    nflog->hdr = *(nflog_hdr_t *)(packet);
    offset += sizeof (nflog_hdr_t);
    
    do{
        tlv_len = *((short*)(packet + offset));
        offset += 2;
        tlv_type = *((short*)(packet + offset));
        nflog->tlvs[tlv_type].tlv_length=tlv_len;
        nflog->tlvs[tlv_type].tlv_type=tlv_type;
        offset += tlv_len - 2;
        if(tlv_type == NFULA_PAYLOAD){
            break;
        }
        offset = ((offset + 3) / 4) * 4;
        
    }while(1);
    *payload_offset = offset - tlv_len + 4;
    return 0;
}

/*
struct ethernet pad_linker = {
    .dst = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    .src = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .type = {0x08, 0x00}
};

void task_joy_libpcap_process_packet(unsigned char *ctx_index,
                        const struct pcap_pkthdr *header,
                        const unsigned char *packet)
{
   

    uint64_t index = 0;
    joy_ctx_data *ctx = NULL;    
    struct nflog nflog;
    struct ethernet * linker_addr;
    int payload_offset;

    parser_nflog(packet, &nflog, &payload_offset);
    
    int payload_tlv_len = nflog.tlvs[NFULA_PAYLOAD].tlv_length;
    if(payload_tlv_len == 0){
        err_quit("nflog payload tlv error");  
    }
    linker_addr = (struct ethernet *)(packet + payload_offset - sizeof (struct ethernet));
    
    if(*(packet + payload_offset) != 0x45){
        fprintf(stderr, "caplen:%d\n", header->caplen);
        for(int i=0; i<header->caplen; i++){
            fprintf(stderr, "%#x ", *(packet + i));
        }
        fprintf(stderr, "\n");
        for(int i=0; i<header->caplen; i++){
            fprintf(stderr, "%#x ", *((unsigned char *)linker_addr + i));
        }
        fprintf(stderr, "\n");
        getchar();
    }
    
    *linker_addr = pad_linker;
    struct pcap_pkthdr * new_header = (struct pcap_pkthdr *)malloc(sizeof (struct pcap_pkthdr));
    new_header->caplen = payload_tlv_len + 2;
    new_header->len = new_header->caplen;
    new_header->ts = header->ts;

    unsigned char * new_packet = (unsigned char *)(linker_addr);
    /* make sure we have a packet to process */
/*    if (packet == NULL) {
        return;
    }
    /* ctx_index has the int value of the data context
     * This number is between 0 and max configured contexts
     */
 /*   index = (uint64_t)ctx_index;

    ctx = joy_index_to_context(index);
    process_packet((unsigned char*)ctx, new_header, new_packet);
    free(new_header);
}*/
