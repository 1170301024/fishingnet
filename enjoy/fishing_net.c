#include    <pthread.h>
#include    <unistd.h>

#include    "../include/enjoy.h"
#include    "../include/fishing_net.h"
#include    "../include/user.h"
#include    "../include/feature.h"
#include    "../include/connect_manage.h"
#include    "../include/feature_extract.h"
#include    "../include/fnetthread.h"
#include    "../include/error.h"

int main(void){
    system_init();
}

int system_init(){
    pthread_t cmtid, dtid;
    int rc, fxpid;
    int fxd_pipe[2];

    if(init_user_list() != 0){
        err_quit("init user list error");
    }

    Pthread_create(&cmtid, NULL, init_udp_connect_service, NULL);
    printf("connect service created\n");

    if(pipe(fxd_pipe) == -1){
        err_quit("pipe error");
    }
    
    if((fxpid = fork()) == 0){
        close(fxd_pipe[1]);
        init_feature_extract();
    }

    close(fxd_pipe[0]);
    Pthread_create(&dtid, NULL, init_distribute_service, NULL);
    printf("distribute service created\n");

    Pthread_join(cmtid, NULL);
    Pthread_join(dtid, NULL);
    exit(0);
}