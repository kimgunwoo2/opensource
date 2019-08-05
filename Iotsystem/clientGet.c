/*
 * clientGet.c: A very, very primitive HTTP client for console.
 * 
 * To run, prepare config-cg.txt and try: 
 *      ./clientGet
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * For testing your server, you will want to modify this client.  
 *
 * When we test your server, we will be using modifications to this client.
 *
 */


#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include "stems.h"
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#include <signal.h>

#define STR_LEN 1024

#define MAX_TOKENS 128
/*
 * Send an HTTP request for the specified file 
 */
char hostname[MAXLINE], webaddr[MAXLINE];
  char sendcmd[MAXLINE];
  int port;
int cmd_quit(int argc, char *argv[]);
int cmd_list(int argc, char *argv[]);
int cmd_help(int argc, char *argv[]);
int cmd_get(int argc, char *argv[]);
int cmd_info(int argc, char *argv[]);
int cmdProcessing(void);


struct CMD {                                                

char *name;                                                 

char *desc;                                               

int(*cmd)(int argc,char *argv[]);   

};
struct CMD builtin[] = {                    


{ "HELP", "도움말 출력", cmd_help },

{ "LIST", "sensor들의 목록 출력", cmd_list },

{ "INFO", "<sname>  sensor <sname>의 정보 출력", cmd_info },

{"GET", "sensor <sname>의 가장 최근 (time,value)", cmd_get},

{"GET <sname> <n>", "sensor <sname>의 가장 최근 (time,value)", NULL},

{"quit", "clientGet 프로세스를 종료한다.", cmd_quit},
{ "list", "sensor들의 목록 출력", cmd_list },

{ "info", "<sname>  sensor <sname>의 정보 출력", cmd_info },

{"get", "sensor <sname>의 가장 최근 (time,value)", cmd_get}
};

 

const int builtins = 9;                           

 
void int_handler(int signum) {

printf("\n");

}

void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
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
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      printf("Length = %d\n", length);
    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

/* currently, there is no loop. I will add loop later */
void userTask(char hostname[], int port, char webaddr[])
{
  int clientfd;

  clientfd = Open_clientfd(hostname, port);
  clientSend(clientfd, webaddr);
  clientPrint(clientfd);
  Close(clientfd);
}

void getargs_cg(char hostname[], int *port, char webaddr[])
{
  FILE *fp;

  fp = fopen("config-cg.txt", "r");
  if (fp == NULL)
    unix_error("config-cg.txt file does not open.");

  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", webaddr);
  fclose(fp);
}

int main(void)
{
  
  getargs_cg(hostname, &port, webaddr);

  struct sigaction act;

pid_t pid =Fork();
 if(pid==0){
	Execve("./pushServer",NULL,NULL);
	return 0;
}
void int_handler(int);

act.sa_handler = int_handler;

sigfillset(&act.sa_mask);

sigaction(SIGINT, &act, NULL);

int isExit = 0;

while (!isExit)

isExit = cmdProcessing();    

fputs("Shell을 종료합니다.\n", stdout);
kill(pid,SIGKILL);
  
  return(0);
}

int cmd_list(int argc, char *argv[]){
	sprintf(sendcmd,"%scommand=LIST",webaddr);
	userTask(hostname,port,sendcmd);
	return 0;
}

int cmd_info(int argc, char *argv[]){
	if(argc!=2){
		printf("다시 입력하세요\n");		
		return 0;
	}
	sprintf(sendcmd,"%scommand=INFO&value=%s",webaddr,argv[1]);
	userTask(hostname,port,sendcmd);
	return 0;
}

int cmd_get(int argc, char *argv[]){
	if(argc==2){
		sprintf(sendcmd,"%scommand=GET&NAME=%s&N=1",webaddr,argv[1]);
		userTask(hostname,port,sendcmd);
	}
	else if(argc==3){
		sprintf(sendcmd,"%scommand=GET&NAME=%s&N=%s",webaddr,argv[1],argv[2]);
		userTask(hostname,port,sendcmd);
	}
	else{
		printf("다시 입력하세요\n");		
	
	}
	return 0;

}
int cmdProcessing(void)
{

char cmdLine[STR_LEN];                      

char *cmdTokens[MAX_TOKENS];       

char delim[] = " \t\n\r";                   

char *token;                                        

int tokenNum;                                        

pid_t pid;                                                

char path[] = "/bin/";                      

int status;                                               

int exitCode = 0;         

 

fputs(">> ", stdout);   

 

memset(cmdLine, '\0', sizeof(cmdLine));

fgets(cmdLine, STR_LEN, stdin);       

tokenNum = 0;

token = strtok(cmdLine, delim);       

 

while (token) {               

 

cmdTokens[tokenNum++] = token;   

token = strtok(NULL, delim);  

}

cmdTokens[tokenNum] = NULL;

if (tokenNum == 0)

return exitCode;

for (int i = 0; i < builtins; ++i) {   

if (strcmp(cmdTokens[0], builtin[i].name) == 0)

return builtin[i].cmd(tokenNum, cmdTokens);

}

pid = fork();             

if (pid > 0)           

wait(&status);        

if (pid == 0) {                                     

strcat(path, cmdTokens[0]);          

execv(path, cmdTokens);                     

exit(0);                                            

}

else if (pid < 0)                                                  

fputs("프로세스 생성 x \n", stdout);      

return exitCode;

}



int cmd_quit(int argc, char *argv[])

{

return 1;    

}
int cmd_help(int argc, char *argv[])

{

if (argc == 1) {                                           

for (int i = 0; i < builtins-3; i++) {   

fputs(builtin[i].name, stdout);

fputs(": ", stdout);

fputs(builtin[i].desc, stdout);

fputs("\n", stdout);

}                                                            
	fputs("\n", stdout);
}

else if (argc > 1) {                                                              

for (int count = 1; count < argc; count++) {          

for (int i = 0; i < builtins; i++) {               

if (strcmp(builtin[i].name, argv[count]) == 0) {  

 

fputs(builtin[i].name, stdout);                                

fputs(": ", stdout);

fputs(builtin[i].desc, stdout);

fputs("\n", stdout);

}

else if ((i == 3) && (strcmp(builtin[i].name, argv[count]) == 0)) {

fputs(argv[count], stdout);                 

fputs(": ", stdout);                       

fputs("해당 명령어는 외부 명령어이거나, 존재하지 않는 명령어입니다.\n", stdout);

break;

}

}

}

}

return 0;     

}  
