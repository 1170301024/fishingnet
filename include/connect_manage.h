#ifndef _CONNECT_MANAGE_H
#define _CONNECT_MANAGE_H

void *init_udp_connect_service(void * arg);

static void parse_udp_connect_msg(struct sockaddr *addr, int addr_len, char *msg, int len);
static void process_connect(struct sockaddr *addr, int addr_len);
static void process_restore(struct sockaddr *addr);
static void process_config(struct sockaddr *addr, char *cfg_data, int cfg_data_len);
static void process_pause();
static void response(struct sockaddr *addr, int addr_len, unsigned type, unsigned code, char *msg, int msg_len);
static void send_packet(struct sockaddr *addr, int addr_len, char *buffer, int buf_len);

#endif