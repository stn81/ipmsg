#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "ipmsg.h"
#include "udp_comm.h"
#include "utils.h"
#include "userlist.h"

struct user_info ipmsg_usrlist;
pthread_mutex_t MUTEX_USERLIST;

int add_user(struct user_info *user_list,struct msg_info *msg)
{
	struct user_info *pre,*cur;
	struct user_info *new_user;
	short comp_name,comp_host,comp_addr;

	
	if((user_list == NULL)||(msg == NULL))
		return FAILED_ADD;
	
	if(msg->peer.sin_addr.s_addr == htonl(INADDR_ANY))
		return FAILED_ADD;
	
	new_user = (struct user_info *)s_malloc(sizeof(struct user_info));
	bzero(new_user,sizeof(struct user_info));
	memcpy(&new_user->peer,&msg->peer,sizeof(msg->peer));
	strncpy(new_user->name,msg->sender,sizeof(new_user->name));
	strncpy(new_user->host,msg->host,sizeof(new_user->host));
//	new_user->in_use = 0;
	new_user->exit = 0;
	new_user->next = NULL;
	
	pthread_mutex_lock(&MUTEX_USERLIST);
	pre = user_list;
	cur = user_list->next;
	
	
	while(cur != NULL)
	{
		comp_name = strncmp(cur->name,new_user->name,LEN_NAME);
		if(comp_name > 0)
			break;
		comp_host = strncmp(cur->host,new_user->host,LEN_NAME);
		comp_addr = memcmp(&cur->peer,&new_user->peer,sizeof(cur->peer));
		
		if( (comp_name == 0)&&(comp_host == 0)&&(comp_addr == 0) )
		{
			if(cur->exit == 1)
				cur->exit = 0;
			
			pthread_mutex_unlock(&MUTEX_USERLIST);
			return USER_EXIST;
		}
		
		pre = cur;
		cur = cur->next;
	}
	
	new_user->next = cur;
	pre->next	 = new_user;
	pthread_mutex_unlock(&MUTEX_USERLIST);
	return SUCCESS;
}

int del_user(struct user_info *user_list,struct msg_info *msg)
{
	struct user_info *cur;
	int comp_name,comp_host,comp_addr;
	
	if((user_list == NULL)||(msg == NULL))
		return FAILED_DEL;
	
	pthread_mutex_lock(&MUTEX_USERLIST);
	cur = user_list->next;
	
	while(cur != NULL)
	{
		comp_name = strncmp(cur->name,msg->sender,LEN_NAME);
		if(comp_name > 0) {
			pthread_mutex_unlock(&MUTEX_USERLIST);
			return USER_NONEXIST;
		}
			
		comp_host = strncmp(cur->host,msg->host,sizeof(cur->host));
		comp_addr = memcmp(&cur->peer,&msg->peer,sizeof(cur->peer));
		
		if( (comp_name == 0)&&(comp_host == 0)&&(comp_addr == 0))
		{
			cur->exit = 1;
			break;
		}
		
		cur = cur->next;
	}
	pthread_mutex_unlock(&MUTEX_USERLIST);
	return SUCCESS;
}

int find_user_byNO(struct user_info *user_list,int user_no,struct user_info **target)
{
	struct user_info *cur;
	int count;
	
	if((user_list == NULL)||(user_no < 1))
		return -1;
	
	pthread_mutex_lock(&MUTEX_USERLIST);
	cur = user_list->next;
	
	for(count = 1; (cur->next != NULL) &&(count < user_no); cur = cur->next,count++ );
	*target=cur;
	pthread_mutex_unlock(&MUTEX_USERLIST);
	
	return (count != user_no);
}
		
		
int list_user(struct user_info *user_list)
{
	int user_no;
	char title_name[] = "USERNAME";
	char title_host[] = "HOST";
	char title_IP[] = "IP";
	char title_pri[] = "PRIORITY";/*use in later*/
	struct user_info *cur;
	
		
	printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("\t%-10s %-15s %-15s %-5s\n",title_name,title_host,title_IP,title_pri);
	pthread_mutex_lock(&MUTEX_USERLIST);
	cur = user_list->next;
	for(user_no = 0; cur != NULL; cur = cur->next)
	{
		if(cur->exit == 0)
		{
			printf("    %-4d%-10s %-15s %-15s  %-5s\n",
				++user_no,cur->name,cur->host,inet_ntoa(cur->peer.sin_addr),"-");
		}
	}
	pthread_mutex_unlock(&MUTEX_USERLIST);
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	
	
	if(user_no == 0) {
		printf("No users exists!\n");
		printf("Please list later.\n\n");
	}
	return user_no;
}




int free_userlist(struct user_info *user_list)
{
	struct user_info *cur;
	struct user_info *next;
	
	if(user_list == NULL)
		return -1;
	
	pthread_mutex_lock(&MUTEX_USERLIST);
	cur = user_list->next;
	while(cur != NULL)
	{
		next = cur->next;
		free(cur);
		cur = next;
	}
	pthread_mutex_unlock(&MUTEX_USERLIST);
	return SUCCESS;
}		

int talkto_user(struct user_info *user_list,int user_no,char *talk_msg,int msg_len)
{
	int ret;
	struct user_info *target_user;
	struct msg_info *msg;
	
	if((user_list == NULL) || (user_no < 1)||(talk_msg == NULL)) {
		printf("!!!Non user exist OR provied user_no is invailid OR message is nil.\n");
		return -1;
	}
	
	ret = find_user_byNO(user_list,user_no,&target_user);
	if(ret != 0) {
		printf("\nThe USER_NO provided can't be find! \n");
		printf("Please check your input is correct.\n");
		return USER_NONEXIST;
	}
	
	
	msg = (struct msg_info *)s_malloc(sizeof(struct msg_info));
	init_msg(msg,IPMSG_SENDMSG|IPMSG_SENDCHECKOPT);
	memcpy(&msg->peer,&target_user->peer,sizeof(msg->peer));
	strncpy(msg->extended,talk_msg,LEN_MSG);
	
	ret=send_msg(msg_sock_fd,msg);
	if(ret < 0) {	
		printf("The message FAILED to SEND!(%s) :\n MESSAGE CONTENT:%s\n",
				target_user->name,msg->extended);
	}
	else
		printf("Message send OK!MSG_ID[$%u]\n",msg->packet_no);
	free(msg);
	return ret;
}

int refresh()
{
	login();
	list_user(&ipmsg_usrlist);
	return 0;
}
