#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <arpa/inet.h>
#include    <sys/param.h>
#include    <string.h>
#include    <stdio.h>

#include    "../include/enjoy.h"
#include    "../include/proto.h"
#include    "../include/user.h"
#include    "../include/error.h"
#include    "../include/connect_manage.h"

/*
 * I should consider that user has more than one ip, in this case, maybe two udp packets from user to
 * server have different ip, then which ip should be chosen to send from server. 
 */

int cmsockfd;

void *init_udp_connect_service(void *arg){
    char msg[MAX_UDP_MSG];
    int n;
    socklen_t len;
    struct sockaddr_in server_addr, client_addr;
    
    cmsockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == cmsockfd){
        err_sys("socket error");
        return ; 
    }

    memset(&server_addr, 0, sizeof server_addr);

    // init the server endpoint info
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(SERVER_UDP_CONNECT_PORT);
    
    server_addr.sin_addr.s_addr = htonl(SERVER_UDP_CONNECT_IPv4);
    if(bind(cmsockfd, (struct sockaddr *)&server_addr, sizeof server_addr) == -1){
        err_sys("bind error");
        return ;
    }
    init_user_list();
    printf("Initialization of connection service completed, waiting for user request\n");
    for( ; ; ){
        n = recvfrom(cmsockfd, msg, MAX_UDP_MSG, 0, (struct sockaddr*)&client_addr, &len);
        if(n < 0){
            err_sys("recvfrom error");
            continue;
        }
#ifdef LOCAL_NET_ENV
        inet_pton(AF_INET, "192.168.79.128", &(client_addr.sin_addr.s_addr));
        client_addr.sin_port = htons(60606);
#endif
        printf("Receive a packet from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        parse_udp_connect_msg((struct sockaddr *)&client_addr, sizeof client_addr, msg, n);
    }
}

static void parse_udp_connect_msg(struct sockaddr *addr, int addr_len, char *msg, int len){
    udp_connect_protocol p;
    unsigned rsp_type, rsp_code;

    p.code = msg[0];
    p.type = msg[1];
    p.proto_length = ntohs(*((short*)(msg+2)));

    if(p.proto_length != len){
        err_msg("payload length error");
        return ;
    }
    if(p.proto_length > CHEADER_LEN){
        p.data = msg + CHEADER_LEN;
    }
    else{
        p.data = NULL;
    }


    switch(p.code){
        case CONNECT:
            process_connect(addr, addr_len);
            break;

        case PAUSE:
            process_pause();
            break;
        case RESTORE:
            process_restore(addr); 
            break;
        case CONFIG:
            process_config(addr, p.data, len - CHEADER_LEN);
            break;
        case RESPONSE:
            break;
        default:
            err_msg("protocol type error");
            return;
    }
    return ;
}

/*
 * search for a user with the same addr as the argument,if the user exists, return the error
 * for
 */
static void process_connect(struct sockaddr *addr, int addr_len){
    struct user *u;
    
    if(search_user_with_addr(addr) != NULL){
        goto err_rsp;
    }
    u = new_user();
    if(u == NULL){
        goto err_rsp;
    }
    
    u->user_msghdr.msg_name = (struct sockaddr_in *)malloc(addr_len);
    *((struct sockaddr*)(u->user_msghdr.msg_name)) = *addr;
    u->user_msghdr.msg_namelen = addr_len;

    // we don't have a log action, so we set the state USER_CONNECTED and USER_LOGGED
    u->user_state = USER_CONNECTED | USER_LOGGED;
    if(-1 == time(u->last_ctime)){
        err_msg("time error");
        goto err_rsp;
    }
    if(add_user(u) < 0){
        goto err_rsp;
    }
    
#ifdef ENJOY_DEBUG
    struct sockaddr_in *uaddr = (struct sockaddr_in*)(u->user_msghdr.msg_name);
    printf("%s:%d is connecting to server\n", inet_ntoa(uaddr->sin_addr), ntohs(uaddr->sin_port));
#endif
    response(addr, sizeof (*addr), CONNECT_RSP, CMRSP_OK, NULL, 0);
    return ;

err_rsp:
    err_msg("connect error");
    response(addr, sizeof (*addr), CONNECT_RSP, CMRSP_ERR, NULL, 0);
    return ;
}

static void process_pause(){

}

/*
 * 
 * 
 */
