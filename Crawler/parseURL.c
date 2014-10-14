#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "RIO.h"

#define BUFFERSIZE 1024
char host[BUFFERSIZE] , URI[BUFFERSIZE];
void parseURL(char * URL )
{
	char * ptr;
	char * start;
	if(strstr(URL , "http://"))
		start = URL + strlen("http://");
	else
		start = URL;


	if( !(ptr = index(start , '/') ) )
	{
		strcpy(host , start);
		return;
	}

	strncpy(host , start , ptr - start);
	strcpy(URI , ptr);
	return;
}
int main(int argc, char const *argv[])
{
	/*if(argc != 2)
	{
		printf("usage : %s <URL>\n",argv[0]);
		return -1;
	}*/
	char buf[BUFFERSIZE];
	//strcpy(buf , argv[1] );
	rio_t rp;
	rio_readinitb(&rp , STDIN_FILENO);
	rio_readlineb(&rp , buf , BUFFERSIZE);
	parseURL(buf);
	printf("host = %s\n",host);
	printf("URI = %s\n",URI);
	fflush(stdout);
	return 0;
}