#include    <stdio.h>

#include    "../include/stats.h"
#include    "../include/joy/joy_api.h"

extern fnet_stats_t * stats;

void fnet_stats_output(FILE *f){
    fprintf(f, "%lu connected users, %lu configured users, %lu working users, %lu fishingnet alloc fails\n",
            stats->num_connected_user, stats->num_configured_user, stats->num_working_user, stats->fnet_malloc_fail);
    
    joy_print_flocap_stats_output(0); // print the unique ctx stats
    fflush(f);
}