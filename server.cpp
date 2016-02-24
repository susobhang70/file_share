#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <string>

using namespace std;

int connection_type, received_command_length;

char received_data[2048], initial_received_command[2048], received_command[50][100];

void parse_request()
{
	strcpy(initial_received_command, received_data);
	
	received_command_length = 0;
	char *temp;
	temp = strtok(initial_received_command," ,.-");
	
	while(temp != NULL){
		strcpy(received_command[received_command_length], temp);
		received_command_length ++ ;
		temp = strtok(NULL," ,.-");
	}
	
	strcpy(initial_received_command, received_data);
	return;
}

int startServer(int server_port)
{
	int sock, sin_size, link, received_bytes;

	struct sockaddr_in server_address, client_address;

	if(connection_type == 1)
	{
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if(sock == -1)
		{
			perror("Error");
			return 1;
		}
	}

	else if(connection_type == 2)
	{
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if(sock == -1)
		{
			perror("Error");
			return 1;
		}
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server_port);
	server_address.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_address.sin_zero), 8);

	if (bind(sock, (struct sockaddr *)&server_address, sizeof(struct sockaddr)) == -1) 
	{
		perror("Error");
		return 2;
	}

	if(connection_type == 1)
	{
		if (listen(sock, 10) == -1)
		{
			printf("Error: Failed to listen\n");
			return 3;
		}
	}

	printf("Server waiting for client to connect on port %d\n", server_port);

	while(1)
	{
		sin_size = sizeof(struct sockaddr_in);
		socklen_t *temp = (socklen_t *) &sin_size;

		if(connection_type == 1)
		{
			link = accept(sock, (struct sockaddr *)&client_address, temp);
			printf("Client connected from %s:%d", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
		}
		while(1)
		{
			if(connection_type == 1)
			{
				received_bytes = recv(link, received_data, 2048, 0);

			}
			else if(connection_type == 2)
			{
				received_bytes = recvfrom(sock, received_data, 2048, 0,(struct sockaddr *)&client_address, temp);
				
			}

			received_data[received_bytes] = '\0';

			parse_request();

			if (!received_bytes || !strcmp(received_data, "exit"))
			{
				printf("Connection closed\n");
				close(link);
				break;
			}

			else
			{

			}
		}
	}

}

int main()
{
	
	int server_port;
	cout<<"Enter server port: ";
	cin>>server_port;

	string type;
	cout<<"Connection Type (tcp/udp)? ";
	cin>>type;

	if(type == "tcp")
		connection_type = 1;
	else if(type == "tcp")
		connection_type = 2;
	else
	{
		cout<<"Invalid Connection Type";
		exit(EXIT_FAILURE);
	}

	startServer(server_port);

	return 0;
}