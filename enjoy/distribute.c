#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <arpa/inet.h>
#include    <sys/param.h>
#include    <stdio.h>
#include    <string.h>
#include    <unistd.h>

#include    "../include/error.h"
#include    "../include/enjoy.h"
#include    "../include/proto.h"
#include    "../include/feature.h"
#include    "../include/user.h"
#include    "../include/flow.h"
#include    "../include/config.h"
   
extern fnet_configuration_t fnet_glb_config;
extern user_list_t work_user_list;

int dsockfd;

void *
init_distribute_service(void * arg){
    struct sockaddr_in daddr;
    struct distribute_service_config *dsc = &(fnet_glb_config.distribute_s_cfg);
    dsockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == dsockfd){
        err_sys("socket error");
    }
    memset(&daddr, 0, sizeof daddr);

    // init endpoint info
    daddr.sin_family = AF_INET; 

    // get port from configuration struct 
    daddr.sin_port = htons(dsc->port);

    /* get sin_addr from configuration in two situations
     *     1. address = "anyaddr" 
     *     2. address = "*.*.*.*"
     */  

    // address = "anyaddr"
    if(!strcmp(dsc->address, IPv4_ANYADDR)){ 
        daddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    // normal IPv4 address
    else if(inet_pton(AF_INET, dsc->address, &(daddr.sin_addr)) == -1){
        err_sys("address error");
    }
    if(-1 == bind(dsockfd, (struct sockaddr *)&daddr, sizeof daddr)){
        err_sys("bind error");
        return ;
    }
    
    fprintf(stdout, "\nDistribution service initialized\n");
    distribute();
}

/*
 * 
 * 
 * 
 */
void distribute(){
    struct flow_record record;
    struct user *u;
    int msg_len;

    while(1){
        
        if(get_flow_record(&record) < 0){
            //err_msg("next_record error");
            continue;
        }
        pthread_rwlock_rdlock(&(work_user_list.rwlock));
        for(u = l_head(work_user_list); u != NULL; u = u->work_users.next){
            if(0 == user_features_match(record.fm, u->config->fm)){
                continue;
            }
            if((msg_len = construct_feature_msg(u, &record)) < 0){
                err_msg("construct feature message error");
                continue;
            }

            if(msg_len > MAX_UDP_MSG){
                err_msg("length of udp message beyond MAX_UDP_MSG");
                continue;
            }
            
            printf("Send a packet to %s:%d\n", inet_ntoa(((struct sockaddr_in*)(u->user_msghdr.msg_name))->sin_addr), ntohs(((struct sockaddr_in*)(u->user_msghdr.msg_name))->sin_port));

            sendmsg(dsockfd, &(u->user_msghdr), 0);
        }
        pthread_rwlock_unlock(&(work_user_list.rwlock));
        free_flow_record(&record);
    }
}

/*
 * This function works very much like recv in socket programming. If there is a new flow record
 * generated from joy, the function will return the flow record through the pointer argument, and 
 * its return value of 1 indicated success. if there is not a new flow record, the function 
 * will be blocked until a new flow record comes.
 */
int get_flow_record(struct flow_record *record){
    
    int len;
    
    char json_str[65535];

    fgets(json_str, 65535, stdin);
    //read(STDIN_FILENO, json_str, 1023);
    
    len = strlen(json_str);
    if(len <= 1){
        return -1;
    }
    if(len > 65535){
        err_msg("string error");
        return -1;
    }
   // puts(json_str);

    json_str[len-1] = '\0';
    init_flow_record(record);
    if(json_string2flow_record(record, json_str) < 0){
        return -1;
    }
    /*sleep(2);
    for (int i = 1; i <= NO_FEATURE; i++){
        if(record->features[i].flags == NONEMPTY){
            fprintf(stdout, "%d. \"%s\":%s, len:%d\n", i, record->features[i].name, record->features[i].value, record->features[i].val_len);
        }
    }*/

    return 0;
}

