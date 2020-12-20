#include "../include/enjoy.h"
#include "../include/user.h"
#include "../include/error.h"
#include    <string.h>
#include    <pthread.h>




user_list_t user_list;
user_list_t work_user_list;




int init_user_list(){
    user_list.no_user = 0;
    l_head(user_list) = l_tail(user_list) =  NULL;    

    work_user_list.no_user = 0;
    l_head(work_user_list) = l_tail(work_user_list) = NULL;
    get_users_from_tdb();
    return 0;
}

/*
 * create a new user and initialize it
 */
struct user* new_user(){
    struct user *u;

    u = (struct user*)malloc(sizeof (struct user));
    if(u == NULL)
        goto malloc_err;

    u->user_info = (struct user_register*)malloc(sizeof (struct user_register));
    if(u->user_info == NULL)
        goto malloc_err;

    u->config = (struct user_feature*)malloc(sizeof (struct user_feature));
    if(u->config == NULL)
        goto malloc_err;

    u->user_state = 0;
    return u;

malloc_err:
    err_msg("malloc error");
    return NULL;

}

/*
 * add user to user_list which has all connected users 
 */
int add_user(struct user *u){
    if(u == NULL){
#ifdef ENJOY_DEBUG
        err_msg("struct user is null");
#endif
       return -1;
    }
    pthread_rwlock_wrlock(&(user_list.rwlock));
    if(NULL == l_head(user_list)){
        l_head(user_list) = l_tail(user_list) = u;
        u->users.next = NULL;
        u->users.prev = NULL;
        
    }
    else{
        (l_tail(user_list)->users).next = u;
        (u->users).prev = l_tail(user_list);
        (u->users).next = NULL;
        l_tail(user_list) = u;
    }
    user_list.no_user++;

    pthread_rwlock_unlock(&(user_list.rwlock));

    if(u->user_state & USER_WORKING){
        add_work_user(u);
    }
    printf("added a new working  user\n");
    return 0;
}

/*
 * delete all information of a user in system.
 */
void delete_user(struct user *u){

}

/*
 * dump user information to disk,or save the user information to database.
 * 
 */
void dump_user(struct user *u){

}

/*
 * restore user information from database or disk file
 */
void restore_user(struct user *u){
    
}

struct user* get_users_from_tdb(){

}

/*
 * search for a user with the argument addr, if the user exists, then return it,
 * otherwise return NULL
 * the function can be optimized by a specific data structure.
 */
struct user* search_user_with_addr(struct sockaddr *addr){
    struct sockaddr *uaddr;
    if(addr == NULL){
        err_msg("error");
        return NULL;
    }
    pthread_rwlock_rdlock(&(user_list.rwlock));
    for(struct user* u = l_head(user_list); u != NULL; u = (u->users).next){
        uaddr = (struct sockaddr*)((u->user_msghdr).msg_name);
        if(0 == memcmp(uaddr->sa_data, addr->sa_data, sizeof (uaddr->sa_data))){
            pthread_rwlock_unlock(&(user_list.rwlock));
            return u;
        }
    }
    pthread_rwlock_unlock(&(user_list.rwlock));
    return NULL;
}

/*
 * add user to work_user_list which has all working users
 */
int add_work_user(struct user *u){
    if(u == NULL){
#ifdef ENJOY_DEBUG
        err_msg("struct user is null");
#endif
       return -1;
    }
    
    if(u->user_state != WORKING_USER_STATE){
        err_msg("user state error");
        return -1;
    }
    pthread_rwlock_wrlock(&(work_user_list.rwlock));
    if(NULL == l_head(work_user_list)){
        l_head(work_user_list) = l_tail(work_user_list) = u;
        (u->work_users).next = NULL;
        (u->work_users).prev = NULL;
        
    }
    else{
        (l_tail(work_user_list)->work_users).next = u;
        (u->work_users).prev = l_tail(work_user_list);
        (u->work_users).next = NULL;
        l_tail(work_user_list) = u;
    }
    work_user_list.no_user++;
    pthread_rwlock_unlock(&(work_user_list.rwlock));
    return 0;
}

/* 
 * delete a user from working user list, and then set the user state with the argument state
 * the two actions should be atomic in the system level? 
 */
int delete_work_user(struct user *u, unsigned int state){
    if(u == NULL){
#ifdef ENJOY_DEBUG
        err_msg("struct user is null");
#endif
       return -1;
    }

    if(state == WORKING_USER_STATE){
        return -2;
    }
    // TO-CONSIDERATE
    (u->work_users.prev)->work_users.next = (u->work_users.next);
    (u->work_users.next)->work_users.prev = (u->work_users.prev);
    u->user_state = state;
}

