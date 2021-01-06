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
#include    "../include/config.h"


fnet_configuration_t fnet_glb_config;


int 
system_config(void){

    int no_ifs;
    config_from_xml(&fnet_glb_config, "../conf/fishing_net.xml");

    // print configuration
    if(fnet_glb_config.show_config){
        fnet_config_print(stderr, &fnet_glb_config);
    }
    no_ifs = interface_list_get();
    if(fnet_glb_config.show_interface)
        print_interfaces(stderr, no_ifs);
    return 0;
    
    
}

/* system initialization included 
 *   1. system configuration initialization
 *   2. connection service and distribution service initialization
 *   3. feature extraction servicer initialization
 * 
 */
int system_init(){
    pthread_t cmtid, dtid;
    int rc, fxpid;
    int fxd_pipe[2];

    
    if(init_user_list() != 0){
        err_quit("init user list error");
    }

    Pthread_create(&cmtid, NULL, init_udp_connect_service, NULL);
    
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
        return -1;
    }
    close(fxd_pipe[0]);
    
    Pthread_create(&dtid, NULL, init_distribute_service, NULL);
    
    Pthread_join(cmtid, NULL);
    Pthread_join(dtid, NULL);
    exit(0);
}


int main(){
    system_config();
    system_init();
    return 0;
}
