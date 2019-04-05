#ifndef USERLIST_H
#define USERLIST_H

#define LEN_NAME 64
#define INIT_USERLIST(userlist) { (userlist).next = NULL ;}

#define USER_EXIST -2
#define USER_NONEXIST -2
#define FAILED_ADD -1
#define FAILED_DEL -1
#define SUCCESS 0

#include <pthread.h>
#include "msglist.h"

struct user_info
{
	struct sockaddr_in peer;
	char name[LEN_NAME];
	char host[LEN_NAME];
	char nickname[LEN_NAME];
	short priority;
//	short in_use;
	short exit;
	struct user_info *next;
};

extern struct user_info ipmsg_usrlist;
extern pthread_mutex_t MUTEX_USERLIST;
extern int add_user(struct user_info *user_list,struct msg_info *msg);
extern int del_user(struct user_info *user_list,struct msg_info *msg);
extern int find_user_byNO(struct user_info *user_list,int user_no,struct user_info **target);
extern int list_user(struct user_info *user_list);
extern int free_userlist(struct user_info *user_list);

extern int refresh();
extern int talkto_user(struct user_info *user_list,int user_no,char *talk_msg,int msg_len);
#endif
