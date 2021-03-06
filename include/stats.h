#ifndef __STATS_H__
#define __STATS_H__

#include "joy/p2f.h" // include flocap_status_t

typedef struct fnet_stats_{
    unsigned long int num_connected_user;
    unsigned long int num_configured_user;
    unsigned long int num_working_user;
    unsigned long int fnet_malloc_fail; // not include malloc fail in joy


    flocap_stats_t *joy_ctx_status; // only include one ctx status as far

}fnet_stats_t;

#endif