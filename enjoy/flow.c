#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/flow.h"
#include "../include/test.h"
#include "../include/error.h"

static const char *features[NO_FEATURE+1] = {
    [0]= NULL, [1]="sa", [2]="da", [3]="pr",
    [4]="sp", [5]="dp", [6]="bytes_out", [7]="num_pkts_out", 
    [8]="bytes_in", [9]="num_pkts_in", [10]="time_start", [11]="time_end",
    [12]="packets", [13]="byte_dist", [14]="byte_dist_mean", [15]="byte_dist_std",
    [16]="entropy", [17]="total_entropy", [18]="p_malware", [19]="ip", 
    [20]="tcp", [21]="oseq", [22]="oack", [23]="iseq",
    [24]="iack", [25]="ppi", [26]="fingerprints", [27]="wht",
    [28]="dns", [29]="ssh", [30]="tls", [31]="dhcp",
    [32]="dhcpv6", [33]="http", [34]="ike", [35]="payload",
    [36]="exe", [37]="hd", [38]="probable_os", [39]="idp_out",
    [40]="idp_len_out", [41]="idp_in", [42]="idp_len_in", [43]="debug",
    [44]="expire_type"
};

/*
 * 
 */
void 
init_flow_record(struct flow_record *record){
    if (NULL == record)
    {
        fprintf(stderr, "flow record pointer is null");
        return ;
    }
    
    record->no_feature = 0;
    record->features[0].flags = RESERVED;
    for (int i=1; i <= NO_FEATURE; i++){
        record->features[i].flags = EMPYT;
        record->features[i].name = NULL;
        record->features[i].value = NULL;
    }
    return ; 
}

void 
free_flow_record(struct flow_record *record){
    if (NULL == record)
    {
        fprintf(stderr, "flow record pointer is null");
        return ;
    }
    for (int i=0; i<=NO_FEATURE; i++){
        if(record->features[1].flags == NONEMPTY){
            free(record->features[i].name);
            free(record->features[i].value);
        }
    }
}

/*
 * Convert a flow record string by joy into a flow record struct, the flow
 * record string is in standord JSON format, and there is no white space
 * in the flow record string. 
 * 
 */
int
json_string2flow_record(struct flow_record *flow_record, char *str){
    int deep_count = 0;
    char value_border_start, value_border_end;
    char *name = NULL, *value = NULL, *str_init = str;
    int i, name_len, value_len, lct = 0, feature_idx;

    if(str == NULL){
        err_msg("the json string is null");
        return;
    }

    if(*str != '{'){
        goto error;
    }
    str++;
    deep_count++;
    do{
        if(*str != '\"'){
            lct = str - str_init;
            goto error;
        }
        name_len = 0;
        while(*(str + name_len + 1) != '\"'){
            name_len++;
        }
        name = (char *)malloc(name_len + 1);
        if(name == NULL){
            err_msg("malloc error");
            return -1;
        }
        strncpy(name, str + 1, name_len);
        name[name_len] = '\0';
        str += name_len + 2;
        
        if(*str != ':'){
            lct = str - str_init + 1;
            goto error;
        }
        str++;

        value_len = 1;
        switch(*str){
            // json string 
            case '\"':
                while(1){
                    if(*(str + value_len) == '\"' && *(str + value_len - 1) != '\\'){
                        break;
                    }
                    value_len++;
                }
                break;
            // json object
            case '{':
                
            // json array
            case '[':
                if(*str =='{'){
                    value_border_start = '{';
                    value_border_end = '}';
                }
                else{
                    value_border_start = '[';
                    value_border_end = ']';
                }
                while(1){
                    if(*(str + value_len) == value_border_start){
                        deep_count++;
                    }
                    if(*(str + value_len) == value_border_end && deep_count == 1){
                        break;
                    }
                    else if(*(str + value_len) == value_border_end){
                        deep_count--;
                    }
                    value_len++;
                }
                break;
            // json number
            default:
                while(1){
                    if(*(str + value_len) == ',' || *(str + value_len) == '}'){
                        value_len--;
                        break;
                    }
                    value_len++;
                }
        }   
        value_len++;
        value = (char *)malloc(value_len + 1);
        strncpy(value, str, value_len);
        value[value_len] = '\0';
        feature_idx = feature_code(name);
        if (feature_idx > 0){
            mask_fm(flow_record->fm, feature_idx);
            flow_record->features[feature_idx].flags = NONEMPTY;
            flow_record->features[feature_idx].code = feature_idx;
            flow_record->features[feature_idx].name = name;
            flow_record->features[feature_idx].value = value;
            flow_record->features[feature_idx].val_len = value_len;
            flow_record->no_feature++;
        }
        else{
            goto error;
        }
        str += value_len + 1; 
    }while(*str != '\0');
    
    return 0;

error:
    free_flow_record(flow_record);
    free(value);
    free(name);
    err_msg("flow record json string:%d:string:%s\n\t| %s", lct, str_init, str_init+lct);
    return -1;

}
/*
void init_feature(){
    
}
*/
char * feature_name(int code){
    if (code <= 0 || code > NO_FEATURE){
        return NULL;
    }
    return features[code];
}

