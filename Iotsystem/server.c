#include "stems.h"
#include "request.h"
#include <pthread.h>
#include <semaphore.h>
// 
// To run:
// 1. Edit config-ws.txt with following contents
//    <port number>
// 2. Run by typing executable file
//    ./server
// Most of the work is done within routines written in request.c
//


sem_t start,Full,Empty;
int q_size,thread_size;
typedef struct Node{

	int fd;
	long arrivalTime;
}Node;

int in=0,count=0,out=0;
Node *buffer;
int threadpool_size;

int buffer_full(){
	if(q_size==count)
		return 1;
	else
		return 0;
}

int buffer_empty(){
	if(count==0)
		return 1;
	else 
		return 0;
}
void push_buffer(Node data_num){
	buffer[in]=data_num;
	in=(in+1)%q_size;
	count++;
	
}
Node pop_buffer(){
	
	Node data;
	data = buffer[out];
	out=(out+1)%q_size;
	count--;
	return data;
}
void getargs_ws(int *port,int *thread_size,int *q_size)
{
  FILE *fp;

  if ((fp = fopen("config-ws.txt", "r")) == NULL)
    unix_error("config-ws.txt file does not open.");

  fscanf(fp, "%d", port);
  fscanf(fp, "%d", thread_size);
  fscanf(fp, "%d", q_size);
  fclose(fp);
}

void consumer(int connfd, long arrivalTime,int id)
{
  requestHandle(connfd, arrivalTime,id);
  Close(connfd);
}

void *exec_thread(void *data){
 Node node;
 int id= *(int *)data;
 while(1){
	sem_wait(&Empty);
  	sem_wait(&start);
	node= pop_buffer();
	sem_post(&start);
	sem_post(&Full);
	consumer(node.fd,node.arrivalTime,id);
	

 	

 }
}

int main(void)
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;

  pthread_t *thread_id;
  int *arr;

  initWatch();
  getargs_ws(&port,&thread_size,&q_size);
  listenfd = Open_listenfd(port);

  arr=(int*)malloc(sizeof(int)*thread_size);
  buffer=(Node *)malloc(sizeof(Node)*q_size);
  thread_id=(pthread_t*)malloc(sizeof(pthread_t)*thread_size);


   sem_init(&start,0,1);
   sem_init(&Full,0,q_size);
   sem_init(&Empty,0,0);
 
  for(int i=0; i<thread_size; i++){
	arr[i]=i;
	pthread_create(&thread_id[i],NULL,&exec_thread,(void*)&arr[i]);
	pthread_detach(thread_id[i]);
}
   
 
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    

    sem_wait(&Full);
    sem_wait(&start);
    Node temp ={connfd,getWatch()};
    push_buffer(temp);
    sem_post(&start);
    sem_post(&Empty);
 	
	
  }
  return(0);
}
