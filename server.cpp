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
#include <ctime>

#define MD5_LENGTH 32

using namespace std;

struct filestructure
{
	char name[2048], type[2048], timestamp[200], checksum[MD5_LENGTH + 1];
	int size;
	time_t rawtimestamp;
};

int connection_type, received_command_length, server_file_count = 0;

char received_data[2048], initial_received_command[2048], received_command[50][100];
char send_data[2048];

struct filestructure server_file_structure[2048];

// function to calculate MD5 checksum of a file
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

void parse_request()
{
	strcpy(initial_received_command, received_data);
	
	received_command_length = 0;
	char *temp;
	temp = strtok(initial_received_command," ");
	
	while(temp != NULL){
		strcpy(received_command[received_command_length], temp);
		received_command_length ++ ;
		temp = strtok(NULL," ");
	}
	
	strcpy(initial_received_command, received_data);
	return;
}

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

// have to change this shit
void sync_files()
{
	int i;
	DIR *dir;
	struct dirent *ep;

	dir = opendir("./");
	if (dir)
	{
		for(i = 0; (ep = readdir(dir)); i++) 
		{
			strcpy(server_file_structure[i].name, ep->d_name);

			struct stat details;
			stat(ep->d_name, &details);

			server_file_structure[i].size = details.st_size;

			char file_details_command[100];

			strcpy(file_details_command, "file ");
			strcat(file_details_command, server_file_structure[i].name);
			strcat(file_details_command, "> file_details");
			system(file_details_command);

			ifstream file_details;
			string input;
			file_details.open("file_details");
			getline(file_details, input);
			file_details.close();

			strcpy(server_file_structure[i].type, input.c_str());

			strcpy(file_details_command, "rm file_details");
			system(file_details_command);

			strcpy(server_file_structure[i].timestamp, ctime(&details.st_mtime));

			server_file_structure[i].rawtimestamp = details.st_mtime;
			//MD5

			// cout<<server_file_structure[i].name<<endl;

			if(check_directory(server_file_structure[i].type))
				MD5(server_file_structure[i].name, server_file_structure[i].checksum);
			else
				strcpy(server_file_structure[i].checksum, "                                \0");
		}
		server_file_count = i-1;
		closedir(dir);
	}
	else
	{
		printf("\n Error : could not open directory.\n");
	}
}

