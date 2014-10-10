/*************************************************************************
    > File Name: tiny.c
    > Author: Morain
    > Mail: morainchen135@gmail.com
    > Created Time: 2014年10月09日 星期四 19时08分23秒
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "RIO.h"
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#define LISTENQ 1024
#define MAXLINE 1024
typedef struct sockaddr SA;
extern char ** environ;
int logfd;

void doit(int fd);
void read_requesthdrs(rio_t * rp);
int parse_uri(char * uri , char * filename , char * cgiarg);
void clienterror(int fd , char * cause , char * errnum , char * shortmsg , char * longmsg);
void serv_static(int fd , char * filename ,int filesize);
void serv_dynamic(int fd , char * filename , char *cgiarg);
void get_filetype(char * filename ,char * filetype);
void handler(int);


void get_filetype(char * filename ,char * filetype)
{
	if(strstr(filename , ".html"))
		strcpy(filetype ,"text/html");
	else if(strstr(filename , ".gif"))
		strcpy(filetype , "image/gif");
	else if(strstr(filename , ".jpg"))
		strcpy(filetype , "image/jpeg");
	else
		strcpy(filetype , "text/plain");
}
void serv_static(int fd , char * filename ,int filesize)
{
	int srcfd ;
	char *srcp , filetype[MAXLINE] ,buf[MAXLINE];

	get_filetype(filename , filetype);

	sprintf(buf , "HTTP/1.0 200 OK\r\n");
	sprintf(buf , "%sServer:Tiny Web Server\r\n" , buf);
	sprintf(buf , "%sContent-length:%d\r\n" , buf , filesize);
	sprintf(buf , "%sContent-type:%s\r\n\r\n" , buf ,filetype);
	rio_writen(fd , buf , strlen(buf));

	srcfd = open(filename , O_RDONLY , 0);
	srcp = mmap(0 , filesize , PROT_READ , MAP_PRIVATE , srcfd , 0);
	close(srcfd);
	rio_writen(fd , srcp , filesize);
	munmap(srcp , filesize);
}

void serv_dynamic(int fd , char * filename , char *cgiarg)
{
	char buf[MAXLINE] , *emptylist[] = {NULL};

	sprintf(buf , "HTTP/1.0 200 OK\r\n");
	rio_writen(fd , buf , strlen(buf));
	sprintf(buf , "Server:Tiny Web Server\r\n");
	rio_writen(fd , buf , strlen(buf));


	if(fork() == 0)
	{
		setenv("QUERY_STRING" , cgiarg , 1);
		dup2(fd , STDOUT_FILENO);
		execve(filename , emptylist , environ);
	}
}


void clienterror(int fd , char * cause , char * errnum , 
	char * shortmsg , char * longmsg)
{
	char buf[MAXLINE] , body[MAXLINE];
	sprintf(body , "<html><title>Tiny Error</title>");
	sprintf(body , "%s<body bgcolor=""ffffff"">\r\n",body);
	sprintf(body , "%s%s : %s \r\n" , body ,errnum , shortmsg);
	sprintf(body , "%s<p>%s: %s\r\n" , body ,longmsg ,cause);
	sprintf(body , "%s<hr><em>The Tiny Web Server</em>\r\n" , body);


	sprintf(buf , "HTTP/1.0 %s %s \r\n" , errnum , shortmsg);
	rio_writen(fd , buf ,strlen(buf));
	sprintf(buf , "Content-type: text/html\r\n");
	rio_writen(fd , buf ,strlen(buf));
	sprintf(buf , "Content-length: %d\r\n\r\n",(int)strlen(body));
	rio_writen(fd , buf ,strlen(buf));
	rio_writen(fd , body ,strlen(body));
}
int parse_uri(char * uri , char * filename , char * cgiarg)
{
	char *ptr;
	if( !strstr(uri , "cgi-bin") )
	{
		strcpy(cgiarg , "");
		strcpy(filename , ".");
		strcat(filename , uri);
		if(uri[strlen(uri) -1] == '/')
			strcat(filename , "home.html");
		return 1;
	}
	else
	{
		ptr = index(uri , '?');
		if(ptr)
		{
			strcpy(cgiarg , ptr+1);
			*ptr = '\0';
		}
		else
			strcpy(cgiarg , "");
		strcpy(filename , ".");
		strcat(filename , uri);
		return 0;
	}
}
void read_requesthdrs(rio_t * rp)
{
	char buf[MAXLINE];
	rio_readlineb(rp , buf , MAXLINE);
	while(strcmp(buf , "\r\n"))
	{
		//printf("%s\n",buf);
		rio_writen(logfd , buf , strlen(buf));
		rio_readlineb(rp , buf ,MAXLINE);
	}
	sprintf(buf , "\r\n\r\n");
	rio_writen(logfd , buf , strlen(buf));
	return;
}
void doit(int fd)
{
	int is_static;
	struct stat sbuf;
	char method[MAXLINE] , uri[MAXLINE] , version[MAXLINE];
	char filename[MAXLINE] , cgiarg[MAXLINE];
	char buf[MAXLINE];
	char timestr[MAXLINE];
	rio_t rio;
	rio_readinitb(&rio , fd);
	rio_readlineb(&rio , buf , MAXLINE);
	sscanf(buf , "%s %s %s" , method , uri , version);

	logfd = open("log" , O_RDWR|O_APPEND|O_CREAT , S_IRUSR|S_IWUSR);
	struct timeval tv;
	gettimeofday(&tv , NULL);
	sprintf(timestr , "%s\n" , ctime(&tv.tv_sec));
	rio_writen(logfd , timestr , strlen(timestr));
	rio_writen(logfd , buf , strlen(buf));
	
	if(strcasecmp(method , "GET"))
	{
		clienterror(fd , method , "501" , "Not Implemented" 
			,"Tiny has not implemented this method");
		return;
	}
	read_requesthdrs(&rio);

	is_static = parse_uri(uri , filename , cgiarg);

	if(stat(filename , &sbuf) < 0)
	{
		clienterror(fd , filename , "404" ,"Not Found" 
			, "Tiny can't find this file");
		return;
	}

	if(is_static)
	{
		if( !(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode ) )
		{
			clienterror(fd , filename , "403" , "Forbidden"
				, "Tiny can't read this file");
			return;
		}
		serv_static(fd , filename , sbuf.st_size);
	}
	else
	{
		if( !(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode) )
		{
			clienterror(fd , filename , "403" , "Forbidden"
				, "Tiny can't run this program");
			return;
		}
		serv_dynamic(fd , filename , cgiarg);
	}

}

void handler(int sig)
{
	wait(NULL);
}
int main(int argc, char const *argv[])
{
	int port , listenfd , connfd , clientlen;
	pid_t pid;
	struct sockaddr_in clientaddr , serveraddr;
	signal(SIGCHLD , handler);
	if(argc != 2)
	{
		fprintf(stderr, "usage : %s  <port> \n",argv[0]);
		exit(1);
	}

	if((listenfd = socket(AF_INET , SOCK_STREAM , 0)) < 0 )
	{
		perror("socket error");
		return -1;
	}

	port = atoi(argv[1]);

	bzero(&serveraddr , sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(port);

	if((bind(listenfd , (SA*)&serveraddr , sizeof(serveraddr) ) ) < 0 )
	{
		perror("bind error");
		return -1;
	}
	if(listen(listenfd , LISTENQ) < 0)
	{
		perror("listen error");
		return -1;
	}

	while(1)
	{
		clientlen = sizeof(clientaddr);
		if( (connfd = accept(listenfd , (SA *)&clientaddr , &clientlen)) < 0)
		{
			perror("accept error");
			return -1;
		}
		if( (pid = fork()) == 0)
		{
			close(listenfd);
			doit(connfd);
			close(connfd);
			exit(0);
		}
		close(connfd);
	}
	return 0;
}