
#ifndef _FLOW_H
#define _FLOW_H

/*
  all feature codes in flow record 
*/
#define SA      1
#define DA      2
#define PR      3
#define SP      4
#define DP      5
#define BYTES_OUT       6
#define NUM_PKTS_OUT        7
#define BYTES_IN        8
#define NUM_PKTS_IN         9
#define TIME_START      10
#define TIME_END        11
#define PACKETS     12
#define BYTE_DIST      13
#define BYTE_DIST_MEAN     14
#define BYTE_DIST_STD      15
#define ENTROPY     16
#define TOTAL_ENTROPY       17
#define P_MALWARE       18
#define IP      19
#define TCP     20
#define OSEQ        21
#define OACK        22
#define ISEQ        23
#define IACK        24
#define PPI     25
#define FINTERPRINTS       26
#define WHT     27
#define DNS     28
#define SSH     29
#define TLS     30
#define DHCP        31
#define DHCPV6      32
#define HTTP        33
#define IKE     34
#define PAYLOAD     35
#define EXE     36
#define HD      37
#define PROBABLE_OS     38
#define IDP_OUT     39
#define IDP_LEN_OUT     40
#define IDP_IN      41
#define IDP_LEN_IN      42
#define DEBUG       43
#define EXPIRE_TYPE     44

#define NO_FEATURE 44

#define NONEMPTY 0
#define EMPYT 1
#define RESERVED 2

#define empty_feature(x) (x.flags = EMPTY)

struct feature_mask{
    unsigned int fm_low;
    unsigned int fm_mid;
    unsigned int fm_high;
};
/*
 * before invoking the micro you should check the code of the feature so that the value of it is not
 * beyond NO_FEATURE
 */
#define get_fm(f, c) (((int *)(&f))[c/32] & (1 << (c % 32u)))

#define mask_fm(f, c) (((int *)(&f))[c/32] |= (1 << (c % 32u)))

#define delete_fm(f, c) (((int *)(&f))[c/32] ^= (1 << c % 32u))

#define empty_fm(f) f.fm_low = 0; f.fm_mid = 0; f.fm_high = 0

struct feature{
  int flags;
  int code;
  char * name;
  char * value;
  short val_len;  
};

struct flow_record{
  /* number of features in a flow record*/
  int no_feature;

  struct feature_mask fm;
  
  struct feature features[NO_FEATURE+1];


};


void init_flow_record(struct flow_record *record);

void free_flow_record(struct flow_record *record);

int json_string2flow_record(struct flow_record *flow_record, char *str);

void flow_record2json_string(struct flow_record *flow_record, char **str);

int feature_code(char *feature);

#endif


