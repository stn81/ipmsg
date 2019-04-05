#ifndef UDP_COMM_H
#define UDP_COMM_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include "msglist.h"


extern char broad_cast_addr[16];
extern int msg_sock_fd;
extern int file_sock_fd;

extern void init_socket();
extern int send_msg(int msg_sock_fd, struct msg_info *msg);
extern int recv_msg(int msg_sock_fd, struct msg_info *msg);
extern int login();
extern int logout();
extern void send_command(int msg_sock_fd,struct msg_info *msg,unsigned int commands);
#endif
