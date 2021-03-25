/*
 *	Example from: https://www.binarytides.com/server-client-example-c-sockets-linux/
	C socket server example
*/

#include<stdio.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write

int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[2000];
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1){
		printf("Could not create socket\n");
	}
	puts("Socket created\n");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );
	
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
		perror("bind failed. Error\n");
		return 1;
	}
	puts("bind done\n");
	
	//Listen
	listen(socket_desc , 3);

	while(1){	
		//Accept and incoming connection
		puts("Waiting for incoming connections...\n");
		c = sizeof(struct sockaddr_in);
	
		//accept connection from an incoming client
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		if (client_sock < 0){
			perror("Accept failed\n");
			return 1;
		}
		puts("Connection accepted!\n");
		
		//Receive a message from client
		while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 ){
			puts("Received GPS data:");
			puts("Raw:");
			for(int i=0; i<=15; i++){
				printf("%02x\n", (unsigned char) client_message[i]);
			}
			puts("Converted:");
			double lon = *(double*)(&client_message[0]);
			double lat = *(double*)(&client_message[8]);

			printf("Longitude: %f \nLatitude: %f \n", lon, lat);
			// sizeof(f) = 4
			write(client_sock, client_message, sizeof(client_message));
		}
		
		if(read_size == 0){
			puts("Client disconnected\n");
			fflush(stdout);
		}
		else if(read_size == -1){
			perror("recv failed\n");
		}
	}	
	return 0;
}