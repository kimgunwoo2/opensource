/*
 * clientPost.c: A very, very primitive HTTP client for sensor
 * 
 * To run, prepare config-cp.txt and try: 
 *      ./clientPost
 *
 * Sends one HTTP request to the specified HTTP server.
 * Get the HTTP response.
 */


#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/time.h>
#include "stems.h"
#include "stems.c"

#define MSG_SIZE 80

void clientSend(int fd, char *filename, char *body)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "POST %s HTTP/1.1\n", filename);
  sprintf(buf, "%sHost: %s\n", buf, hostname);
  sprintf(buf, "%sContent-Type: text/plain; charset=utf-8\n", buf);
  sprintf(buf, "%sContent-Length: %d\n\r\n", buf, (int)strlen(body));
  sprintf(buf, "%s%s\n", buf, body);
  Rio_writen(fd, buf, strlen(buf));
}
  
/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];  
  int length = 0;
  int n;
  
  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1)
      printf("Length = %d\n", length);
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

/* currently, there is no loop. I will add loop later */
void userTask(char *myname, char *hostname, int port, char *filename, char* time, float value)
{
  int clientfd;
  char msg[MAXLINE];

  sprintf(msg, "name=%s&time=%s&value=%f", myname, time, value);
  clientfd = Open_clientfd(hostname, port);
  clientSend(clientfd, filename, msg);
  clientPrint(clientfd);
  Close(clientfd);
}

void getargs_cp( char *hostname, int *port, char *filename, float *threshold)
{
  FILE *fp;

  fp = fopen("config-pc.txt", "r");
  if (fp == NULL)
    unix_error("config-pc.txt file does not open.");

  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", filename);
  fscanf(fp, "%f", threshold);
  fclose(fp);
}

int main(void)
{
  char *tempData, *tempValue;
  int readf;
  char *myname, hostname[MAXLINE], filename[MAXLINE];
  char *timen;
  int port;
  float time, value;
  float threshold;
  int fd;
  char body[MAXLINE];
  char buf[MAXLINE];
  char *fifo_name=getenv("FIFO_NAME");

  int filedes;
  int nread;

  // named pipe create

  if(mkfifo(fifo_name,0666)==-1){
	printf("AlarmServer :: fail mkfifo()\n");
        unlink(fifo_name);
	exit(1);
 }

  // name pipe open
  if((filedes = open(fifo_name, O_RDWR)) < 0){

        printf("AlarmServer :: fail to call fifo()\n");
  	unlink(fifo_name);
        exit(1);
  }

  // read 출력내용 여기서 대기 하더
  if((nread = read(filedes, body, 10)) < 0 ){
        printf("AlarmServer :: fail to call read()-1\n");
	unlink(fifo_name);
	exit(1);
  }
  
  sleep(1);
  
  if((nread = read(filedes, buf, (atoi(body)+1))) < 0 ){
        printf("AlarmServer :: fail to call read()-2\n");
  	unlink(fifo_name);
   	exit(1);
  }

  // named pipe delete
  unlink(fifo_name);


  tempData = strtok(buf,"&");
  
  // Name 저장
  tempValue = strstr(tempData,"=");
  myname = ++tempValue;
  tempData = strtok(NULL,"&");
  // Time 저장
  tempValue = strstr(tempData,"=");
  timen = ++tempValue;
  tempData = strtok(NULL,"&");
  // Value 저장
  tempValue = strstr(tempData,"=");
  value = atof(++tempValue);
  tempData = strtok(NULL,"&");

  getargs_cp(hostname, &port, filename, &threshold);
  if(value>threshold)
  userTask(myname, hostname, port, filename, timen, value);
  
  return(0);
}
