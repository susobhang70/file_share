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
#include <regex.h>

using namespace std;

struct filestructure
{
	char name[2048], type[2048], timestamp[200], checksum[1000];
	int size;
	time_t rawtimestamp;
};

struct hostent *host;

time_t time1, time2;
struct tm *temptime1, *temptime2;

int connection_type, send_command_count = 0;

struct filestructure server_file_structure[2048];

char input_command[2048], send_command[50][100], received_data[2048], send_data[2048];

regex_t regex;

int check_directory(char *type)
{
	char temp[2048];
	strcpy(temp, type);
	char * pch;
	pch = strtok (temp," ");
	while (pch != NULL)
	{
		if(!strcmp(pch, "directory"))
			return 0;
		pch = strtok (NULL, " ");
	}
	return 1;
}

int MD5(char *filename, char *checksum)
{
	ifstream::pos_type filesize;
	char *memoryblock;

	ifstream file(filename, ios::ate);

	if(!file.is_open())
	{
		cout<<"Unable to open\t"<<filename<<endl;
		return 0;
	}

	filesize = file.tellg();
	memoryblock = new char[filesize];
	file.seekg(0, ios::beg);
	file.read(memoryblock, filesize);
	file.close();

	unsigned char csum[MD5_DIGEST_LENGTH];
	MD5((unsigned char *) memoryblock, filesize, csum);

	ostringstream os;
    for (int i=0; i<MD5_DIGEST_LENGTH; i++) 
    {
        os<< hex << setw(2) << setfill('0') << (int) csum[i];
    }

    strcpy(checksum, (os.str()).c_str());
	return (strlen(checksum) == 32);
}

