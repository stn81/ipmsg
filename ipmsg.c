#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <locale.h>
#include <langinfo.h>
#include <sys/utsname.h>
#include <pwd.h>


#include "ipmsg.h"
#include "msglist.h"
#include "userlist.h"
#include "staff.h"
#include "udp_comm.h"
#define MSG_MAX 100

struct passwd *local_user;
struct utsname local_host;
int utf8=0;


int main(int argc,char **argv)
{

	pthread_t guide_t,msg_recv_t,msg_proc_t,usr_check_t;
	uname(&local_host);
	local_user = getpwuid(getuid());

	if(setlocale(LC_CTYPE,""))
	{
		if(!strcmp(nl_langinfo(CODESET),"UTF-8"))
			utf8=1;
	}
	
	INIT_MSGLIST(ipmsg_msglist);
	INIT_USERLIST(ipmsg_usrlist);
	sem_init(&NE_MSGLIST,0,0);
	sem_init(&NF_MSGLIST,0,MSG_MAX);
	
	init_socket();
	login();
	pthread_create(&guide_t,NULL,&program_guide,NULL);
	pthread_create(&msg_recv_t,NULL,&msg_receiver,&msg_sock_fd);
	pthread_create(&msg_proc_t,NULL,&msg_processor,&msg_sock_fd);
	pthread_create(&usr_check_t,NULL,&user_checker,NULL);
	
	pthread_join(guide_t,NULL);
	pthread_join(msg_recv_t,NULL);
	pthread_join(msg_proc_t,NULL);
	pthread_join(usr_check_t,NULL);
	return 0;
}
	
	
	
	
	
