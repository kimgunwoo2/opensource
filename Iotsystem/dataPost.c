#include "stems.h"
#include "stems.c"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include "/usr/include/mysql/mysql.h"
#include "sys/stat.h"
#include "sys/types.h"
#include <errno.h>

//
// This program is intended to help you test your web server.
// 

#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PASS "0401"
#define DB_NAME "opensource"
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

  char *tempData;
  char *tempValue;
  char *sensorName;
  char *sensorTime;
  double sensorValue;
  char *fifo_name=getenv("FIFO_NAME1");
  char *buf;
  char restbody[MAXBUF]={};
  buf = getenv("REST_CONTENTS");
  printf("HTTP/1.0 200 OK\r\n");\
  printf("Content-Length: %s\r\n", getenv("Content-Length"));
  printf("Content-Type: text/plain\r\n\r\n");
  
  if(atoi(getenv("Content-Length"))>strlen(buf)){
	rio_t rio;
  	Rio_readinitb(&rio,STDIN_FILENO);
	Rio_readnb(&rio,restbody,strlen(buf)-atoi(getenv("Content-Length")));
	sprintf(buf,"%s%s",buf,restbody);
  }

//Step 6




sleep(1);


  int fd;
  char body_size[10]={};

  sprintf(body_size,"%d",(int)strlen(buf));

 // 파일 오픈, named pipe는 alarmclient에서 생성
 if((fd=open(fifo_name,O_WRONLY))<0){
	printf("dataPost :: fail open()%s\n",fifo_name);
	exit(1);
 }

 if(write(fd,body_size,10)==-1){
	printf("dataPost :: fail write()-body_size\n");
	exit(1);
 }

 if(write(fd,buf,strlen(buf)+1)==-1){
	printf("dataPost :: fail write()-buf\n");
	exit(1);
 }

  htmlReturn(buf);  
 
  tempData = strtok(buf,"&");
  
   //sensorName 저장
  tempValue = strstr(tempData,"=");
  sensorName = ++tempValue;
  tempData = strtok(NULL,"&");
  // sensorTime 저장
  tempValue = strstr(tempData,"=");
  sensorTime = ++tempValue;
  tempData = strtok(NULL,"&");
  // sensorValue 저장
  tempValue = strstr(tempData,"=");
  sensorValue = atof(++tempValue);
  tempData = strtok(NULL,"&");

 
   MYSQL mysql ;
   MYSQL_RES* res ;
   MYSQL_ROW row ;
   int fields ;
   char query[254];
   char tableName[254];
   int tableCount =0;

   // DB open
   mysql_init(&mysql) ;

   if(!mysql_real_connect(&mysql, NULL, "root","0401", NULL ,3306, (char *)NULL, 0)){
	printf("%s\n",mysql_error(&mysql));
	exit(1);
   }

   if(mysql_query(&mysql, "USE opensource")){
	printf("%s\n", mysql_error(&mysql));
	exit(1);
   }


   // sensorName check
   sprintf(query,"select exists(select *from sensorList where name=\"%s\")",sensorName);
   if(mysql_query(&mysql, query) ){
	printf("%s\n", mysql_error(&mysql));
	exit(1);
   }
   res = mysql_store_result( &mysql );
   row = mysql_fetch_row( res );
   if(!strcmp(row[0],"0")){
	mysql_free_result( res );


 	sprintf(query,"select count(*) from sensorList");
	if(mysql_query(&mysql, query) ){
		printf("%s\n", mysql_error(&mysql));
		exit(1);
	}
	res = mysql_store_result( &mysql );
	row = mysql_fetch_row( res );
	tableCount = atoi(row[0]);
	mysql_free_result( res );


	sprintf(query,"insert into sensorList values(\"sensor%d\",\"%s\",%d,0,0.0)",tableCount+1,sensorName,tableCount+1);
	if(mysql_query(&mysql, query) ){
		printf("%s\n", mysql_error(&mysql));
		exit(1);
	}

	sprintf(query,"create table sensor%d(time VARCHAR(30),value DOUBLE,idx INT)",tableCount+1);
	if(mysql_query(&mysql, query) ){
		printf("%s\n", mysql_error(&mysql));
		exit(1);
   	}
   }
   else{
	mysql_free_result( res );
   }

   sprintf(query,"select sensor from sensorList where name=\"%s\"",sensorName);
   if(mysql_query(&mysql, query) ){
	printf("%s\n", mysql_error(&mysql));
	exit(1);
   }
   res = mysql_store_result( &mysql );
   row = mysql_fetch_row( res );
   sprintf(tableName,"%s",row[0]);
   mysql_free_result( res );



   sprintf(query,"select count(*) from %s ",tableName);
   if(mysql_query(&mysql, query) ){
	printf("%s\n", mysql_error(&mysql));
	exit(1);
   }
     res = mysql_store_result( &mysql );
     row = mysql_fetch_row( res );
     tableCount = atoi(row[0]);
     mysql_free_result( res );

   mysql_close(&mysql);



   mysql_init(&mysql) ;

   if(!mysql_real_connect(&mysql, NULL, "root","0401", NULL ,3306, (char *)NULL, 0)){
	printf("%s\n",mysql_error(&mysql));
	exit(1);
   }

   if(mysql_query(&mysql, "USE opensource")){
	printf("%s\n", mysql_error(&mysql));
	exit(1);
   }

  
 sprintf(query,"insert into %s values(\"%s\", %lf, %d)",tableName,sensorTime,sensorValue,tableCount+1);
   if(mysql_query(&mysql, query)){
	printf("%s\n", mysql_error(&mysql));
	exit(1);
   }

 sprintf(query," update sensorList set count=%d where sensor=\"%s\"",tableCount+1,tableName);
   if(mysql_query(&mysql, query)){
	printf("%s\n", mysql_error(&mysql));
	exit(1);
   }
     

 double average;
 int aveCount = 0;
 sprintf(query,"select value from %s",tableName);
   if(mysql_query(&mysql, query) ){
	printf("%s\n", mysql_error(&mysql));
   }
   else{
	res = mysql_store_result( &mysql );
	fields = mysql_num_fields(res) ;

	while( ( row = mysql_fetch_row( res ) ) ){
		for(int cnt = 0 ; cnt < fields ; ++cnt){
			average += atof(row[cnt]);
                	aveCount++;
        	}
	}
	mysql_free_result( res );
   }
   average = average/ aveCount;   

 sprintf(query,"update sensorList set average=%f where sensor=\"%s\"",average,tableName);
   if(mysql_query(&mysql, query)){
	printf("%s\n", mysql_error(&mysql));
	exit(1);
   }

   mysql_close(&mysql);

  fflush(stdout);
  return(0);
}