void get_input()
{
	char c;
	int count = 0;

	cout<<"$> ";

	scanf("%c", &c);

	while(c!='\n')
	{
		input_command[count++] = c;
		scanf("%c",&c);
	}

	input_command[count++] = '\0';
	count = 0;
	send_command_count = 0;

	char temp[2048];
	strcpy(temp, input_command);
	char * pch;
	pch = strtok (temp," ");
	while (pch != NULL)
	{
		strcpy(send_command[send_command_count], pch);
		// printf ("%s\n", send_command[send_command_count]);
		send_command_count++;
		pch = strtok (NULL, " ");
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
					recv(sock, server_file_structure[i].name, 2048, 0);

					recv(sock, &server_file_structure[i].size, sizeof(int), 0);

					recv(sock, server_file_structure[i].type, 2048, 0);
					
					recv(sock, server_file_structure[i].timestamp, 200, 0);

					recv(sock, server_file_structure[i].checksum, 33, 0);

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

					received_bytes = recvfrom(sock, received_data, 200, 0, (struct sockaddr *)&server_address, temp);
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
							printf("Last Modified: %s\n\n", server_file_structure[i].timestamp);
							break;
						}
					}

					if(i == length)
					{
						printf("Error: No such file exists in server directory\n");
					}
				}
			}

			else if(!strcmp(send_command[1], "checkall"))
			{
				int i;
				for(i = 0; i < length; i++)
				{
					printf("File: %s\n", server_file_structure[i].name);
					printf("Checksum: %s\n", server_file_structure[i].checksum);
					printf("Last Modified: %s\n\n", server_file_structure[i].timestamp);
				}
			}
			else
			{
				printf("Error: Invalid arguments\n");
			}
		}

		else if(!strcmp(send_command[0], "IndexGet"))
		{
			if(connection_type == 1)
			{
				send(sock, input_command, sizeof(input_command), 0);
				recv(sock, &server_file_count, sizeof(server_file_count), 0);
				// cout<<server_file_count<<endl;
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
					recv(sock, server_file_structure[i].name, 2048, 0);

					recv(sock, &server_file_structure[i].size, sizeof(int), 0);

					recv(sock, server_file_structure[i].type, 2048, 0);
					
					recv(sock, server_file_structure[i].timestamp, 200, 0);

					recv(sock, &server_file_structure[i].rawtimestamp, sizeof(time_t), 0);
					

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

					received_bytes = recvfrom(sock, received_data, 200, 0, (struct sockaddr *)&server_address, temp);
					received_data[received_bytes] = '\0';
					strcpy(server_file_structure[i].timestamp, received_data);

					recvfrom(sock, &server_file_structure[i].rawtimestamp, sizeof(time_t), 0, (struct sockaddr *)&server_address, temp);

				}

			}

			if(!strcmp(send_command[1], "longlist"))
			{
				int i;
				for(i = 0; i < length; i++)
				{
					printf("File: %s\n", server_file_structure[i].name);
					printf("Type: %s\n", server_file_structure[i].type);
					printf("Last Modified: %s\n", server_file_structure[i].timestamp);
					// printf("RawTimeStamp:%d\n\n", server_file_structure[i].rawtimestamp);
				}
			}

			else if(!strcmp(send_command[1], "regex"))
			{
				int reti, i;
				char msgbuf[100];
				reti = regcomp(&regex, send_command[2], 0);

				if (reti) 
				{
				    fprintf(stderr, "Could not compile regex\n");
				    exit(1);
				}

				for(i = 0; i < length; i++)
				{
					reti = regexec(&regex, server_file_structure[i].name, 0, NULL, 0);
					if (!reti) 
					{
					    printf("File: %s\n", server_file_structure[i].name);
						printf("Type: %s\n", server_file_structure[i].type);
						printf("Last Modified: %s\n", server_file_structure[i].timestamp);
					}
					else if (reti == REG_NOMATCH) 
					{
					    continue;
					}
					else 
					{
					    regerror(reti, &regex, msgbuf, sizeof(msgbuf));
					    fprintf(stderr, "Regex match failed: %s\n", msgbuf);
					    exit(1);
					}
				}

				/* Free memory allocated to the pattern buffer by regcomp() */
				regfree(&regex);
			}

			else if(!strcmp(send_command[1], "shortlist"))
			{
				if(!strcmp(send_command[2], "") || !strcmp(send_command[3], ""))
				{
					printf("Error: Missing filename argument\n");
				}
				
				else
				{
					// get_dates();

				}	

			}
			else
			{
				printf("Error: Invalid arguments. Usage: IndexGet <flag> <parameter>\n");
			}

		}

		else if(!strcmp(send_command[0], "FileDownload"))
		{
			if(send_command_count < 3)
			{
				printf("Error : missing arguments for download\n");
				get_input();
				continue;
			}
			else
			{
				int c_type, c_link, new_sock;

				struct sockaddr_in s_address;


				if(!strcmp(send_command[1], "tcp"))
				{
					if(connection_type == 1)
					{
						c_type = 1;
						new_sock = sock;
					}

					send(new_sock, input_command, sizeof(input_command), 0);
					received_bytes = recv(new_sock, received_data, 2048, 0);

				}
				else if(!strcmp(send_command[1], "udp"))
				{
					if(connection_type == 2)
					{
						c_type = 2;
						new_sock = sock;
						s_address = server_address;
					}
					sendto(new_sock, input_command, sizeof(input_command), 0, (struct sockaddr *)&s_address, sizeof(struct sockaddr));
					received_bytes = recvfrom(new_sock, received_data, 2048, 0, (struct sockaddr *)&s_address, temp);

				}
				else
				{
					printf("Error: Invalid Input\n");
					get_input();
					continue;
				}

				received_data[received_bytes] = '\0';
				if(strcmp(received_data, "File not found") != 0)
				{
					if(c_type == 1)
					{
						recv(new_sock, received_data, 33, 0);
					}
					else
					{
						received_bytes = recvfrom(new_sock, received_data, 33, 0, (struct sockaddr *)&s_address, temp);
						received_data[received_bytes] = '\0';

					}

					if(strcmp(received_data, "File is directory, can't calc md5") != 0)
					{
						char incoming_md5[33];
						strcpy(incoming_md5, received_data);

						int incoming_size;

						if(c_type == 1)
						{
							recv (new_sock, &incoming_size, sizeof(int), 0);
							received_bytes = recv(new_sock, received_data, 2048, 0);

						}
						else
						{
							recvfrom(new_sock, &incoming_size, sizeof(int), 0,(struct sockaddr *)&s_address, temp);
							received_bytes = recvfrom (new_sock, received_data, 2048, 0,(struct sockaddr *)&s_address, temp);

						}

						FILE *fp;
						fp = fopen(send_command[2], "w");

						while(strcmp(received_data, "eof"))
						{
							for (int i = 0; i < incoming_size; i++)
							{
								fprintf(fp,"%c",received_data[i]);
							}

							if(c_type == 1)
							{
								recv(sock, &incoming_size, sizeof(int), 0);
								received_bytes = recv(sock, received_data, 2048, 0);

							}
							else
							{
								recvfrom(sock, &incoming_size, sizeof(int), 0, (struct sockaddr *)&s_address, temp);
								received_bytes = recvfrom(sock, received_data, 2048, 0, (struct sockaddr *)&s_address, temp);
							}
							received_data[received_bytes] = '\0';
						}

						fclose(fp);

						char new_checksum[33];

						MD5(send_command[2], new_checksum);

						if(!strcmp(incoming_md5, new_checksum))
						{
							printf("MD5 Checksum matched! File download complete.\n");
						}
						else
						{
							printf("Checksum didn't match. Download failed.\n");
						}
					}
				}
				else
				{
					printf("File not found\n");
					get_input();
					continue;
				}
			}
		}

		else if(!strcmp(send_command[0], "FileUpload"))
		{
			ifstream ifile(send_command[1]);
			if (ifile)
			{
				if (connection_type == 1)
				{
					send(sock, input_command, 2048, 0);
				}
				else
				{
					sendto(sock, input_command, 2048, 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr));
				}

				char file_details_command[100];

				strcpy(file_details_command, "file ");
				strcat(file_details_command, send_command[2]);
				strcat(file_details_command, "> file_details_2");
				system(file_details_command);

				ifstream file_details;
				string input;
				file_details.open("file_details_2");
				getline(file_details, input);
				file_details.close();

				strcpy(file_details_command, "rm file_details_2");
				system(file_details_command);

				char tempstr[2048], sending_checksum[33];
				strcpy(tempstr, input.c_str());

				if (!check_directory(send_command[1]))
				{
					printf("Error: Cannot upload directory\n");
					get_input();
					continue;
				}
				else
				{
					MD5(send_command[1], sending_checksum);
					printf ("Check sum for file to be uploaded: %s\n", sending_checksum);
				}

				int count;
				char c;
				FILE *fp = fopen(send_command[1], "r");

				if (connection_type == 1)
				{
					send(sock, sending_checksum, 33, 0);
				}
				else
				{
					sendto(sock, sending_checksum, 33, 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr));
				}

				while(fscanf(fp,"%c",&c) != EOF)
				{
					count = 0;
					send_data[count] = c;
					count++;

					while (count < 2048 && fscanf(fp, "%c", &c) != EOF)
					{
						send_data[count] = c;
						count++;
					}

					if (connection_type == 1)
					{
						send(sock, &count, sizeof(int), 0);
						send (sock, send_data, 2048, 0);

					}
					else
					{
						sendto(sock, &count, sizeof(int), 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr));
						sendto (sock, send_data, 2048, 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr));

					}
				}

				int garbage = 0;

				if (connection_type == 1)
				{
					send(sock, &garbage, sizeof(int), 0);
					send(sock, "eof", 2048, 0);

				}
				else
				{
					sendto(sock, &garbage, sizeof(int), 0,(struct sockaddr *)&server_address, sizeof(struct sockaddr));
					sendto(sock, "eof", 2048, 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr));

				}
			}
			else
			{
				printf("\n Error : No such file or directory.\n");
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
	else if(type == "udp")
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