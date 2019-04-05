#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "utils.h"

void *s_malloc(size_t size)
{
	int retry = 0;
	void *ret;
	do {
		ret = malloc(size);
		if(ret == NULL) {
			retry++;
			fprintf(stderr,"malloc() failed:%s\n",strerror(errno));
			sleep(5);
		}
	}while((ret == NULL)&&(retry < 3));
	return ret;
}


char *fget_str(char  *s, int n, FILE * stream)
{
	char str_buf[n];
	char *s_ptr,*e_ptr;

	bzero(str_buf,sizeof(str_buf));
	fgets(str_buf,n,stream);
	
	s_ptr = str_buf;
	while(*s_ptr == ' '){
		s_ptr++;
	}
	
	e_ptr = s_ptr;
	while((*e_ptr != '\0')&&(*e_ptr != '\n'))
		e_ptr++;
	if(e_ptr <= &str_buf[n-1])
		*e_ptr = '\0';
	else
		str_buf[n-1] = '\0';

	if(strlen(s_ptr) == 0)
		return NULL;
	else {
		bzero(s,n);
		strncpy(s,s_ptr,strlen(s_ptr));
	}
	return s_ptr;
}

