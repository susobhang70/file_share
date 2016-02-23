#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <arpa/inet.h>

using namespace std;

struct hostent *host;

int connection_type;

int startClient(int server_port)
{
	int sock, sin_size, link;

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
	server_address.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(server_address.sin_zero), 8);

	sin_size = sizeof(struct sockaddr_in);
	socklen_t *temp = (socklen_t *) &sin_size;

	if(connection_type == 1)
	{
		if (connect(sock, (struct sockaddr *)&server_address, sizeof(struct sockaddr)) == -1)
		{
			printf ("Error: Unable to connect to server on port %d\n", server_port);
			return 2;
		}
		printf("Client connected to port %d\n", server_port);
	}

}


int main()
{
	int server_port;

	char servername[100];

	cout<<"Enter servername / serverIP: ";
	cin>>servername;

	host = gethostbyname(servername);

	if(host == NULL)
	{
		perror("Error: ");
		exit(EXIT_FAILURE);
	}

	cout<<"Enter server port to connect to: ";
	cin>>server_port;

	// cout<<"Enter client port to connect through (input 0 to select default 27666 as port): ";
	// cin>>local_port;

	// if(local_port == 0)
	// 	local_port = 27666;

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

	while(1)
	{
		startClient(server_port);
		sleep(1);
	}

	return 0;
}