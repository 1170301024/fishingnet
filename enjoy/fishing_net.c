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

int main(){
    system_init();
}

int system_init(){
    pthread_t cmtid, dtid;
    int rc, fxpid;
    int fxd_pipe[2];


    
   // sleep(5);
    if(pipe(fxd_pipe) == -1){
        err_quit("pipe error");
    }

    if((fxpid = fork()) == 0){
        sleep(3);
        close(fxd_pipe[0]);
        if(dup2(fxd_pipe[1], STDOUT_FILENO) < 0){
            err_sys("dup2 error");
            return;
        }
        
        close(fxd_pipe[1]);
        init_feature_extract_service();
        return;
    }

    close(fxd_pipe[1]);
    if(dup2(fxd_pipe[0], STDIN_FILENO) < 0){
        err_sys("dup2 error");
        return ;
    }
    close(fxd_pipe[0]);
    
    Pthread_create(&dtid, NULL, init_distribute_service, NULL);
    printf("distribute service created\n");
    
 );
    Pthread_join(dtid, NULL);
    exit(0);
}