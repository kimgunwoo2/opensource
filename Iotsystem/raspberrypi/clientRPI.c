#include "stems.h"
#include "stems.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <wiringPi.h>

#define MAXTIMINGS 60
#define DHTPIN 7
#define PIN_NUM1 27		// RED
#define PIN_NUM1 28		// GREAN

int data[5] = { 0, 0, 0, 0, 0 };


void getargs_cp(char *hostname, int *port, char *filename, int *period, int *max)
{
	FILE *fp;
	fp = fopen("config-pi.txt", "r");
	if (fp == NULL) { unix_error("PIError :: config-pi.txt file not open."); }
	fscanf(fp, "%s", hostname);
	fscanf(fp, "%d", port);
	fscanf(fp, "%s", filename);
	fscanf(fp, "%d", period);
	fscanf(fp, "%d", max);
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

void clientPrint(int fd)
{
	rio_t rio;
	char buf[MAXBUF];
	int length = 0;
	int rio_number;

	Rio_readinitb(&rio, fd);

	// Header Read
	rio_number = Rio_readlineb(&rio, buf, MAXBUF);
	while (strcmp(buf, "\r\n") && (rio_number > 0)) {
		sscanf(buf, "Content-Length: %d ", &length);
		if (sscanf(buf, "Content-Length: %d ", &length) == 1) {}
		printf("Header: %s", buf);
		rio_number = Rio_readlineb(&rio, buf, MAXBUF);
	}

	// Header Body
	rio_number = Rio_readlineb(&rio, buf, MAXBUF);
	while (rio_number > 0) {
		printf("%s", buf);
		rio_number = Rio_readlineb(&rio, buf, MAXBUF);
	}
}

void userTask(char *myname, char *hostname, int port, char *filename, char *value)
{
	char time_n[MAXLINE];
	time_t now_time;
	time(&now_time);
	sprintf(time_n, "%d", (int)now_time);
	int clientfd;
	char msg[MAXLINE];


	sprintf(msg, "name=%s&time=%s&value=%f", myname, time_n, atof(value));


	clientfd = Open_clientfd(hostname, port);
	clientSend(clientfd, filename, msg);
	clientPrint(clientfd);
	close(clientfd);
}

int read_data(char *temperature, char *humidity, int max_temp) {
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;
	uint8_t flag = HIGH;
	uint8_t state = 0;
	float f;
	memset(data, 0, sizeof(int) * 5);
	pinMode(DHTPIN, OUTPUT);
	digitalWrite(DHTPIN, LOW);
	delay(18);
	digitalWrite(DHTPIN, HIGH);
	delayMicroseconds(30);
	pinMode(DHTPIN, INPUT);
	for (i = 0; i < MAXTIMINGS; i++) {
		counter = 0;
		while (digitalRead(DHTPIN) == laststate) {
			counter++;
			delayMicroseconds(1);
			if (counter == 200) break;
		}
		laststate = digitalRead(DHTPIN);
		if (counter == 200) break; // if while breaked by timer, break for   
		if ((i >= 4) && (i % 2 == 0)) {
			data[j / 8] <<= 1;
			if (counter > 20)
				data[j / 8] |= 1;      j++;
		}
	}

	if ((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] +
		data[3]) & 0xff))) {

		if(data[2] > max_temp){
			digitalWrite(PIN_NUM1,1);
			digitalWrite(PIN_NUM2,0);
		}
		else{
			digitalWrite(PIN_NUM1,0);
			digitalWrite(PIN_NUM2,1);
		}

		printf("humidity = %d.%d %% Temperature = %d.%d *C \n", data[0],
			data[1], data[2], data[3]);
		sprintf(humidity, "%d.%02d", data[0], data[1]);
		sprintf(temperature, "%d.%02d", data[2], data[3]);
		return 1;
	}
	else {
		printf("Data get failed\n");
		return 0;
	}

}

int main(void)
{
	char hostname[MAXLINE], filename[MAXLINE];
	int port, period, max_temp;
	char temperature[MAXLINE], humidity[MAXLINE];

	getargs_cp(hostname, &port, filename, &period, &max_temp);

	printf("Raspberry PI On Mode\n");

	if (wiringPiSetup() == -1) { exit(1); }
	
	pinMode(PIN_NUM1, OUTPUT);
	pinMode(PIN_NUM2, OUTPUT);
	
	digitalWrite(PIN_NUM1,0);
	digitalWrite(PIN_NUM2,0);
	
	period *= 1000;

	while (1) {
		delay(period / 2);
		if (read_data(temperature, humidity, max_temp)) {
			userTask("temperaturePI", hostname, port, filename, temperature);
			delay(period / 2);
			userTask("humidityPI", hostname, port, filename, humidity);
		}
	}

	printf("Raspberry PI Off Mode\n");
	return 0;
}