#ifndef _FEATURE_H
#define _FEATURE_H

extern int no_user;
extern struct user *user_list;
extern struct user *user_list_tail;


#define low_feature_match(rm, um) ((((rm).fm_low) & ((um).fm_low)) == ((um).fm_low))
#define mid_feature_match(rm, um) ((((rm).fm_mid) & ((um).fm_mid)) == ((um).fm_mid))
#define high_feature_match(rm, um) ((((rm).fm_high) & ((um).fm_high)) == ((um).fm_high))
// true if the features of user included in the features of flow record 
#define user_features_match(rm, um) (low_feature_match(rm, um) && mid_feature_match(rm, um) && high_feature_match(rm, um))

void *init_distribute_service(void *);
void *shutdown_distribute_service();

void distribute();
int get_flow_record(struct flow_record *record);
int construct_feature_msg(struct user *u, struct flow_record *record);

char * feature_msg(struct user_feature *user_feature, struct flow_record *record, int *msg_len);
char * feature_options2string(struct feature_option *foptions, int no_foptions, int * options_str_len);
char * proto2str(struct protocol *proto);

#endif