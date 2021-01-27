#ifndef _FEATURE_H
#define _FEATURE_H

struct feature{
    unsigned char ft_code;
    unsigned short ft_len; // length of feature string
    char * ft_val;  // feature string;
};

struct feature_set{
    int no_ft;
    unsigned char f_feature[NO_FEATURE + 1];
    struct feature *features[NO_FEATURE + 1];
};

#define empty_feature_set(fsp)   do{      \
                                    for(int i=0; i<=NO_FEATURE; i++){ \
                                        fsp->f_feature[i] = 0; \
                                        fsp->features[i] = NULL; \
                                    }   \
                                    fsp->no_ft = 0;   \
                                }while(0);


typedef void (*feature_handler)(const unsigned char *, struct feature_set*);


void *init_distribute_service(void *);

void distribute();
int get_flow_record(struct flow_record *record);
int construct_feature_msg(struct user *u, struct flow_record *record);

char * feature_msg(struct user_feature *user_feature, struct flow_record *record, int *msg_len);
char * feature_options2string(struct feature_option *foptions, int no_foptions, int * options_str_len);
char * proto2str(struct protocol *proto);

#endif