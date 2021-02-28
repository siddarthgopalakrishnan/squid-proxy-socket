/* f20171379@hyderabad.bits-pilani.ac.in Siddarth Gopalakrishnan */

/* Brief description of program...*/
/* 
In this assignment, we establish a socket connection to the squid proxy server.
The socket is created by passing the IP, port number of the proxy and the encoded username and password for auth.
Then an HTTP request is sent and the response received from the server is read into index.html file after removing headers.
Then we send another request to get the logo (this is only if the host is info.in2p3.fr).
The response received from the server is read into a logo.gif file after removing the HTTP headers.
*/

#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>

/*Function to establish a socket connection*/
int createSocket(int sockfd, char* port, char* ip) {
	printf("Establishing socket connection...\n");
	struct sockaddr_in serv_addr;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error while creating socket\n");
		return 1;
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	int portno = atoi(port); // need to pass integer in htons
	serv_addr.sin_port = htons(portno);
	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		printf("inet_pton Error occurred\n");
		return 1;
	}
	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("Error : Couldn't connect\n");
		return 1;
	}
	return sockfd;
}

/*Function to implement substring function*/
char* substr(char* str, int l, int r) {
	char *subBuff = (char*)malloc(sizeof(char)*(r-l+10));
	for(int i=l; (i<r && (*(str+i) != '\0')); i++) {
		*subBuff = *(str+i);
		subBuff++;
	}
	*subBuff = '\0';
	return subBuff - (r-l);
}

/*Function to encode string using base64 scheme*/
char* base64(char str[]) {
	int len = strlen(str);
	char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int base64ind, numOfEquals = 0, stringInd = 0;
	char *finalstr = (char *)malloc(5000 * sizeof(char));
	for(int i=0; i<len; i=i+3) {
		int count = 0, val = 0, no_of_bits = 0;
		for(int j = i; (j<len && j<=i+2); j++) {
			++count;
			val = val << 8;
			val = val | str[j];
		}
		no_of_bits = count;
		no_of_bits = no_of_bits * 8;
		if(count != 3) numOfEquals = no_of_bits % 3;
		while(no_of_bits != 0) {
			if(no_of_bits < 6) {
				base64ind = (val << (6-no_of_bits)) & 63;
				no_of_bits = 0;
			}
			else {
				base64ind = (val >> (no_of_bits-6)) & 63;
				no_of_bits = no_of_bits-6;
			}
			char tempchar = base64chars[base64ind];
			finalstr[stringInd] = tempchar;
			++stringInd;
		}
	}
	for(int i = 1; i <= numOfEquals; i++) finalstr[stringInd++] = '=';
	finalstr[stringInd] = '\0';
	return finalstr;
}