/*
*    0        8        16               32
 *    +--------+--------+----------------+
 *    |  type  | code   |     length     |
 *    +--------+--------+----------------+
 *    | focode |      folen     |  foval     feature options
 *    +--------+----------------+--------+
 *  return the length of feature message if everything goes well
 */

int construct_feature_msg(struct user *u, struct flow_record *record){
    int option_len = 0;
    int ft_count = 0;
    
#ifdef ENJOY_DEBUG
    if(0 == user_features_match(record->fm, u->config->fm)){
        err_msg("features of user not included in features of flow record");
        return -1;
    }
#endif

    for(int cd=0; cd < NO_FEATURE; cd++){
        if(get_fm(u->config->fm, cd) && get_fm(record->fm, cd)){
            if(record->features[cd].flags != NONEMPTY){
                err_msg("a not non-empty feature is filled into a feature message");
                return -1;
            }
        
            ((char *)(u->user_msghdr.msg_iov[ft_count * 2 + 1].iov_base))[0] = cd;
            *((short *)((char *)(u->user_msghdr.msg_iov[ft_count * 2 + 1].iov_base) + 1)) = (short)(record->features[cd].val_len);
            u->user_msghdr.msg_iov[ft_count * 2 + 2].iov_base = record->features[cd].value;
            u->user_msghdr.msg_iov[ft_count * 2 + 2].iov_len = record->features[cd].val_len;;
            option_len += record->features[cd].val_len + 3;
            ft_count++;
        }
    }

    ((char *)(u->user_msghdr.msg_iov[0].iov_base))[0] = FEATURE;
    ((char *)(u->user_msghdr.msg_iov[0].iov_base))[1] = GENERAL_CODE;
    *((short *)((char *)(u->user_msghdr.msg_iov[0].iov_base) + 2)) = option_len + 4;
    return option_len + 4;
}
/*
char * feature_msg(struct user_feature *user_feature, struct flow_record *record, int *msg_len){

#ifdef __ENJOY_DEBUG
    if(0 == user_features_match(record->fm, user_feature->fm)){
        err_msg("features of user not included in features of flow record");
        return NULL;
    }
#endif
    int flct = 0, fplen;
    char *msg;

    struct protocol fproto;
    fproto.type = FEATURE;
    fproto.code = GENERAL_CODE;
    
    struct feature_option *foptions = malloc(sizeof (struct feature_option) * user_feature->no_feature);
    if(NULL == foptions){
        err_msg("malloc error");
        return NULL;
    }
    for(int n=0; n < NO_FEATURE; n++){
        if(get_fm(user_feature->fm, n) && get_fm(record->fm, n)){
            if(record->features[n].flags != NONEMPTY){
                err_quit("a non-empty feature is filled into a feature message");
            }
            // when do I free the memory of value field?
            foptions[flct].code = record->features[n].code;
            foptions[flct].ft_data.length = strlen(record->features[n].value);
            foptions[flct].ft_data.value = record->features[n].value;
            ++flct;
        }
    }
    fproto.data =  feature_options2string(foptions, user_feature->no_feature, &fplen);
    if(NULL == fproto.data){
        return NULL;
    }
    *msg_len = fproto.proto_length = fplen + 4;
    return proto2str(&fproto);

}*/
/*
char * feature_options2string(struct feature_option *foptions, int no_foptions, int *options_str_len){
    int slct = 0;
    char *str;

    *options_str_len = 0;
    for(int i=0; i < no_foptions; i++){
        *options_str_len += 1; // len of feature option code field
        *options_str_len += 2; // len of feature option length field  
        *options_str_len += foptions[i].ft_data.length;
    }
    str = (char *)malloc(*options_str_len);
    if(NULL == str){
        err_msg("malloc error");
        return NULL;
    }

    for(int i=0; i < no_foptions; i++){
        *(str + slct) = foptions[i].code;

        // align exception?
        *((short *)(str + slct + 1)) = htons(foptions[i].ft_data.length);
        
        slct += sizeof foptions[i].code + sizeof foptions[i].ft_data.length;
        memcpy(str + slct, foptions[i].ft_data.value, foptions[i].ft_data.length);
        slct += foptions[i].ft_data.length;

    } 
    return str;
}

char * proto2str(struct protocol *proto){
    char *str;
    switch(proto->type){
        case FEATURE:
            str = (char *)malloc(proto->proto_length);
            if(NULL == str){
                err_msg("malloc error");
                return NULL;
            }
            *str = proto->type;
            *(str + 1) = proto->code;
            *(short*)(str + 2) = htons(proto->proto_length);
            memcpy(str + 4, proto->data, proto->proto_length - 4);
            // the memery copy operation can be solved by using multiple buffers in the sendmsg function. 
            free(proto->data);
            return str;
        default:
            err_msg("incorrect type");
            return NULL;
    }
    return NULL;
}

#ifdef __ENJOY_DEBUG
void feature_unit_test(){
    // test micro about feature_mask
    struct feature_mask fmt, fmt2;
    empty_fm(fmt);
    mask_fm(fmt, SA);
    mask_fm(fmt, DP);
    mask_fm(fmt, EXE);
    mask_fm(fmt, IP);
    printf("%#x", fmt.fm_low);
    if(fmt.fm_low != 0x80022){
        err_msg("fm_low error");
    }
    if(!get_fm(fmt, SA)){
         err_msg("get sa error");
    }
    if(get_fm(fmt, DA)){
         err_msg("get da error");
    }
    
    if(fmt.fm_mid != 0x10){
        err_msg("fm_mid error");
    }
    if(fmt.fm_high != 0x0){
        err_msg("fm_mid error");
    }
    fmt2 = fmt;
    mask_fm(fmt2, BYTES_IN);
    if(user_features_match(fmt2, fmt) == 0){
        err_msg("feature match error");
    }

    struct flow_record record; 
    struct user user;
    struct user_feature uf;
    int msg_len;

    empty_fm(uf.fm);
    // construct a user
    mask_fm(uf.fm, SA);
    mask_fm(uf.fm, DA);
    mask_fm(uf.fm, DP);
    mask_fm(uf.fm, BYTES_IN);
    mask_fm(uf.fm, PAYLOAD);

    uf.no_feature = 5;
    user.config = &uf;

    // construct a flow record
    char *json_str1 = "{\"sa\":\"192.168.1.85\",\"da\":\"158.130.5.201\",\"pr\":6,\"sp\":54006,\"dp\":22,\"bytes_out\":946,\"num_pkts_out\":4,\"bytes_in\":1489,\"num_pkts_in\":5,\"time_start\":1497548878.438211,\"time_end\":1497548878.573496,\"packets\":[{\"b\":762,\"dir\":\">\",\"ipt\":0},{\"b\":41,\"dir\":\"<\",\"ipt\":28},{\"b\":1064,\"dir\":\"<\",\"ipt\":24},{\"b\":48,\"dir\":\">\",\"ipt\":0},{\"b\":280,\"dir\":\"<\",\"ipt\":28},{\"b\":16,\"dir\":\">\",\"ipt\":2},{\"b\":120,\"dir\":\">\",\"ipt\":0},{\"b\":52,\"dir\":\"<\",\"ipt\":26},{\"b\":52,\"dir\":\"<\",\"ipt\":24}],\"byte_dist\":[157,3,4,1,6,2,5,1,2,2,6,0,6,3,5,3,5,2,4,4,4,4,2,1,1,0,3,2,0,0,4,4,5,3,0,2,4,6,2,1,0,3,1,1,83,146,36,4,8,55,91,10,13,54,36,3,25,10,1,3,3,1,1,2,29,3,1,2,2,1,5,4,5,3,6,2,5,3,1,1,1,3,2,9,0,7,2,2,3,1,2,3,3,3,3,4,3,119,24,118,29,100,23,17,120,43,4,4,27,81,59,74,51,2,33,162,47,26,3,7,4,7,7,4,5,2,1,2,2,2,6,0,1,1,1,11,5,1,1,1,6,6,2,5,3,2,3,0,0,2,1,0,1,2,1,1,2,1,0,2,0,4,0,3,1,0,2,4,2,4,0,1,4,1,1,0,3,2,3,0,1,0,1,1,2,1,2,1,0,1,0,0,2,0,1,4,1,2,2,1,2,2,3,4,4,2,0,1,2,0,1,1,4,6,1,1,1,2,2,2,3,2,3,0,1,0,3,0,2,2,2,3,4,1,5,2,0,2,0,3,3,2,3,5,3,8,1,1,1,4,1,2,1,0,3,0],\"byte_dist_mean\":88.828747,\"byte_dist_std\":36.470586,\"entropy\":5.957095,\"total_entropy\":14505.526681,\"p_malware\":0.002029,\"ip\":{\"out\":{\"ttl\":64,\"id\":[2196,2121,2819,5190]},\"in\":{\"ttl\":48,\"id\":[2206,2207,2208,2210,2211]}},\"ssh\":{\"cli\":{\"protocol\":\"SSH-2.0-dropbear_2017.75\",\"cookie\":\"404b87b069b089cb5b2b484029557e5e\",\"kex_algos\":\"curve25519-sha256@libssh.org,ecdh-sha2-nistp521,ecdh-sha2-nistp384,ecdh-sha2-nistp256,diffie-hellman-group14-sha1,diffie-hellman-group1-sha1,kexguess2@matt.ucc.asn.au\",\"s_host_key_algos\":\"ecdsa-sha2-nistp256,ecdsa-sha2-nistp384,ecdsa-sha2-nistp521,ssh-rsa,ssh-dss\",\"c_encryption_algos\":\"aes128-ctr,aes256-ctr,aes128-cbc,aes256-cbc,twofish256-cbc,twofish-cbc,twofish128-cbc,3des-ctr,3des-cbc\",\"s_encryption_algos\":\"aes128-ctr,aes256-ctr,aes128-cbc,aes256-cbc,twofish256-cbc,twofish-cbc,twofish128-cbc,3des-ctr,3des-cbc\",\"c_mac_algos\":\"hmac-sha1-96,hmac-sha1,hmac-sha2-256,hmac-sha2-512,hmac-md5\",\"s_mac_algos\":\"hmac-sha1-96,hmac-sha1,hmac-sha2-256,hmac-sha2-512,hmac-md5\",\"c_comp_algos\":\"zlib@openssh.com,zlib,none\",\"s_comp_algos\":\"zlib@openssh.com,zlib,none\",\"c_languages\":\"\",\"s_languages\":\"\",\"kex_algo\":\"curve25519-sha256@libssh.org\",\"c_kex\":\"34877d10cb2555ea557212f9e753a1a32ff238d5ef65accae887dc4666044c4a\",\"newkeys\":\"true\",\"unencrypted\":3},\"srv\":{\"protocol\":\"SSH-2.0-OpenSSH_7.3p1 Ubuntu-1ubuntu0.1\",\"cookie\":\"38b914cd2d88997c7910f41025061082\",\"kex_algos\":\"curve25519-sha256@libssh.org,ecdh-sha2-nistp256,ecdh-sha2-nistp384,ecdh-sha2-nistp521,diffie-hellman-group-exchange-sha256,diffie-hellman-group16-sha512,diffie-hellman-group18-sha512,diffie-hellman-group14-sha256,diffie-hellman-group14-sha1\",\"s_host_key_algos\":\"ssh-rsa,rsa-sha2-512,rsa-sha2-256,ecdsa-sha2-nistp256,ssh-ed25519\",\"c_encryption_algos\":\"chacha20-poly1305@openssh.com,aes128-ctr,aes192-ctr,aes256-ctr,aes128-gcm@openssh.com,aes256-gcm@openssh.com\",\"s_encryption_algos\":\"chacha20-poly1305@openssh.com,aes128-ctr,aes192-ctr,aes256-ctr,aes128-gcm@openssh.com,aes256-gcm@openssh.com\",\"c_mac_algos\":\"umac-64-etm@openssh.com,umac-128-etm@openssh.com,hmac-sha2-256-etm@openssh.com,hmac-sha2-512-etm@openssh.com,hmac-sha1-etm@openssh.com,umac-64@openssh.com,umac-128@openssh.com,hmac-sha2-256,hmac-sha2-512,hmac-sha1\",\"s_mac_algos\":\"umac-64-etm@openssh.com,umac-128-etm@openssh.com,hmac-sha2-256-etm@openssh.com,hmac-sha2-512-etm@openssh.com,hmac-sha1-etm@openssh.com,umac-64@openssh.com,umac-128@openssh.com,hmac-sha2-256,hmac-sha2-512,hmac-sha1\",\"c_comp_algos\":\"none,zlib@openssh.com\",\"s_comp_algos\":\"none,zlib@openssh.com\",\"c_languages\":\"\",\"s_languages\":\"\",\"s_hostkey_type\":\"ecdsa-sha2-nistp256\",\"s_hostkey\":\"0000001365636473612d736861322d6e69737470323536000000086e697374703235360000004104907c3d7b7c5ab2d0870981cc79d0f53c8d567b600eb0e582a92f49875158f9dd51425a799cdeef3b72f54463f547b49003c91ecd13061f23163578f523b1c695\",\"s_signature_type\":\"ecdsa-sha2-nistp256\",\"s_signature\":\"0000001365636473612d736861322d6e69737470323536000000490000002100cc52eb186a4a0f24824e9543744c8de9f55d38b6ed8f2e6de84109c8d46a98340000002057f40e438511356f4511e68c1f707b529a0e57d644d54af0769cc3b23e5ddbf2\",\"kex_algo\":\"curve25519-sha256@libssh.org\",\"s_kex\":\"71295107f7507c7ffad98c31752512de2e968fa665c3a4f0e726a9feea600221\",\"newkeys\":\"true\",\"unencrypted\":3}},\"payload\":{\"out\":\"5353482d322e302d64726f70626561725f323031372e37350d0a000002ac0414\",\"in\":\"5353482d322e302d4f70656e5353485f372e337031205562756e74752d317562\"},\"oseq\":[2767029945,762,48,16],\"oack\":[949335929,1105,280,0],\"iseq\":[949335929,41,1064,280,52],\"iack\":[2767030707,0,48,136,0],\"ppi\":[{\"seq\":2767029945,\"ack\":949335929,\"rseq\":0,\"rack\":0,\"b\":762,\"olen\":12,\"dir\":\">\",\"t\":0,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":854270487,\"ecr\":195739614}}]},{\"seq\":949335929,\"ack\":2767030707,\"rseq\":0,\"rack\":762,\"b\":41,\"olen\":12,\"dir\":\"<\",\"t\":28,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":195739621,\"ecr\":854270487}}]},{\"seq\":949335970,\"ack\":2767030707,\"rseq\":41,\"rack\":762,\"b\":1064,\"olen\":12,\"dir\":\"<\",\"t\":52,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":195739628,\"ecr\":854270514}}]},{\"seq\":2767030707,\"ack\":949337034,\"rseq\":762,\"rack\":1064,\"b\":48,\"olen\":12,\"dir\":\">\",\"t\":53,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":854270538,\"ecr\":195739628}}]},{\"seq\":949337034,\"ack\":2767030755,\"rseq\":1064,\"rack\":48,\"b\":280,\"olen\":12,\"dir\":\"<\",\"t\":81,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":195739635,\"ecr\":854270538}}]},{\"seq\":2767030755,\"ack\":949337314,\"rseq\":48,\"rack\":280,\"b\":16,\"olen\":12,\"dir\":\">\",\"t\":84,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":854270568,\"ecr\":195739635}}]},{\"seq\":2767030771,\"ack\":949337314,\"rseq\":16,\"rack\":280,\"b\":120,\"olen\":12,\"dir\":\">\",\"t\":84,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":854270568,\"ecr\":195739635}}]},{\"seq\":949337314,\"ack\":2767030891,\"rseq\":280,\"rack\":120,\"b\":52,\"olen\":12,\"dir\":\"<\",\"t\":111,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":195739642,\"ecr\":854270568}}]},{\"seq\":949337366,\"ack\":2767030891,\"rseq\":52,\"rack\":120,\"b\":52,\"olen\":12,\"dir\":\"<\",\"t\":135,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":195739648,\"ecr\":854270593}}]}],\"hd\":{\"n\":4,\"cm\":\"24\",\"cv\":\"00\",\"sm\":\"00\",\"i\":\"53\"},\"idp_out\":\"4512032e089440004006c8dbc0a801559e8205c9d2f60016a4ed86b93895b77980181015a98e00000101080a32eb22170baabfde5353482d322e302d64726f70626561725f323031372e37350d0a000002ac0414404b87b069b089cb5b2b484029557e5e000000a6637572766532353531392d736861323536406c69627373682e6f72672c656364682d736861322d6e697374703532312c656364682d736861322d6e697374703338342c656364682d736861322d6e697374703235362c6469666669652d68656c6c6d616e2d67726f757031342d736861312c6469666669652d68656c6c6d616e2d67726f7570312d736861312c6b6578677565737332406d6174742e7563632e61736e2e61750000004b65636473612d736861322d6e697374703235362c65636473612d736861322d6e697374703338342c65636473612d736861322d6e697374703532312c7373682d7273612c7373682d647373000000676165733132382d6374722c6165733235362d6374722c6165733132382d6362632c6165733235362d6362632c74776f666973683235362d6362632c74776f666973682d6362632c74776f666973683132382d6362632c336465732d6374722c336465732d636263000000676165733132382d6374722c6165733235362d6374722c6165733132382d6362632c6165733235362d6362632c74776f666973683235362d6362632c74776f666973682d6362632c74776f666973683132382d6362632c336465732d6374722c336465732d6362630000003b686d61632d736861312d39362c686d61632d736861312c686d61632d736861322d3235362c686d61632d736861322d3531322c686d61632d6d64350000003b686d61632d736861312d39362c686d61632d736861312c686d61632d736861322d3235362c686d61632d736861322d3531322c686d61632d6d64350000001a7a6c6962406f70656e7373682e636f6d2c7a6c69622c6e6f6e650000001a7a6c6962406f70656e7373682e636f6d2c7a6c69622c6e6f6e6500000000000000000100000000da8ff56e0000002c061e0000002034877d10cb2555ea557212f9e753a1a32ff238d5ef65accae887dc4666044c4ad4770cc862c5\",\"idp_len_out\":814,\"idp_in\":\"4502005d089e40003006dbb29e8205c9c0a801550016d2f63895b779a4ed89b3801800dedbf100000101080a0baabfe532eb22175353482d322e302d4f70656e5353485f372e337031205562756e74752d317562756e7475302e310d0a\",\"idp_len_in\":93}";
    init_flow_record(&record);
    json_string2flow_record(&record, json_str1);
    fprintf(stdout, "flow record features are following:\n");
    for (int i = 1; i <= NO_FEATURE; i++){
        if(record.features[i].flags == NONEMPTY){
            fprintf(stdout, "\"%s\":%s\n", record.features[i].name, record.features[i].value);
        }
    }
    if(user_features_match(record.fm, user.config->fm) == 0){
        err_msg("feature match error");
    }

    // test for feature_msg
    char * msg;
    msg = feature_msg(user.config, &record, &msg_len);
    printf("message len %d\n", msg_len);
    printf("hex of message\n");
    for(int i=0; i<msg_len; i++){
        printf("%#x[%c] ", (unsigned int)msg[i], msg[i] > ' ' ? msg[i] : ' ');
    }
}
#endif*/
