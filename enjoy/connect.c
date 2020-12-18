#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <arpa/inet.h>
#include    <sys/param.h>

#include "../include/proto.h"
#include "../include/user.h"
#include "../include/error.h"

/*
 * I should consider that user has more than one ip, in this case, maybe two udp packets from user to
 * server have different ip, then which ip should be chosen to send from server. 
 * 
 * 
 */


void init_udp_connect_service(){
    char msg[MAX_UDP_MSG];
    int sockfd, n;
    socklen_t len;
    struct sockaddr_in server_addr, client_addr;


    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == sockfd){
        err_sys("socket error");
    }


    memset(&server_addr, 0, sizeof server_addr);

    // init the server endpoint info
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htos(SERVER_UDP_CONNECT_PORT);
    inet_pton(AF_INET, SERVER_UDP_CONNECT_IPv4, &server_addr.sin_addr);
    
    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof server_addr) == -1){
        err_sys("bind error");
    }
    
    for( ; ; ){
        n = recvfrom(sockfd, msg, MAX_UDP_MSG, 0, (struct sockaddr *)&client_addr, sizeof client_addr);
        if(n < 0){
            err_sys("recvfrom error");
        }

        parse_udp_connect_msg((struct sockaddr *)&client_addr, sizeof client_addr, msg, n);
    }
}

void parse_udp_connect_msg(struct sockaddr *addr, int addr_len, char *msg, int len){
     udp_connect_protocol p;
     p.code = msg[0];
     p.type = msg[1];
     p.data_length = ntohs(msg[2]);
     switch(p.code){
         case CONNECT:
            
         case PAUSE:
         case RESTORE:
         case CONFIG:
         case RESPONSE:
     }

}



