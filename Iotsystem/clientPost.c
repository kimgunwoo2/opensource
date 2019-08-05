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
#include "stems.h"
#include <time.h>
 
#define STR_LEN 1024

#define MAX_TOKENS 128
 

char myname[STR_LEN], hostname[STR_LEN], filename[STR_LEN];
  int port;
  float time1, value;

struct CMD {                                                

 

char *name;                                                 

 

char *desc;                                               

 

int(*cmd)(int argc, char *argv[]);   

};

 
void getargs_cp(char *myname, char *hostname, int *port, char *filename, float *time1, float *value)
{
  FILE *fp;

  fp = fopen("config-cp.txt", "r");
  if (fp == NULL)
    unix_error("config-cp.txt file does not open.");

  fscanf(fp, "%s", myname);
  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", filename);
  fscanf(fp, "%f", time1);
  fscanf(fp, "%f", value);
  fclose(fp);
}
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
void userTask(char *myname, char *hostname, int port, char *filename, float time2, float value)
{
  int clientfd;
  char msg[MAXLINE]; 
  char time_n[MAXLINE];
  time_t now_time;
  time(&now_time);
  sprintf(time_n,"%d",(int)now_time);
  sprintf(msg, "name=%s&time=%s&value=%f", myname,time_n, value);
  clientfd = Open_clientfd(hostname, port);
  clientSend(clientfd, filename, msg);
  clientPrint(clientfd);
  Close(clientfd);
}
int cmd_quit(int argc, char *argv[]);

int cmd_help(int argc, char *argv[]);

int cmd_value(int argc, char *argv[]);

int cmdProcessing(void);

int cmd_name(int argvc, char *argv[]);

int cmd_send(int argvc, char *argv[]);
 
int cmd_random(int argvc, char *argv[]);


struct CMD builtin[] = {                    

{ "help", "list available commands.", cmd_help },


{ "name", "print current sensor name.", cmd_name },

{ "name <sensor>", "change sensor name to <sensor>.", NULL },

{ "value", "print current value of value of senser.", cmd_value },

{ "value <n>", "set sensor value to <n>.", NULL },

{ "send", "send (current sensor name, time, value) to server.", cmd_send },

{ "random", "send (name, time, random value) to server.", cmd_random },


{ "random <n>", "send (name, time, random value) to server <n> times.", NULL },

{ "quit", "quit the program.", cmd_quit }
};

 

const int builtins = 9;                             

 
void int_handler(int signum) {

printf("\n");

}

int main(void)

{
struct sigaction act;


 getargs_cp(myname, hostname, &port, filename, &time1, &value);

void int_handler(int);

act.sa_handler = int_handler;

sigfillset(&act.sa_mask);

sigaction(SIGINT, &act, NULL);

int isExit = 0;

while (!isExit)

isExit = cmdProcessing();    

fputs("Shell을 종료합니다.\n", stdout);

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

 int cmd_send(int argvc, char *argv[]){
	 userTask(myname, hostname, port, filename, time1, value);
	return 0;
}



int cmd_quit(int argc, char *argv[])

{


return 1;    

}

int cmd_name(int argvc, char *argv[]){
  char buf[STR_LEN];
  if(argvc!=1&&argvc!=2){
     fputs("다시 입력하세요.\n",stdout);
     return 0;
  }else if(argvc==1){
	sprintf(buf,"Current sensor is '%s'",myname);
	fputs(buf,stdout);
	fputs("\n",stdout);
	return 0;
  }else if(argvc==2){
	sprintf(myname,"%s",argv[1]);
	sprintf(buf,"Sensor name is changed to '%s'",myname);
	fputs(buf,stdout);
	fputs("\n",stdout);
	return 0;
  }else{
	return 0;
 }
 	
}
int cmd_value(int argvc, char *argv[]){
  char buf[STR_LEN];
  if(argvc!=1&&argvc!=2){
     fputs("다시 입력하세요.\n",stdout);
     return 0;
  }else if(argvc==1){
	sprintf(buf,"Current value of sensor is %.4f",value);
	fputs(buf,stdout);
	fputs("\n",stdout);
	return 0;
  }else if(argvc==2){
        if(atof(argv[1])==0){
		fputs("다시 입력하세요.\n",stdout);
     		return 0;	
	}
	value=atof(argv[1]);
	sprintf(buf,"Sensor name is changed to %.4f",value);
	fputs(buf,stdout);
	fputs("\n",stdout);
	return 0;
  }else
	return 0;
}
int cmd_random(int argc, char *argv[])
{
	float temp=0;
	if(argc!=2){
		fputs("다시 입력하세요.\n",stdout);
   		return 0;	
	}else{
		srand(time(NULL));
		for(int i=0; i<atoi(argv[1]); i++){
			temp=rand()%20-10+value;
			userTask(myname, hostname, port, filename, time1, temp);
			sleep(1);
			fputs("\n",stdout);		
		}
	return 0;
	}
                                                            

}


int cmd_help(int argc, char *argv[])

{

if (argc == 1) {                                           

for (int i = 0; i < builtins; i++) {   

fputs(builtin[i].name, stdout);

fputs(": ", stdout);

fputs(builtin[i].desc, stdout);

fputs("\n", stdout);

}                                                            

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

