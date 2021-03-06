#ifndef _USER_H
#define _USER_H

#include    <netinet/in.h>
#include    <sys/socket.h>
#define __USE_UNIX98
#include    <pthread.h>

#include    "flow.h"
#include    "proto.h"

#define __USE_UNIX98
typedef struct user_list_t{
    pthread_rwlock_t rwlock;
    int no_user;
    struct user * user_list_head;
    struct user * user_list_tail;
} user_list_t;

#define l_head(list) ((list).user_list_head)
#define l_tail(list) ((list).user_list_tail)

typedef struct list_head{
    struct user *prev;
    struct user *next;
} list_head;

// the features that user configs
struct user_feature{
    int no_feature;
    struct feature_mask fm;
};

#define NO_MAX_USER 1024

// user status offset in flags
#define USER_CONNECTED   0x01 
#define USER_LOGGED  0x02
#define USER_CONFIGURED   0x04
#define USER_WORKING 0x08
#define USER_ABNORMAL    0x10

#define WORKING_USER_STATE (USER_CONNECTED | USER_LOGGED | USER_CONFIGURED | USER_WORKING)

#define USER_DUMP_FILE "userinfo.tdb"

struct user{
    unsigned int id;
    pthread_rwlock_t rwlock; 
    struct user_register *user_info;
    unsigned int user_state;
    struct user_feature *config;
    unsigned int no_records_send;
    time_t last_ctime;    
    long cduration;     /* if the user is unconnected, the field denotes last connection duration,
                        otherwises denotes the current connection duration*/
    struct msghdr user_msghdr;
    list_head users;
    list_head work_users;
};

int init_user_list();

struct user* new_user();

int add_user(struct user *u);
void delete_user(struct user *u);
int dump_user(struct user *u);
void restore_user(struct user *u);
struct user* get_users_from_tdb();

int add_work_user(struct user *u);
int delete_work_user(struct user *u, unsigned int state);

struct user* search_user_with_addr(struct sockaddr *addr);





#endif