int startServer(int server_port)
{
	int sock, sin_size, connection_link, received_bytes;

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
		// printf("1\n");

		if(connection_type == 1)
		{
			connection_link = accept(sock, (struct sockaddr *)&client_address, temp);
			printf("Client connected from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
		}
		while(1)
		{
			// printf("2\n");
			if(connection_type == 1)
			{
				received_bytes = recv(connection_link, received_data, 2048, 0);

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
				close(connection_link);
				break;
			}

			else
			{
				printf("Client Request : %s\n", initial_received_command);

				if(!strcmp(received_command[0], "FileHash"))
				{
					sync_files();

					if(connection_type == 1)
					{
						send(connection_link, &server_file_count, sizeof(int), 0);
					}
					else
					{
						sendto(sock, &server_file_count, sizeof(int), 0,(struct sockaddr *)&client_address, sizeof(struct sockaddr));
					}

					int i;
					for(i = 0; i < server_file_count; i++)
					{
						// cout<<i<<endl;
						if(connection_type == 1)
						{
							send(connection_link, server_file_structure[i].name, 2048, 0);
							send(connection_link, &server_file_structure[i].size, sizeof(int), 0);
							send(connection_link, server_file_structure[i].type, 2048, 0);
							// printf("%s %d\n", server_file_structure[i].type, strlen(server_file_structure[i].type));
							send(connection_link, server_file_structure[i].timestamp, 200, 0);
							send(connection_link, server_file_structure[i].checksum, 33, 0);
							
						}
						else
						{
							sendto(sock, server_file_structure[i].name, 2048, 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));
							sendto(sock, &server_file_structure[i].size, sizeof(int), 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));
							sendto(sock, server_file_structure[i].type, 2048, 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));
							sendto(sock, server_file_structure[i].timestamp, 200, 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));
							sendto(sock, server_file_structure[i].checksum, 33, 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));
							
						}
					}
				}


				else if(!strcmp(received_command[0], "IndexGet"))
				{
					sync_files();

					if(connection_type == 1)
					{
						send(connection_link, &server_file_count, sizeof(int), 0);
					}
					else
					{
						sendto(sock, &server_file_count, sizeof(int), 0,(struct sockaddr *)&client_address, sizeof(struct sockaddr));
					}

					int i;
					for(i = 0; i < server_file_count; i++)
					{
						// cout<<i<<endl;
						if(connection_type == 1)
						{
							int temp = strlen(server_file_structure[i].name);
							send(connection_link, &(temp), sizeof(int), 0);
							send(connection_link, server_file_structure[i].name, temp, 0);

							send(connection_link, &server_file_structure[i].size, sizeof(int), 0);

							temp = strlen(server_file_structure[i].type);
							send(connection_link, &(temp), sizeof(int), 0);
							send(connection_link, server_file_structure[i].type, temp, 0);

							temp = strlen(server_file_structure[i].timestamp);
							send(connection_link, &(temp), sizeof(int), 0);
							send(connection_link, server_file_structure[i].timestamp, temp, 0);

							send(connection_link, &server_file_structure[i].rawtimestamp, sizeof(time_t), 0);
						}
						else
						{
							int temp = strlen(server_file_structure[i].name);
							sendto(sock, &temp, sizeof(int), 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));
							sendto(sock, server_file_structure[i].name, temp, 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));

							sendto(sock, &server_file_structure[i].size, sizeof(int), 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));

							temp = strlen(server_file_structure[i].type);
							sendto(sock, &temp, sizeof(int), 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));
							sendto(sock, server_file_structure[i].type, temp, 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));

							temp = strlen(server_file_structure[i].timestamp);
							sendto(sock, &temp, sizeof(int), 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));
							sendto(sock, server_file_structure[i].timestamp, temp, 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));

							temp = sizeof(time_t);
							sendto(sock, &temp, sizeof(int), 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));
							sendto(sock, &server_file_structure[i].rawtimestamp, temp, 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));
							
						}
					}
				}

				else if(!strcmp(received_command[0], "FileDownload"))
				{
					ifstream ifile(received_command[2]);

					int c_type, c_link, new_sock;

					struct sockaddr_in cl_address;

					if(ifile)
					{

						if(!strcmp(received_command[1], "tcp"))
						{
							if(connection_type == 1)
							{
								send(connection_link, "Success", 2048, 0);
								c_type = 1;
								c_link = connection_link;
								
							}
						}

						else if(!strcmp(received_command[1], "udp"))
						{
							if(connection_type == 2)
							{
								sendto(sock, "Success", 2048, 0,(struct sockaddr *)&client_address, sizeof(struct sockaddr));
								c_type = 2;
								cl_address = client_address;
								new_sock = sock;
							}
						}
						else
						{
							printf("Error: Invalid argument. Usage: FileDownload <tcp/udp> filename");
							if(connection_type == 1)
							{
								send(connection_link, "File not found", 2048, 0);
							}

							else
							{
								sendto(sock, "File not found", 2048, 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));
							}

							continue;
						}

						char file_details_command[100];

						strcpy(file_details_command, "file ");
						strcat(file_details_command, received_command[2]);
						strcat(file_details_command, "> file_details");
						system(file_details_command);

						ifstream file_details;
						string input;
						file_details.open("file_details");
						getline(file_details, input);
						file_details.close();

						strcpy(file_details_command, "rm file_details");
						system(file_details_command);

						char tempstr[2048];
						strcpy(tempstr, input.c_str());

						if(!check_directory(tempstr))
						{
							printf("Error calculating checksum.\n");
							if (c_type == 1)
							{
								send (c_link, "File is directory, can't calc md5", 33, 0);
							}
							else
							{
								sendto(new_sock, tempstr, 33, 0,(struct sockaddr *)&cl_address, sizeof(struct sockaddr));
							}
							continue;
						}

						else
						{
							MD5(received_command[2], tempstr);
							printf("File checksum: %s\n", tempstr);
						}

						char c;
						int count;
						FILE *fp;
						fp = fopen(received_command[2], "r");

						if (c_type == 1)
						{
							send (c_link, tempstr, 33, 0);
						}
						else
						{
							sendto(new_sock, tempstr, 33, 0,(struct sockaddr *)&cl_address, sizeof(struct sockaddr));
						}


						while(fscanf(fp, "%c",&c) != EOF)
						{
							count = 0;
							send_data[count++] = c;

							while(count < 2048 && fscanf(fp, "%c",&c) != EOF)
							{
								send_data[count++] = c;
							}

							if (c_type == 1)
							{
								send (c_link, &count, sizeof(int), 0);
								send (c_link, send_data, 2048,0);
							}
							else
							{
								sendto(new_sock, &count, sizeof(int), 0,(struct sockaddr *)&cl_address, sizeof(struct sockaddr));
								sendto(new_sock, send_data, 2048, 0,(struct sockaddr *)&cl_address, sizeof(struct sockaddr));

							}
						}
							
						int garbage = 0;

						if(c_type == 1)
						{
							send(c_link, &garbage, sizeof(int), 0);
							send(c_link, "eof", 2048, 0);

						}

						else
						{
							sendto(new_sock, &garbage, sizeof(int), 0, (struct sockaddr *)&cl_address, sizeof(struct sockaddr));
							sendto(new_sock, "eof", 2048, 0,(struct sockaddr *)&cl_address, sizeof(struct sockaddr));

						}

					}
					else
					{
						printf("Error: Invalid argument. Usage: FileDownload <tcp/udp> filename");
						if(connection_type == 1)
						{
							send(connection_link, "File not found", 2048, 0);
						}

						else
						{
							sendto(sock, "File not found", 2048, 0, (struct sockaddr *)&client_address, sizeof(struct sockaddr));
						}

						continue;
					}

				}

				else if(!strcmp(received_command[0], "FileUpload"))
				{
					char incoming_checksum[33], received_file_checksum[33];
					if(connection_type == 1)
					{
						recv(connection_link, incoming_checksum, 33, 0);
						recv(connection_link, &received_command_length, sizeof(int), 0);
						received_bytes = recv(connection_link, received_data, 2048, 0);
						received_data[received_bytes] = '\0';

					}
					
					else
					{
						recvfrom(sock, incoming_checksum, 33, 0, (struct sockaddr *)&client_address, temp);
						recvfrom(sock, &received_command_length, sizeof(int), 0, (struct sockaddr *)&client_address, temp);
						received_bytes = recvfrom(sock, received_data, 2048, 0, (struct sockaddr *)&client_address, temp);
						received_data[received_bytes] = '\0';
						
					}

					FILE *fp = fopen(received_command[1], "w");
					while(strcmp(received_data, "eof") != 0)
					{
						for (int i = 0; i < received_command_length; i++)
						{
							fprintf(fp, "%c", received_data[i]);
						}

						if (connection_type == 1)
						{
							recv(connection_link, &received_command_length, sizeof(int), 0);
							received_bytes = recv(connection_link, received_data, 2048, 0);
						}
						else
						{
							recvfrom(sock, &received_command_length, sizeof(int), 0,(struct sockaddr *)&client_address, temp);
							received_bytes = recvfrom(sock, received_data, 2048, 0, (struct sockaddr *)&client_address, temp);

						}

						received_data[received_bytes] = '\0';
					}

					fclose(fp);

					printf("MD5 Checksum of uploaded file : %s\n", incoming_checksum);

					MD5(received_command[1], received_file_checksum);

					printf("MD5 checksum of recieved file : %s\n", received_file_checksum);

					if (strcmp(incoming_checksum, received_file_checksum))
					{
						printf("Error : MD5 Checksum didn't match! :(\n");

					}

					else
					{
						printf("MD5 matched! File successfully uploaded\n");
					}

				}
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
	else if(type == "udp")
		connection_type = 2;
	else
	{
		cout<<"Invalid Connection Type";
		exit(EXIT_FAILURE);
	}

	startServer(server_port);

	return 0;
}