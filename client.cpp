#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <vector>
#include <ctype.h>
#include <fstream>
#include <stddef.h>
#include <dirent.h>
#include <signal.h>
#include <iomanip>
#include <openssl/md5.h>
#include <sstream> 

using namespace std;

struct filestructure
{
	char name[2048], type[2048], timestamp[200], checksum[1000];
	int size;
};

struct hostent *host;

int connection_type, send_command_count = 0;

struct filestructure server_file_structure[2048];

char input_command[2048], send_command[50][100], received_data[2048];

void get_input()
{
	char c;
	int count = 0;

	scanf("%c", &c);

	while(c!='\n')
	{
		input_command[count++] = c;
		scanf("%c",&c);
	}

	input_command[count++] = '\0';
	count = 0;
	send_command_count = 0;

	char * pch;
	pch = strtok (input_command," ,.-");
	while (pch != NULL)
	{
		strcpy(send_command[send_command_count], pch);
		// printf ("%s\n", send_command[send_command_count]);
		send_command_count++;
		pch = strtok (NULL, " ,.-");
	}

}

int startClient(int server_port)
{
	int sock, sin_size, link, server_file_count, received_bytes;

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

	get_input();

	while(strcmp(send_command[0], "exit"))
	{
		if(!strcmp(send_command[0], "FileHash"))
		{
			if(connection_type == 1)
			{
				send(sock, input_command, sizeof(input_command), 0);
				recv(sock, &server_file_count, sizeof(server_file_count), 0);
				cout<<server_file_count<<endl;
			}
			else
			{
				sendto(sock, input_command, sizeof(input_command), 0,(struct sockaddr *)&server_address, sizeof(struct sockaddr));
				recvfrom(sock, &server_file_count, sizeof(server_file_count), 0, (struct sockaddr *)&server_address, temp);
			}

			int length = server_file_count;
			for(int i = 0; i < length; i++)
			{
				if(connection_type == 1)
				{
					received_bytes = recv(sock, received_data, 2048, 0);
					received_data[received_bytes] = '\0';
					strcpy(server_file_structure[i].name, received_data);
					cout<<server_file_structure[i].name<<endl;

					recv(sock, &server_file_structure[i].size, sizeof(int), 0);
					cout<<server_file_structure[i].size<<endl;

					received_bytes = recv(sock, received_data, 2048, 0);
					received_data[received_bytes] = '\0';
					strcpy(server_file_structure[i].type, received_data);
					cout<<server_file_structure[i].type<<endl;

					received_bytes = recv(sock, received_data, 2048, 0);
					received_data[received_bytes] = '\0';
					strcpy(server_file_structure[i].timestamp, received_data);
					cout<<server_file_structure[i].timestamp<<endl;


					recv(sock, server_file_structure[i].checksum, 33, 0);

					printf("Checksum: %s\n", server_file_structure[i].checksum);
					


				}
				else
				{
					received_bytes = recvfrom(sock, received_data, 2048, 0, (struct sockaddr *)&server_address, temp);
					received_data[received_bytes] = '\0';
					strcpy(server_file_structure[i].name, received_data);
					recvfrom(sock, &server_file_structure[i].size, sizeof(int), 0, (struct sockaddr *)&server_address, temp);
					received_bytes = recvfrom(sock, received_data, 2048, 0, (struct sockaddr *)&server_address, temp);
					received_data[received_bytes] = '\0';
					strcpy(server_file_structure[i].type, received_data);
					received_bytes = recvfrom(sock, received_data, 2048, 0, (struct sockaddr *)&server_address, temp);
					received_data[received_bytes] = '\0';
					strcpy(server_file_structure[i].timestamp, received_data);
					recvfrom(sock, &server_file_structure[i].checksum, 33, 0, (struct sockaddr *)&server_address, temp);

				}

			}

			if(!strcmp(send_command[1], "verify"))
			{
				if(!strcmp(send_command[2], ""))
				{
					printf("Error: Missing filename argument\n");
				}
				else
				{
					int i;
					for(i = 0; i < length; i++)
					{
						if(!strcmp(send_command[2], server_file_structure[i].name))
						{
							printf("File: %s\n", server_file_structure[i].name);
							printf("Checksum: %s\n", server_file_structure[i].checksum);
							printf("Last Modified: %s", server_file_structure[i].timestamp);
							break;
						}
					}

					if(i == length)
					{
						printf("Error: No such file exists in server directory\n");
					}
				}
			}
		}



		get_input();
	}

	close(sock);

}


int main()
{
	/*
	char *str = new char[10];
	string temp;
	getline(cin,temp);
	strcpy (str, temp.c_str());
	int local_port, server_port;
	get_input();
	*/

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