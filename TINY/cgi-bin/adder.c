/*************************************************************************
    > File Name: adder.c
    > Author: Morain
    > Mail: morainchen135@gmail.com
    > Created Time: 2014年10月10日 星期五 08时09分01秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE 1024
int main(int argc, char const *argv[])
{
	char * buf ,*p;
	char arg1[MAXLINE] , arg2[MAXLINE] , content[MAXLINE];
	int n1 = 0 , n2 = 0;

	if( (buf = getenv("QUERY_STRING")) != NULL )
	{
		p = strchr(buf , '&');
		*p = '\0';
		strcpy(arg1 , buf );
		strcpy(arg2 , p+1);
		n1 = atoi(arg1);
		n2 = atoi(arg2);
	}

	sprintf(content , "Welcome to add.com: ");
	sprintf(content , "%sThe Internet addition portal.\r\n<p>" , content);
	sprintf(content , "%sThe answer is : %d + %d = %d\r\n</p>" , 
		content , n1 , n2 , n1 + n2);
	sprintf(content , "%sThanks for visiting!\r\n" , content);


	printf("Content-length : %d\r\n",(int)strlen(content));
	printf("Content-type : text/html\r\n\r\n");
	printf("%s",content);
	fflush(stdout);
	exit(0);

	return 0;
}
