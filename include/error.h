#ifndef _ERROR_H
#define _ERROR_H


#include "joy/err.h"
#include "joy/utils.h"


#define MAX_ERR_LINE 512

typedef enum fnet_log_level {
    FNET_LOG_OFF = 0,
    FNET_LOG_DEBUG = 1,
    FNET_LOG_INFO = 2,
    FNET_LOG_WARN = 3,
    FNET_LOG_ERR = 4,
    FNET_LOG_CRIT = 5
} fnet_log_level_e;


#define FNET_LOG_DEBUG_STR JOY_LOG_DEBUG_STR
#define FNET_LOG_INFO_STR JOY_LOG_INFO_STR
#define FNET_LOG_WARN_STR JOY_LOG_WARN_STR
#define FNET_LOG_ERR_STR JOY_LOG_ERR_STR
#define FNET_LOG_CRIT_STR JOY_LOG_CRIT_STR

#define fnet_log_debug(...) { \
    if (fnet_glb_config->verbosity != FNET_LOG_OFF && glb_config->verbosity <= FNET_LOG_DEBUG) { \
            char log_ts[JOY_TIMESTAMP_LEN]; \
            joy_log_timestamp(log_ts); \
            fprintf(info, "%s: ", log_ts); \
            fprintf(info, "%s: %s: %d: ", JOY_LOG_DEBUG_STR, __FUNCTION__, __LINE__); \
            fprintf(info, __VA_ARGS__); \
            fprintf(info, "\n"); \
        } \
}

#define fnet_log_info(...) { \
        if (fnet_glb_config->verbosity != FNET_LOG_OFF && glb_config->verbosity <= FNET_LOG_INFO) { \
            char log_ts[JOY_TIMESTAMP_LEN]; \
            joy_log_timestamp(log_ts); \
            fprintf(info, "%s: ", log_ts); \
            fprintf(info, "%s: %s: %d: ", JOY_LOG_INFO_STR, __FUNCTION__, __LINE__); \
            fprintf(info, __VA_ARGS__); \
            fprintf(info, "\n"); \
        } \
}

#define fnet_log_warn(...) \
        if (fnet_glb_config->verbosity != FNET_LOG_OFF && glb_config->verbosity <= FNET_LOG_WARN) { \
            char log_ts[JOY_TIMESTAMP_LEN]; \
            joy_log_timestamp(log_ts); \
            fprintf(info, "%s: ", log_ts); \
            fprintf(info, "%s: %s: %d: ", JOY_LOG_WARN_STR, __FUNCTION__, __LINE__); \
            fprintf(info, __VA_ARGS__); \
            fprintf(info, "\n"); \
        } \
}

#define fnet_log_err(...) \
        if (fnet_glb_config->verbosity != FNET_LOG_OFF && glb_config->verbosity <= FNET_LOG_ERR) { \
            char log_ts[JOY_TIMESTAMP_LEN]; \
            joy_log_timestamp(log_ts); \
            fprintf(info, "%s: ", log_ts); \
            fprintf(info, "%s: %s: %d: ", JOY_LOG_ERR_STR, __FUNCTION__, __LINE__); \
            fprintf(info, __VA_ARGS__); \
            fprintf(info, "\n"); \
        } \
}

#define fnet_log_crit(...) \
        if (fnet_glb_config->verbosity != FNET_LOG_OFF && glb_config->verbosity <= FNET_LOG_CRIT) { \
            char log_ts[JOY_TIMESTAMP_LEN]; \
            joy_log_timestamp(log_ts); \
            fprintf(info, "%s: ", log_ts); \
            fprintf(info, "%s: %s: %d: ", JOY_LOG_CRIT_STR, __FUNCTION__, __LINE__); \
            fprintf(info, __VA_ARGS__); \
            fprintf(info, "\n"); \
        } \
}


void	 err_dump(const char *, ...);
void	 err_msg(const char *, ...);
void	 err_quit(const char *, ...);
void	 err_ret(const char *, ...);
void	 err_sys(const char *, ...);


#endif