/*Main function*/
int main(int argc, char *argv[]) {
	/*----HTML Part----*/
	// Arguments check
	if(argc > 8) {
		printf("Too many arguments! Terminating...\n");
		return 1;
	}
	if(argc < 8) {
		printf("Too few arguments! Terminating...\n");
		return 1;
	}
	// Declare variables
	int sockfd = 0, n = 0; // sock variable and number of bytes
	char recvBuff[10000], requestBuff[5000], hostname[2000], mainhost[2000];
	strcpy(hostname, argv[1]); strcpy(mainhost, argv[1]);
	// Base64Encoding
	char *creds = (char *) malloc(200 * sizeof(char)); // Auth Credentials
	creds = strcpy(creds, argv[4]); creds = strcat(creds,":"); creds = strcat(creds, argv[5]);
	strcpy(creds, base64(creds));
	// base64 encoded -> Y3NmMzAzOmNzZjMwMw==
	int flag = 0, eoh = 0;
	FILE *fptr = fopen(argv[6], "wb"); // opening file pointer to write
	while(flag != 1) {
		sockfd = createSocket(sockfd, argv[3], argv[2]);
		memset(recvBuff, '\0', sizeof(recvBuff));
		memset(requestBuff, '\0', sizeof(requestBuff));
		n = 0;
		printf("\nTesting GET on host = %s\n", hostname);
		snprintf(requestBuff, sizeof(requestBuff),
			"GET http://%s HTTP/1.1\r\nHost: %s\r\nProxy-Authorization: Basic %s\r\nConnection: close\r\n\r\n",
			hostname, mainhost, creds);
		// Proxy-Connection: Keep-Alive
		printf("%s", requestBuff); // printing what we sent in socket
		printf("Writing to socket...\n");
		memset(recvBuff, '\0', sizeof(recvBuff));
		write(sockfd, requestBuff, strlen(requestBuff)); // writing to the socket
		n = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0);
		recvBuff[n] = 0;
		if(n < 0) {
			printf("Error receiving message from server. Terminating...\n");
			printf("Closing socket connection...\n");
			close(sockfd); // close socket
			return 1;
		}
		// printf("Initial chunk of RecvBuff => \n%s\n", recvBuff); // print headers for reference
		if(strstr(recvBuff, "HTTP/1.1 4") != NULL) { // Bad request
			printf("---Entering 400 series message loop---\n");
			printf("Bad connection, some error has occurred. Terminating...\n");
			printf("Closing socket connection...\n");
			close(sockfd); // close socket
			return 1;
		}
		else if(strstr(recvBuff, "HTTP/1.1 2") != NULL) { // OK accpeted
			printf("---Entering 200 series message loop---\n");
			flag = 1;
			int contentLength;
			while(!eoh) {
				char* rnptr = strstr(recvBuff, "\r\n\r\n");
				if(rnptr) {
					rnptr = rnptr + 4;
					int index = rnptr - recvBuff;
					contentLength = n-index; // total bytes read
					fwrite(rnptr, sizeof(char), contentLength, fptr);
					eoh = 1;
				}
				else {
					n = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0);
				}
			}
			memset(recvBuff, 0, sizeof(recvBuff));
			n = 0;
			while((n = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0)) > 0) { // receiving html
				recvBuff[n] = 0;
				contentLength = n;
				fwrite(recvBuff, sizeof(char), contentLength, fptr);
				memset(recvBuff, 0, sizeof(recvBuff));
			}
			break;
		}
		else if(strstr(recvBuff, "HTTP/1.1 30") != NULL) { // redirects
			printf("---Entering 300 series message loop. Redirecting---\n");
			int contentLength;
			char* ptr = strstr(recvBuff, "Location:");
			if(ptr) { // getting host name
				int stInd = ptr - recvBuff; // start Index - Index of 'L'
				stInd += 10; // Index of first character of location
				char* ptr2 = strstr(ptr, "\r\n"); // index of \r\n
				int edInd = ptr2 - recvBuff;
				char locPath[1500];
				strcpy(locPath, substr(recvBuff, stInd, edInd));
				if(strcasestr(locPath, "https") != NULL) { // Case #1
					printf("\n\nRedirecting to a https website\n");
					printf("Closing socket connection...\n");
					close(sockfd);
					printf("Terminating...\n");
					return 1;
				}
				if(strcasestr(locPath, "http") != NULL) { // Case #2
					int ptr1, ptr2=-1, hostflag = 0;
					for(int i = stInd; i < edInd; i++) {
						if(recvBuff[i] == '/') { // get slash
							if(!hostflag) {
								ptr1 = i + 2; // index from where location starts
								i++;
								hostflag = 1;
								continue;
							}
							else {
								ptr2 = i; // store second slash
								break;
							}
						}
					}
					strcpy(hostname, substr(recvBuff, ptr1, edInd));
					if(ptr2 != -1) strcpy(mainhost, substr(recvBuff, ptr1, ptr2));
					else strcpy(mainhost, hostname);
					printf("Redirected to path : %s\n", hostname);
					printf("Closing socket connection...\n");
					close(sockfd);
					continue;
				}
				if(recvBuff[stInd] == '/') { // Case #3 - /path/ or /path
					char* tempHost = (char*)malloc((strlen(mainhost)+strlen(locPath)+10)*sizeof(char));
					tempHost = strcpy(tempHost, mainhost);
					tempHost = strcat(tempHost, locPath);
					strcpy(hostname, tempHost);
					free(tempHost);
					printf("Redirected to path : %s\n", hostname);
					printf("Closing socket connection...\n");
					close(sockfd);
					continue;
				}
				else { // Case #4 - path/path || path/path/ || host/path || host/path/
					printf("Relative path...\n");
					printf("Closing socket connection...\n");
					close(sockfd);
					printf("Terminating...\n");
					return 1;
					/*
					---To be implemented---
					if(strstr(ptr, '.') != NULL) { // host/path || host/path/
						strcpy(hostname, locPath);
						char* slp = strstr(locPath, "/");
						int slptr = slp - locPath;
						strcpy(mainhost, substr(locPath, stInd, slptr));
						int templen = strlen(hostname);
						if(hostname[templen-1] == '/') hostname[templen-1] = '\0';
					}
					else { // path/path || path/path/
						int curHostLen = strlen(hostname);
						for(int i=curHostLen-1; i>=0; i--) {
							if(hostname[i] == '/') { // Location of last slash
								int k = i+1;
								for(int j=stInd; j<edInd; j++) hostname[k++] = recvBuff[j];
								for(int t = k; t<strlen(hostname); t++) hostname[t] = '\0';
								break;
							}
						}
					}
					printf("Redirected to path : %s\n", hostname);
					printf("Closing socket connection...\n");
					close(sockfd);
					continue;
					*/
				}
			}
		}
		else if(strstr(recvBuff, "HTTP/1.1 50") != NULL) {
			printf("Server side error...\n");
			printf("Closing socket connection...\n");
			close(sockfd);
			return 1;
		}
	}
	fclose(fptr); // close file pointer
	close(sockfd); // close socket
	printf("Finished getting the html file. Closing connection...\n");
	/*----Logo Part----*/
	if(strcmp(argv[1], "info.in2p3.fr") == 0) {
		printf("\n\nGetting image file...\n");
		sockfd = createSocket(sockfd, argv[3], argv[2]);
		char requestBuff[2048]; // buffer for sending request
		FILE *fptr = fopen(argv[6], "r");
		if(fptr == NULL) {
			printf("Couldn't open the html file...\n");
			return 1;
		}
		char imgPath[1000];
		char *htmlline = NULL; size_t numBytes = 0;
		ssize_t nread = getline(&htmlline, &numBytes, fptr);
		while(strcasestr(htmlline, "IMG SRC") == NULL) {
			nread = getline(&htmlline, &numBytes, fptr);
		}
		memcpy(imgPath, htmlline, (int)nread); // getting image path from html
		fclose(fptr);
		char* quoteptr = strstr(imgPath, "\"");
		int ptr1, ptr2;
		if(quoteptr) {
			int index = quoteptr - imgPath;
			ptr1 = index+1;
			for(int i=ptr1; i<strlen(imgPath); i++) {
				if(imgPath[i] == '\"') {
					ptr2 = i;
					break;
				}
			}
		}
		strcpy(imgPath, substr(imgPath, ptr1, ptr2)); // get relative path of the image
		char *finPath = (char *) malloc(100 * sizeof(char)); // Auth Credentials
		finPath = strcpy(finPath, argv[1]); finPath = strcat(finPath,"/"); finPath = strcat(finPath, imgPath);
		// printf("Final image path -> %s\n", finPath); => info.in2p3.fr/cc.gif
		n = 0;
		char imgresponse[100000], temprecv[2];
		memset(imgresponse, '\0', sizeof(imgresponse));
		memset(temprecv, '\0', sizeof(temprecv));
		memset(requestBuff, '\0', sizeof(requestBuff));
		long long int totalBytes = 0;
		char* imageptr = imgresponse;
		printf("\nSubmitting GET on host = %s\n", argv[1]);
		snprintf(requestBuff, sizeof(requestBuff),
			"GET http://%s HTTP/1.1\r\nHost: %s\r\nProxy-Authorization: Basic %s\r\nConnection: close\r\n\r\n",
			finPath, argv[1], creds);
		// Proxy-Connection: Keep-Alive
		printf("%s\n", requestBuff); // printing what we sent in socket
		printf("Writing to socket...\n");
		write(sockfd, requestBuff, strlen(requestBuff)); // writing to the socket
		while((n = recv(sockfd, temprecv, 1, 0)) > 0) { // receiving img from the proxy byte by byte
			memcpy(imageptr, temprecv, sizeof(char));
			++imageptr; ++totalBytes;
			memset(temprecv, 0, sizeof(temprecv));
		}
		printf("Total bytes recevied: %lld...\n", totalBytes); // 32734
		char* rnrnptr = strstr(imgresponse, "\r\n\r\n") + 4;
		int index = rnrnptr - imgresponse; totalBytes -= index;
		fptr = fopen(argv[7], "wb"); // opening file pointer to write
		fwrite(rnrnptr, 1, totalBytes, fptr);
		fclose(fptr); // close file pointer
		close(sockfd); // close socket
		printf("\nFinished getting the image file. Closing connection...\n");
	}
	else printf("Not the specified info.in2p3.fr host. Terminating...\n");
	return 0;
}