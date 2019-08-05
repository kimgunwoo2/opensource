#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include </usr/include/mysql/mysql.h>
#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PASS "0401"
#define DB_NAME "opensource"

//
// This program is intended to help you test your web server.
// You can use it to test that you are correctly having multiple 
// threads handling http requests.
//
// htmlReturn() is used if client program is a general web client
// program like Google Chrome. textReturn() is used for a client
// program in a embedded system.
//
// Standalone test:
// # export QUERY_STRING="name=temperature&time=3003.2&value=33.0"
// # ./dataGet.cgi
void myfunc(char *content){
  char *buf;
  int find=0;
  char *cmd,*arg1,*arg2;
   buf = getenv("QUERY_STRING");
 
   MYSQL mysql ;
   MYSQL_ROW row ;
   strtok(buf,"=");

   cmd = strtok(NULL,"&");

   strtok(NULL,"=");

   arg1 = strtok(NULL,"&");

   strtok(NULL,"=");

   arg2 = strtok(NULL,"&");
     mysql_init(&mysql) ;
    if(!mysql_real_connect(&mysql, NULL, "root","0401", NULL ,3306, (char *)NULL, 0)){
	printf("%s\n",mysql_error(&mysql));
	exit(1);
   }

   if(mysql_query(&mysql, "USE opensource")){
	printf("%s\n", mysql_error(&mysql));
	exit(1);
   }
 if(strcmp(cmd, "LIST")==0){
	if(mysql_query(&mysql, "select * from sensorList")){
	printf("%s\n", mysql_error(&mysql));
	exit(1);
   	}
      	MYSQL_RES *result = mysql_store_result(&mysql);
	 while((row = mysql_fetch_row(result))!=NULL)
   	   sprintf(content, "%s %s\t", content, row[1]);
       
 	 sprintf(content, "%s \r\n", content);
  }

 

  else if(strcmp(cmd, "INFO")==0){
	if(mysql_query(&mysql, "select * from sensorList")){
	printf("%s\n", mysql_error(&mysql));
	exit(1);
   	}
      	MYSQL_RES *result = mysql_store_result(&mysql);
	 while((row = mysql_fetch_row(result))!=NULL){
   	   if(!strcmp(arg1,row[1])){
		sprintf(content, "%s count  averate\r\n", content);
		sprintf(content, "%s %s\t%s\r\n", content,row[3],row[4]);
		find=1;
	   }
	}
   	if(find==0){
		sprintf(content, "%s There is no sensor named '%s'.\r\n", content,arg1);	
	}
	find=0;

  }

 

  else if(strcmp(cmd, "GET")==0){
	int id,count;char temp[MAXLINE];
	int d_count=0;
    	if(mysql_query(&mysql, "select * from sensorList")){
	printf("%s\n", mysql_error(&mysql));
	exit(1);
   	}
      	MYSQL_RES *result = mysql_store_result(&mysql);
	 while((row = mysql_fetch_row(result))!=NULL){
   	   if(!strcmp(arg1,row[1])){
		id=atoi(row[2]);
		count=atoi(row[3]);
		find=1;
	   }
	}
	if(find==1){
           sprintf(temp,"select * from sensor%d",id);
	   if(mysql_query(&mysql, temp)){
		printf("%s\n", mysql_error(&mysql));
		exit(1);
   	   }
	   MYSQL_RES *result = mysql_store_result(&mysql);
	 while((row = mysql_fetch_row(result))!=NULL){
		d_count++;
		char temp_time[MAXLINE];
		struct tm* now;
		time_t now_t=atoi(row[0]);
		now=localtime(&now_t);
		strftime(temp_time,MAXLINE,"%a %b %d %T %Y",now);
		if(count-atoi(arg2)<d_count)
			sprintf(content, "%s%s\t%s\r\n", content,temp_time,row[1]);
		
	   }
	
	}
   	if(find==0){
		sprintf(content, "%s There is no sensor named '%s'.\r\n", content,arg1);	
	}
	find =0;
    }
 
  mysql_close(&mysql);

}

void htmlReturn(void)
{
  char content[MAXLINE];

  /* Make the response body */
  
  sprintf(content, "%s<html>\r\n<head>\r\n", content);
  sprintf(content, "%s<title>CGI result</title>\r\n", content);
  sprintf(content, "%s</head>\r\n", content);
  sprintf(content, "%s<body>\r\n", content);
  
  myfunc(content);
  sprintf(content, "%s</body>\r\n</html>\r\n", content);
  
  /* Generate the HTTP response */
  printf("HTTP/1.0 200 OK\r\n");
  printf("Content-Length: %d\r\n", (int)strlen(content));
  printf("Content-Type: text/html\r\n\r\n");
  printf("%s", content);
  fflush(stdout);
}

void textReturn(void)
{
  char content[MAXLINE];
  char *buf;
  char *ptr=NULL;
  buf = getenv("QUERY_STRING");
  sprintf(content,"%sEnv : %s\n", content, buf);
  ptr = strsep(&buf, "&");
  while (ptr != NULL){
    sprintf(content, "%s%s\n", content, ptr);
    ptr = strsep(&buf, "&");
  }
  
  /* Generate the HTTP response */
  printf("Content-Length: %d\n", (int)strlen(content));
  printf("Content-Type: text/plain\r\n\r\n");
  printf("%s", content);
  fflush(stdout);
}

int main(void)
{
  htmlReturn();
  //textReturn();
  return(0);
}
