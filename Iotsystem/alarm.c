#include "stems.h"
#include "stems.c"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

//
// This program is intended to help you test your web server.
// 
void htmlReturn(char *buf)
{
  char content[MAXLINE];

  /* Make the response body */
  
  sprintf(content, "%s<html>\r\n<head>\r\n", content);
  sprintf(content, "%s<title>CGI result</title>\r\n", content);
  sprintf(content, "%s</head>\r\n", content);
  sprintf(content, "%s<body>\r\n", content);
  sprintf(content, "%s%s\r\n", content,buf);
 
  sprintf(content, "%s</body>\r\n</html>\r\n", content);
  
  /* Generate the HTTP response */
  printf("%s", content);
  fflush(stdout);
}
int main(int argc, char *argv[])
{
 
  char *buf;
  char *name,*time,*value;
  float fvalue;
  char restbody[MAXBUF];
  char temp[MAXBUF];
  char temp_time[MAXBUF];
  buf = getenv("REST_CONTENTS");
  if(atoi(getenv("Content-Length"))>strlen(buf)){
	rio_t rio;
  	Rio_readinitb(&rio,STDIN_FILENO);
	Rio_readnb(&rio,restbody,strlen(buf)-atoi(getenv("Content-Length")));
	sprintf(buf,"%s%s",buf,restbody);
  }
  strtok(buf,"=");
  name=strtok(NULL,"&");

  strtok(NULL,"=");
  time=strtok(NULL,"&");
 
  strtok(NULL,"=");
  value=strtok(NULL,"&");
  fvalue=atof(value);
  struct tm* now;
  time_t now_t=atoi(time);
  now=localtime(&now_t);
  strftime(temp_time,MAXLINE,"%a %b %d %T %Y",now);
  sprintf(temp,"Warning %s sensor 에 %s 시간에 %.2f라는 값이 발생하였습니다.",name,temp_time,fvalue);
  printf("HTTP/1.0 200 OK\r\n");
  printf("Content-Length: %d\r\n", (int)strlen(temp));
  printf("Content-Type: text/plain\r\n\r\n");
  
  htmlReturn(temp);
  fflush(stdout);
  return(0);
}