static void process_restore(struct sockaddr *addr){
    struct user *u;


    if((u = search_user_with_addr(addr)) == NULL){
        goto err_rsp;
    }

    pthread_rwlock_wrlock(&(u->rwlock));
    if(0 == (u->user_state & USER_CONNECTED)){
        err_msg("user state error: restore but not connect");
        goto err_rsp;
    }
    if(0 == (u->user_state & USER_LOGGED)){
        err_msg("user state error: restore but not login");
        goto err_rsp;
    }
    if(0 == (u->user_state & USER_CONFIGURED)){
        err_msg("user state error: restore but not configure");
        goto err_rsp;
    }
    u->user_state |= USER_WORKING;
    pthread_rwlock_unlock(&(u->rwlock));
    // TO-DO
    add_work_user(u);  

    response(addr, sizeof (*addr), RESTORE_RSP, CMRSP_OK, NULL, 0);
    return ;

err_rsp:
    response(addr, sizeof (*addr), RESTORE_RSP, CMRSP_ERR, NULL, 0);
    return ;
}

/*
 * 0        8        16                32
 * +--------+--------+----------------+
 * |  0x05  |  0x01  |    proto len   |
 * +--------+--------+----------------+
 * |number of feature|1010111001101000|  if the largest feature code is not an integer multiple of byte bits, complete last byte with bit 0                                     
 * +--------+--------+----------------+
 *                       
 */          
static void process_config(struct sockaddr *addr, char *cfg_data, int cfg_data_len){
    struct user *u;
    unsigned short no_ft;
    char *ft_hdr_buf, ft_flag;
    int b_count = 0, ft_count = 0, code;
    
    
    if((u = search_user_with_addr(addr)) == NULL){
        goto err_rsp;
    }

    if(0 == (u->user_state & USER_CONNECTED)){
        err_msg("user state error: configuring but not connect");    
        goto err_rsp;
    }
    if(0 == (u->user_state & USER_LOGGED)){
        err_msg("user state error: configuring but not login");
        goto err_rsp;
    }

    if(cfg_data_len <= 1){
        err_msg("message length error");
        goto err_rsp;
    }
    no_ft = ntohs(*((unsigned short *)cfg_data));
    
    if(no_ft == 0){
        err_msg("number of feature field error");
        goto err_rsp;
    }

    pthread_rwlock_wrlock(&(u->rwlock));
    u->user_msghdr.msg_iov = (struct iovec*) malloc(sizeof(struct iovec) * (no_ft *  2 + 1));
    u->user_msghdr.msg_iovlen = no_ft * 2 + 1;
    u->user_msghdr.msg_iov[0].iov_base = (char *)malloc(4);
    u->user_msghdr.msg_iov[0].iov_len = 4;
    do{
        ft_flag = *(cfg_data + sizeof no_ft + b_count);
        for(int i=0; i<8; i++){
            if(ft_flag & (1 << i)){
                code = b_count * 8 + i;
                mask_fm(u->config->fm, code);
                u->user_msghdr.msg_iov[ft_count * 2 + 1].iov_base = (char *)malloc(3); // !!!!!
                u->user_msghdr.msg_iov[ft_count * 2 + 1].iov_len = 3;  
                ft_count++;
            }
            if(ft_count == no_ft){
                break;
            }
        }
        b_count++;
        if((b_count + sizeof no_ft) > cfg_data_len){
            err_msg("config message error");
            goto err_rsp;
        }

    }while(ft_count != no_ft);


    u->config->no_feature = ft_count;
    u->user_state |= USER_CONFIGURED;    

    pthread_rwlock_unlock(&(u->rwlock));
    response(addr, sizeof (*addr), CONFIG_RSP, CMRSP_OK, NULL, 0);
    return ;

err_rsp:
    pthread_rwlock_unlock(&(u->rwlock));
    response(addr, sizeof (*addr), CONFIG_RSP, CMRSP_ERR, NULL, 0);
    return ;
}

/*
 * construct response packet
 */
static void response(struct sockaddr *addr, int addr_len, unsigned type, unsigned code, char *msg, int msg_len){
    unsigned char rsp_buffer[128];
    int buf_len;

    buf_len = CHEADER_LEN;

    if(NULL != msg){
        buf_len += msg_len;
    }

    *rsp_buffer = type;
    *(rsp_buffer+1) = code;
    *((unsigned short *)(rsp_buffer+2)) = htons(buf_len);
    if(msg != NULL){
        memcpy(rsp_buffer + CHEADER_LEN, msg, msg_len);
    }
    send_packet(addr, addr_len, rsp_buffer, buf_len);
}

static void send_packet(struct sockaddr *addr, int addr_len, char *buffer, int buf_len){

    if(sendto(cmsockfd, buffer, buf_len, 0, addr, addr_len) != (ssize_t)buf_len){
        err_sys("sendto error");
    }
}