int feature_code(char *name){
    if(name == NULL){
        return -1;
    }
    for(int i = 1; i <= NO_FEATURE; i++){
        if (!strcmp(name, features[i])){
            return i;
        }
    }
    return -1;
}


void test_json_string2flow_record(){
    char *json_str1 = "{\"sa\":\"192.168.1.85\",\"da\":\"158.130.5.201\",\"pr\":6,\"sp\":54006,\"dp\":22,\"bytes_out\":946,\"num_pkts_out\":4,\"bytes_in\":1489,\"num_pkts_in\":5,\"time_start\":1497548878.438211,\"time_end\":1497548878.573496,\"packets\":[{\"b\":762,\"dir\":\">\",\"ipt\":0},{\"b\":41,\"dir\":\"<\",\"ipt\":28},{\"b\":1064,\"dir\":\"<\",\"ipt\":24},{\"b\":48,\"dir\":\">\",\"ipt\":0},{\"b\":280,\"dir\":\"<\",\"ipt\":28},{\"b\":16,\"dir\":\">\",\"ipt\":2},{\"b\":120,\"dir\":\">\",\"ipt\":0},{\"b\":52,\"dir\":\"<\",\"ipt\":26},{\"b\":52,\"dir\":\"<\",\"ipt\":24}],\"byte_dist\":[157,3,4,1,6,2,5,1,2,2,6,0,6,3,5,3,5,2,4,4,4,4,2,1,1,0,3,2,0,0,4,4,5,3,0,2,4,6,2,1,0,3,1,1,83,146,36,4,8,55,91,10,13,54,36,3,25,10,1,3,3,1,1,2,29,3,1,2,2,1,5,4,5,3,6,2,5,3,1,1,1,3,2,9,0,7,2,2,3,1,2,3,3,3,3,4,3,119,24,118,29,100,23,17,120,43,4,4,27,81,59,74,51,2,33,162,47,26,3,7,4,7,7,4,5,2,1,2,2,2,6,0,1,1,1,11,5,1,1,1,6,6,2,5,3,2,3,0,0,2,1,0,1,2,1,1,2,1,0,2,0,4,0,3,1,0,2,4,2,4,0,1,4,1,1,0,3,2,3,0,1,0,1,1,2,1,2,1,0,1,0,0,2,0,1,4,1,2,2,1,2,2,3,4,4,2,0,1,2,0,1,1,4,6,1,1,1,2,2,2,3,2,3,0,1,0,3,0,2,2,2,3,4,1,5,2,0,2,0,3,3,2,3,5,3,8,1,1,1,4,1,2,1,0,3,0],\"byte_dist_mean\":88.828747,\"byte_dist_std\":36.470586,\"entropy\":5.957095,\"total_entropy\":14505.526681,\"p_malware\":0.002029,\"ip\":{\"out\":{\"ttl\":64,\"id\":[2196,2121,2819,5190]},\"in\":{\"ttl\":48,\"id\":[2206,2207,2208,2210,2211]}},\"ssh\":{\"cli\":{\"protocol\":\"SSH-2.0-dropbear_2017.75\",\"cookie\":\"404b87b069b089cb5b2b484029557e5e\",\"kex_algos\":\"curve25519-sha256@libssh.org,ecdh-sha2-nistp521,ecdh-sha2-nistp384,ecdh-sha2-nistp256,diffie-hellman-group14-sha1,diffie-hellman-group1-sha1,kexguess2@matt.ucc.asn.au\",\"s_host_key_algos\":\"ecdsa-sha2-nistp256,ecdsa-sha2-nistp384,ecdsa-sha2-nistp521,ssh-rsa,ssh-dss\",\"c_encryption_algos\":\"aes128-ctr,aes256-ctr,aes128-cbc,aes256-cbc,twofish256-cbc,twofish-cbc,twofish128-cbc,3des-ctr,3des-cbc\",\"s_encryption_algos\":\"aes128-ctr,aes256-ctr,aes128-cbc,aes256-cbc,twofish256-cbc,twofish-cbc,twofish128-cbc,3des-ctr,3des-cbc\",\"c_mac_algos\":\"hmac-sha1-96,hmac-sha1,hmac-sha2-256,hmac-sha2-512,hmac-md5\",\"s_mac_algos\":\"hmac-sha1-96,hmac-sha1,hmac-sha2-256,hmac-sha2-512,hmac-md5\",\"c_comp_algos\":\"zlib@openssh.com,zlib,none\",\"s_comp_algos\":\"zlib@openssh.com,zlib,none\",\"c_languages\":\"\",\"s_languages\":\"\",\"kex_algo\":\"curve25519-sha256@libssh.org\",\"c_kex\":\"34877d10cb2555ea557212f9e753a1a32ff238d5ef65accae887dc4666044c4a\",\"newkeys\":\"true\",\"unencrypted\":3},\"srv\":{\"protocol\":\"SSH-2.0-OpenSSH_7.3p1 Ubuntu-1ubuntu0.1\",\"cookie\":\"38b914cd2d88997c7910f41025061082\",\"kex_algos\":\"curve25519-sha256@libssh.org,ecdh-sha2-nistp256,ecdh-sha2-nistp384,ecdh-sha2-nistp521,diffie-hellman-group-exchange-sha256,diffie-hellman-group16-sha512,diffie-hellman-group18-sha512,diffie-hellman-group14-sha256,diffie-hellman-group14-sha1\",\"s_host_key_algos\":\"ssh-rsa,rsa-sha2-512,rsa-sha2-256,ecdsa-sha2-nistp256,ssh-ed25519\",\"c_encryption_algos\":\"chacha20-poly1305@openssh.com,aes128-ctr,aes192-ctr,aes256-ctr,aes128-gcm@openssh.com,aes256-gcm@openssh.com\",\"s_encryption_algos\":\"chacha20-poly1305@openssh.com,aes128-ctr,aes192-ctr,aes256-ctr,aes128-gcm@openssh.com,aes256-gcm@openssh.com\",\"c_mac_algos\":\"umac-64-etm@openssh.com,umac-128-etm@openssh.com,hmac-sha2-256-etm@openssh.com,hmac-sha2-512-etm@openssh.com,hmac-sha1-etm@openssh.com,umac-64@openssh.com,umac-128@openssh.com,hmac-sha2-256,hmac-sha2-512,hmac-sha1\",\"s_mac_algos\":\"umac-64-etm@openssh.com,umac-128-etm@openssh.com,hmac-sha2-256-etm@openssh.com,hmac-sha2-512-etm@openssh.com,hmac-sha1-etm@openssh.com,umac-64@openssh.com,umac-128@openssh.com,hmac-sha2-256,hmac-sha2-512,hmac-sha1\",\"c_comp_algos\":\"none,zlib@openssh.com\",\"s_comp_algos\":\"none,zlib@openssh.com\",\"c_languages\":\"\",\"s_languages\":\"\",\"s_hostkey_type\":\"ecdsa-sha2-nistp256\",\"s_hostkey\":\"0000001365636473612d736861322d6e69737470323536000000086e697374703235360000004104907c3d7b7c5ab2d0870981cc79d0f53c8d567b600eb0e582a92f49875158f9dd51425a799cdeef3b72f54463f547b49003c91ecd13061f23163578f523b1c695\",\"s_signature_type\":\"ecdsa-sha2-nistp256\",\"s_signature\":\"0000001365636473612d736861322d6e69737470323536000000490000002100cc52eb186a4a0f24824e9543744c8de9f55d38b6ed8f2e6de84109c8d46a98340000002057f40e438511356f4511e68c1f707b529a0e57d644d54af0769cc3b23e5ddbf2\",\"kex_algo\":\"curve25519-sha256@libssh.org\",\"s_kex\":\"71295107f7507c7ffad98c31752512de2e968fa665c3a4f0e726a9feea600221\",\"newkeys\":\"true\",\"unencrypted\":3}},\"payload\":{\"out\":\"5353482d322e302d64726f70626561725f323031372e37350d0a000002ac0414\",\"in\":\"5353482d322e302d4f70656e5353485f372e337031205562756e74752d317562\"},\"oseq\":[2767029945,762,48,16],\"oack\":[949335929,1105,280,0],\"iseq\":[949335929,41,1064,280,52],\"iack\":[2767030707,0,48,136,0],\"ppi\":[{\"seq\":2767029945,\"ack\":949335929,\"rseq\":0,\"rack\":0,\"b\":762,\"olen\":12,\"dir\":\">\",\"t\":0,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":854270487,\"ecr\":195739614}}]},{\"seq\":949335929,\"ack\":2767030707,\"rseq\":0,\"rack\":762,\"b\":41,\"olen\":12,\"dir\":\"<\",\"t\":28,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":195739621,\"ecr\":854270487}}]},{\"seq\":949335970,\"ack\":2767030707,\"rseq\":41,\"rack\":762,\"b\":1064,\"olen\":12,\"dir\":\"<\",\"t\":52,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":195739628,\"ecr\":854270514}}]},{\"seq\":2767030707,\"ack\":949337034,\"rseq\":762,\"rack\":1064,\"b\":48,\"olen\":12,\"dir\":\">\",\"t\":53,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":854270538,\"ecr\":195739628}}]},{\"seq\":949337034,\"ack\":2767030755,\"rseq\":1064,\"rack\":48,\"b\":280,\"olen\":12,\"dir\":\"<\",\"t\":81,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":195739635,\"ecr\":854270538}}]},{\"seq\":2767030755,\"ack\":949337314,\"rseq\":48,\"rack\":280,\"b\":16,\"olen\":12,\"dir\":\">\",\"t\":84,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":854270568,\"ecr\":195739635}}]},{\"seq\":2767030771,\"ack\":949337314,\"rseq\":16,\"rack\":280,\"b\":120,\"olen\":12,\"dir\":\">\",\"t\":84,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":854270568,\"ecr\":195739635}}]},{\"seq\":949337314,\"ack\":2767030891,\"rseq\":280,\"rack\":120,\"b\":52,\"olen\":12,\"dir\":\"<\",\"t\":111,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":195739642,\"ecr\":854270568}}]},{\"seq\":949337366,\"ack\":2767030891,\"rseq\":52,\"rack\":120,\"b\":52,\"olen\":12,\"dir\":\"<\",\"t\":135,\"flags\":\"PA\",\"opts\":[{\"noop\":null},{\"noop\":null},{\"ts\":{\"val\":195739648,\"ecr\":854270593}}]}],\"hd\":{\"n\":4,\"cm\":\"24\",\"cv\":\"00\",\"sm\":\"00\",\"i\":\"53\"},\"idp_out\":\"4512032e089440004006c8dbc0a801559e8205c9d2f60016a4ed86b93895b77980181015a98e00000101080a32eb22170baabfde5353482d322e302d64726f70626561725f323031372e37350d0a000002ac0414404b87b069b089cb5b2b484029557e5e000000a6637572766532353531392d736861323536406c69627373682e6f72672c656364682d736861322d6e697374703532312c656364682d736861322d6e697374703338342c656364682d736861322d6e697374703235362c6469666669652d68656c6c6d616e2d67726f757031342d736861312c6469666669652d68656c6c6d616e2d67726f7570312d736861312c6b6578677565737332406d6174742e7563632e61736e2e61750000004b65636473612d736861322d6e697374703235362c65636473612d736861322d6e697374703338342c65636473612d736861322d6e697374703532312c7373682d7273612c7373682d647373000000676165733132382d6374722c6165733235362d6374722c6165733132382d6362632c6165733235362d6362632c74776f666973683235362d6362632c74776f666973682d6362632c74776f666973683132382d6362632c336465732d6374722c336465732d636263000000676165733132382d6374722c6165733235362d6374722c6165733132382d6362632c6165733235362d6362632c74776f666973683235362d6362632c74776f666973682d6362632c74776f666973683132382d6362632c336465732d6374722c336465732d6362630000003b686d61632d736861312d39362c686d61632d736861312c686d61632d736861322d3235362c686d61632d736861322d3531322c686d61632d6d64350000003b686d61632d736861312d39362c686d61632d736861312c686d61632d736861322d3235362c686d61632d736861322d3531322c686d61632d6d64350000001a7a6c6962406f70656e7373682e636f6d2c7a6c69622c6e6f6e650000001a7a6c6962406f70656e7373682e636f6d2c7a6c69622c6e6f6e6500000000000000000100000000da8ff56e0000002c061e0000002034877d10cb2555ea557212f9e753a1a32ff238d5ef65accae887dc4666044c4ad4770cc862c5\",\"idp_len_out\":814,\"idp_in\":\"4502005d089e40003006dbb29e8205c9c0a801550016d2f63895b779a4ed89b3801800dedbf100000101080a0baabfe532eb22175353482d322e302d4f70656e5353485f372e337031205562756e74752d317562756e7475302e310d0a\",\"idp_len_in\":93}";
    struct flow_record record1;

    init_flow_record(&record1);

    fprintf(stdout, "start test json_string2flow_record function\n");
    fprintf(stdout, "json string :%s\n", json_str1);
    json_string2flow_record(&record1, json_str1);
    fprintf(stdout, "flow record features are following:\n");
    for (int i = 1; i <= NO_FEATURE; i++){
        if(record1.features[i].flags == NONEMPTY){
            fprintf(stdout, "\"%s\":%s\n", record1.features[i].name, record1.features[i].value);
        }
    }


}