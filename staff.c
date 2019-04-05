#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pwd.h>
#include "ipmsg.h"
#include "userlist.h"
#include "udp_comm.h"
#include "msglist.h"
#include "utils.h"
#include "staff.h"

void *program_guide(void *arg)
{
	char instruction[16];
	char  user_no[20];
	char talk_msg[LEN_MSG];
	
	printf("%c[2J", 0x1b);		/*清除屏幕*/
	printf("%c[0;0H", 0x1b);
	printf("\t\tIPMSG FOR LINUX (ver 1.0)\n");
	list_user(&ipmsg_usrlist);
	
	
	while(1)
	{
		printf("\n(IPMSG):");
		bzero(instruction,sizeof(instruction));
		fget_str(instruction,sizeof(instruction),stdin);
	//	printf("\nYOu input %s :size is %d\n",instruction,(int)strlen(instruction));
		
		if(strlen(instruction) == 0)
			continue;
		if(strncmp(instruction,"ls",strlen(instruction)) == 0) {
			printf("\t\tIPMSG FOR LINUX (ver 1.0)\n");
			list_user(&ipmsg_usrlist);
		}
		else if(strncmp(instruction,"talk",strlen(instruction)) == 0)
		{
			printf("Input USER_NO to talk:[1]");
			fget_str(user_no,sizeof(user_no),stdin);
			printf("\n<MESSAGE TO SEND TO %s:",user_no);
			fget_str(talk_msg,sizeof(talk_msg),stdin);
			talkto_user(&ipmsg_usrlist,atoi(user_no),talk_msg,strlen(talk_msg));
			printf("\n(IPMSG):");
		}
		else if(strncmp(instruction,"quit",strlen(instruction)) == 0){
			free_msglist(&ipmsg_msglist);
			free_userlist(&ipmsg_usrlist);
			logout();
			close(msg_sock_fd);
			exit(0);	
		}
		else if(strncmp(instruction,"refresh",strlen(instruction)) == 0){
			printf("%c[2J", 0x1b);		/*清除屏幕*/
			printf("%c[0;0H", 0x1b);
			printf("\t\tIPMSG FOR LINUX (ver 1.0)\n");
			refresh();	
		}
		else {
			printf("Usage for ipmsg:\n\tl(list users)\tt(talk to a user)\n");
			printf("\tr(refresh list)\th(show help)\tq(quit ipmsg)\n");
		}
	}	
}
	
	
	
void *msg_receiver(void *arg)
{
	int msg_sock_fd=*(int *)arg;
	struct msg_info *msg_new;
	struct msg_info msg_tmp;
	int ret;
	
	
	while(1)
	{
		bzero(&msg_tmp,sizeof(msg_tmp));
		ret = recv_msg(msg_sock_fd,&msg_tmp);
			
		if(ret < 0)
			continue;
		else {
			msg_new = (struct msg_info *)s_malloc(sizeof(struct msg_info));
			memcpy(msg_new,&msg_tmp,sizeof(struct msg_info));
		}
		add_tail_msg(&ipmsg_msglist,msg_new);
	}
}

void *msg_processor(void *arg)
{
	struct msg_info *msg_peer,msg;
	unsigned long msg_mode,msg_opt;

	while(1)
	{
		msg_peer = get_head_msg(&ipmsg_msglist);
		if(msg_peer == NULL)
		{
			printf("MSG GET error!\n");
			continue;
		}
		
		bzero(&msg,sizeof(msg));
		init_msg(&msg,IPMSG_NOOPERATION);
		memcpy(&msg.peer,&msg_peer->peer,sizeof(msg.peer));
				
		msg_mode = GET_MODE(msg_peer->command);
		msg_opt = GET_OPT(msg_peer->command);
		
		if(msg_opt & IPMSG_SENDCHECKOPT) {
			snprintf(msg.extended,LEN_MSG,"%u",msg_peer->packet_no);
			send_command(msg_sock_fd,&msg,IPMSG_RECVMSG);
		}
		
		switch(msg_mode)
		{
		case IPMSG_BR_ENTRY:
		//	printf("\nProcessing IPMSG_BR_ENTRY\n");
			init_msg(&msg,IPMSG_ANSENTRY);
			strncpy(msg.extended,local_user->pw_name,sizeof(msg.extended));
			memcpy(&msg.peer,&msg_peer->peer,sizeof(&msg.peer));
			send_msg(msg_sock_fd,&msg);
			add_user(&ipmsg_usrlist,msg_peer);
			break;
			
		case IPMSG_ANSENTRY:
		//	printf("\nProcessing IPMSG_ANSENTRY\n");
			add_user(&ipmsg_usrlist,msg_peer);
			break;
			
		case IPMSG_SENDMSG:
		//	printf("\nProcessing IPMSG_SENDMSG\n");
			if(strlen(msg_peer->extended))
			{
				printf("\n>Message from:%s(%s)\n",msg_peer->sender,msg_peer->host);
				printf("MSG_ID[$%d]:\n%s\n",msg_peer->packet_no,msg_peer->extended);
			}
			/*备用，留作文件附件选项检查*/
			break;
		
		case IPMSG_BR_EXIT:
		//	printf("\nProcessing IPMSG_BR_EXIT\n");
			del_user(&ipmsg_usrlist,msg_peer);
			break;
			
		case IPMSG_NOOPERATION:
		//	printf("\nProcessing IPMSG_NOOPERATION\n");
			break;
			
		case IPMSG_RECVMSG:
		//	printf("\nProcessing IPMSG_RECVMSG\n");
			printf("Message is received by %s(%s).MSG_ID:$%s\n",
				msg_peer->sender,msg_peer->host,msg_peer->extended);
			break;
		
		default:
			printf("HAVEN'T IMPLEMENTED COMMAND:%x\n",msg_peer->command);
			break;
		}
		
		free(msg_peer);
		msg_peer = NULL;

	}
}

void *user_checker(void *arg)
{
	struct user_info *pre,*cur;

	pre = &ipmsg_usrlist;
	cur = pre->next;
	while(1){
		pthread_mutex_lock(&MUTEX_USERLIST);
		while(cur != NULL)
		{
			if(cur->exit == 1)
			{
				pre->next=cur->next;
				free(cur);
			}
			else
				pre = cur;
			cur = pre->next;
		}
		pthread_mutex_unlock(&MUTEX_USERLIST);
		sleep(30);
	}
}
