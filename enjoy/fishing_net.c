#include    <pthread.h>


#include    "../include/enjoy.h"
#include    "../include/fishing_net.h"
#include    "../include/user.h"
#include    "../include/feature.h"
#include    "../include/connect_manage.h"
#include    "../include/fnetthread.h"
#include    "../include/error.h"

int main(void){
    system_init();
}

int system_init(){
    pthread_t cmtid, dtid;
    int rc;

    if(init_user_list() != 0){
        err_quit("init user list error");
    }

    Pthread_create(&cmtid, NULL, init_udp_connect_service, NULL);
    printf("connect service created\n");

    //Pthread_create(&dtid, NULL, init_distribute_service, NULL);
    //printf("distribute service created\n");

    Pthread_join(cmtid, NULL);
    //Pthread_join(dtid, NULL);
    exit(0);
}