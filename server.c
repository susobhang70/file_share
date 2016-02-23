#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <openssl/md5.h>
#include <stddef.h>
#include <dirent.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdlib.h>
#include <syscall.h>

#define server_port 27800
#define MAX_LINE 256


int recv_wrapper(int sock, void *recv_data, int len, int flag) 
{
	int rv = -1;
	set_socket_blocking_enabled(sock,false);
	while( (rv = recv(sock,recv_data,len,flag)) < 0 )
		;
	set_socket_blocking_enabled(sock,true);
	return rv;
}


char * server_receive_buffer = NULL;
char * server_send_buffer = NULL;

int main(int argc, char * argv[])
{
	struct sockaddr_in server_addr;
	int sockfd, sock, connection;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		printf("UNABLE TO RETRIEVE SOCKET");
		exit(EXIT_FAILURE);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero), 8);

	if(bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)  
	{
		printf("UNABLE TO BIND SOCKET");
		exit(EXIT_FAILURE);
	}

	if (listen(sock, 10) == -1) 
	{
		printf("UNABLE TO LISTEN ON SOCKET");
		exit(EXIT_FAILURE);
	}

	server_send_buffer = (char *) calloc(sizeof(char) , 1024);
	server_receive_buffer = (char *) calloc(sizeof(char), 1024);

	while(1)
	{
		connection = accept(sock, (struct sockaddr *)&client_addr, temp_sock_len);

		if(connection == -1)
			printf("You're screwed\n");
		while(1) 
		{
			memset(server_send_buffer, 0, 1024*sizeof(char));
			memset(server_receive_buffer, 0, 1024*sizeof(char));

			bytes_recv = recv_wrapper(connection, server_receive_buffer, 1024, 0);
			server_receive_buffer[bytes_recv] = '\0';
		}

	}

    return 0;
}