#ifndef MSG_H
#define MSG_H

#define LEN_MSG 1000
#define LEN_DGRAM 1500
#define LEN_NAME 64


#include <semaphore.h>
#include "ipmsg.h"
#include <arpa/inet.h>
//#include "comm.h"
#include <pthread.h>

#define INIT_MSGLIST(msglist)  { init_msg(&(msglist).head_msg,IPMSG_NOOPERATION); \
					(msglist).tail_msg_ptr = &(msglist).head_msg;}
					
struct msg_info
{
	unsigned int version;
	unsigned int packet_no;
	char 		sender[LEN_NAME];
	char 		host[LEN_NAME];
	unsigned int command;
	char	        extended[LEN_MSG];
	struct sockaddr_in peer;
	struct msg_info *next;
	/*for extended use*/
};

struct msg_list
{      
        struct msg_info head_msg;
        struct msg_info *tail_msg_ptr;
} ;



extern struct msg_list ipmsg_msglist;
extern sem_t NE_MSGLIST, NF_MSGLIST; 
extern pthread_mutex_t MUTEX_MSGLIST;

extern int init_msg(struct msg_info *msg,unsigned int COMMAND);
extern int parse_msg(char *buf,size_t buf_len,struct msg_info *msg);
extern int form_msg(char *buf,size_t buf_len,struct msg_info *msg);


extern struct msg_info * get_head_msg(struct msg_list *msglist);
extern int add_tail_msg(struct msg_list *msglist,struct msg_info *msg);
extern int free_msglist(struct msg_list *msglist);

#endif  /*MSG_H*/

