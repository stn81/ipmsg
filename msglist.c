#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <pwd.h>
#include "ipmsg.h"
#include "msglist.h"
#include "encode.h"

struct msg_list ipmsg_msglist;
sem_t NE_MSGLIST, NF_MSGLIST; 
pthread_mutex_t MUTEX_MSGLIST;

int init_msg(struct msg_info *msg,unsigned int COMMAND)
{
	if(msg==NULL)
		return -1;
	bzero(msg,sizeof(struct msg_info));
	msg->version = 1;
	msg->packet_no = (unsigned int)time(NULL);
	strncpy(msg->sender,local_user->pw_name,sizeof(msg->sender));
	strncpy(msg->host,local_host.nodename,sizeof(msg->host));
	msg->command = COMMAND;
	msg->next = NULL;
	return 0;
}

int parse_msg(char *buf,size_t buf_len,struct msg_info *msg)
{
	char 	*start,
		*pos,
		buf_gb2312[LEN_DGRAM];
	short 	colon_count,
		index = 0;

	
	
	if(utf8)
	{
		g2u(buf,buf_len,buf_gb2312,sizeof(buf_gb2312));
		start = buf_gb2312;
	}
	else
		start = buf;
		
	bzero(msg,sizeof(struct msg_info));
	while(index<5)
	{
		pos = strchr(start,':');
		if(pos == NULL)
			break;
		
		colon_count = 1;
		
		while(*(pos+1) == ':') {
			pos++;
			colon_count++;
		}
		
		if(colon_count%2 == 0) {
			start=pos+1;
			continue;
		} 
		else 
			*pos='\0';
		
		switch (index)
		{
		case 0:
			msg->version = atoi(start);
			break;
		case 1:
			msg->packet_no=atoi(start);
			break;
		case 2:
			memcpy(msg->sender,start,sizeof(msg->sender));
			break;
		case 3:
			memcpy(msg->host,start,sizeof(msg->host));
			break;
		case 4:
			msg->command = atoi(start);
			break;
		default:
			break;
		}
		
		start = pos+1;
		index++;
	}
	strncpy(msg->extended,start,LEN_MSG);
	
	return index;
}

int form_msg(char *buf,size_t buf_len,struct msg_info *msg)
{
	char	msg_tmp[buf_len];

	
	if((buf == NULL) || (msg == NULL))
		return -1;
	
	snprintf(msg_tmp,sizeof(msg_tmp),"%d:%d:%s:%s:%d:%s",
			msg->version,
			msg->packet_no,
			msg->sender,
			msg->host,
			msg->command,
			msg->extended);

	if(utf8)
		u2g(msg_tmp,sizeof(msg_tmp),buf,buf_len);
	else
		strncpy(buf,msg_tmp,buf_len);
//	printf("formed msg:%s\n",buf);
	return 0;
}


struct msg_info * get_head_msg(struct msg_list *msglist)
{
	struct msg_info *msg;
	if((msglist == NULL)) {
		return NULL;
	}
	sem_wait(&NE_MSGLIST);
	pthread_mutex_lock(&MUTEX_MSGLIST);

	msg = msglist->head_msg.next;
//	printf("inter %p\n",msg);
	msglist->head_msg.next = msglist->head_msg.next->next;
	if(msg == NULL) {
		printf("get_head_msg error!\n");
	}
		
	if(msglist->head_msg.next == NULL)
		msglist->tail_msg_ptr = &(msglist->head_msg);
	sem_post(&NF_MSGLIST);
	pthread_mutex_unlock(&MUTEX_MSGLIST);
	return msg;
}
	
int add_tail_msg(struct msg_list *msglist,struct msg_info *msg)
{
	sem_wait(&NF_MSGLIST);
	pthread_mutex_lock(&MUTEX_MSGLIST);
	msglist->tail_msg_ptr->next = msg;
	msglist->tail_msg_ptr = msg;
	msg->next = NULL;
	sem_post(&NE_MSGLIST);
	pthread_mutex_unlock(&MUTEX_MSGLIST);
	//printf("add tail msg ok!\n");
	return 0;
}

int free_msglist(struct msg_list *msglist)
{
	struct msg_info *msg;
	pthread_mutex_lock(&MUTEX_MSGLIST);
	while(msglist->head_msg.next != NULL)
	{
		msg = get_head_msg(msglist);
		free(msg);
	}
	pthread_mutex_unlock(&MUTEX_MSGLIST);
	return 0;
}
	

		
