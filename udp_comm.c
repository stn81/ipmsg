#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <pwd.h>
#include "ipmsg.h"
#include "udp_comm.h"
#include "utils.h"

int msg_sock_fd;
int file_sock_fd;
/*change here to send individually*/
char broad_cast_addr[16]="255.255.255.255";

void init_socket()
{
	struct sockaddr_in local_addr;
	const int broadcast_on = 1;
	
	msg_sock_fd = socket(AF_INET,SOCK_DGRAM,0);
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(IPMSG_DEFAULT_PORT);
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(setsockopt(msg_sock_fd,SOL_SOCKET,SO_BROADCAST,&broadcast_on,sizeof(broadcast_on)) < 0)
	{
		fprintf(stderr,"setsockopt() failed:%s\n",strerror(errno));
		close(msg_sock_fd);
		exit(1);
	}
	
	if(bind(msg_sock_fd,(struct sockaddr *)&local_addr,sizeof(local_addr)) < 0)
	{
		fprintf(stderr,"bind() failed:%s\n",strerror(errno));
		close(msg_sock_fd);
		exit(1);
	}
}

int send_msg(int msg_sock_fd,struct msg_info *msg)
{
	char msg_buf[LEN_DGRAM];
	int ret;
	socklen_t addr_len;
	
	addr_len = sizeof(msg->peer);	
	if((msg_sock_fd < 0)||(msg == NULL)) {
		printf("BAD arugment to call send_msg().\n");
		return -1;
	}
	form_msg(msg_buf,LEN_DGRAM,msg);
	
	ret = sendto(msg_sock_fd,msg_buf,sizeof(msg_buf),0,(struct sockaddr *)&msg->peer,addr_len);
	if(ret < 0) {
		perror("sendto()");
	}
	return ret;
}
	
int recv_msg(int msg_sock_fd,struct msg_info *msg)
{
	char msg_buf[LEN_DGRAM];
	int ret;
	socklen_t addr_len;
	struct sockaddr_in peer_addr;
	
	//bzero(msg_buf,sizeof(msg_buf));
	addr_len = sizeof(peer_addr);
	ret = recvfrom(msg_sock_fd,msg_buf,sizeof(msg_buf),0,(struct sockaddr *)&peer_addr,&addr_len);
	

//	printf("\nmsg from :%s\n",inet_ntoa(peer_addr.sin_addr));
//	printf("received:%s\n",msg_buf);
	if(ret < 0)
		return -1;
	parse_msg(msg_buf,sizeof(msg_buf),msg);
	memcpy(&msg->peer,&peer_addr,sizeof(msg->peer));
	return ret;
}

int login()
{
	struct msg_info msg;
	init_msg(&msg,IPMSG_NOOPERATION);
	msg.peer.sin_family = AF_INET;
	msg.peer.sin_port = htons(IPMSG_DEFAULT_PORT);
	if(inet_pton(AF_INET,broad_cast_addr,&msg.peer.sin_addr) < 0)
		printf("login:%s\n",strerror(errno));
		
	/*send IPMSG_NOOPERATION*/
	send_command(msg_sock_fd,&msg,IPMSG_NOOPERATION);
	
	/*send IPMSG_BR_ENTRY*/
	strncpy(msg.extended,local_user->pw_name,LEN_MSG);
	send_command(msg_sock_fd,&msg,IPMSG_BR_ENTRY|IPMSG_DIALUPOPT);
	return 0;
}

int logout()
{
	struct msg_info msg;


	init_msg(&msg, IPMSG_NOOPERATION);
  	msg.peer.sin_family = AF_INET;
	msg.peer.sin_port = htons(IPMSG_DEFAULT_PORT);
  
	if (inet_pton(AF_INET,broad_cast_addr , &msg.peer.sin_addr)<0)
	printf("logout: error\n");

	send_command(msg_sock_fd,&msg,IPMSG_NOOPERATION);
	strncpy(msg.extended, local_user->pw_name, LEN_MSG);
  	send_command(msg_sock_fd,&msg,IPMSG_BR_EXIT);
	printf("Good Bye!\n");
	return 0;
}

/*Use current msg_info struct to send COMMAND,unchange its original command flags*/
void send_command(int msg_sock_fd,struct msg_info *msg,unsigned int commands)
{
	unsigned int flags = msg->command;
	msg->command &= IPMSG_NOOPERATION;
	msg->command |= commands;
	send_msg(msg_sock_fd,msg);
	msg->command = flags;
}

