#include <stdio.h>          /* These are the usual header files */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 2   /* Number of allowed connections */
#define BUFF_SIZE 10240

/*
Tách chuỗi đầu vào thành chuỗi toàn số và toàn chữ
Output: 1 - chuỗi hợp lệ, 0 - chuỗi bị lỗi
*/
int parse_string (char buff[BUFF_SIZE]){
	char string[BUFF_SIZE], number[BUFF_SIZE];
	int n = strlen(buff);
	int j=0,k=0;
	for(int i = 0; i < n; i++){
  	if( buff[i] >= '0' && buff[i] <= '9' ){
  		number[j] = buff[i];
  		j++;
  	}else if((buff[i] >= 'a' && buff[i] <= 'z') || (buff[i] >= 'A' && buff[i] <= 'Z' )){
  		string[k] = buff[i];
  		k++;
  	}else if(buff[i]!='\n'){
    	return 0;
  	}
	}

	//copy tu number va string sang buff
	if(j==0){ //ko co so
  	for(int x = 0; x < k ; x++)
  		buff[x] = string[x];
  	buff[k] = '\0';
	}else if(k==0){ //ko co chu
  	for(int z=0; z<j; z++)
  		buff[z] = number[z];
  	buff[j] = '\0';
	}else{ //co ca so va chu
  	for(int z=0; z<j; z++)
  		buff[z] = number[z];
  	buff[j] = '\n';
  	for(int x = 0; x < k ; x++)
  		buff[j+1+x] = string[x];
  	buff[j+1+k] = '\0';
	}
	return 1;
}

/*
Kiểm tra số hiệu cổng
Output: 1 - cổng hợp lệ, 0 - cổng không hợp lệ
*/
int check_port(char *port){
	int n = strlen(port);
	for(int i=0; i< n; i++){
  	if(port[i]<'0' || port[i]>'9')
  		return 0;
	}
	return 1;
}

int main(int argc, char *argv[]){
 	
 	if(argc != 2){
    printf("Syntax Error.\n");
    printf("Syntax: ./server PortNumber\n");
    return 0;
  }
  if(check_port(argv[1]) == 0){
    printf("Port invalid\n");
    return 0;
  }

  int PORT = atoi(argv[1]);

	int listen_sock, conn_sock; /* file descriptors */
	char recv_data[BUFF_SIZE];
	int bytes_sent, bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	unsigned int sin_size;
	
	//Step 1: Construct a TCP socket to listen connection request
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		return 0;
	}
	
	//Step 2: Bind address to socket
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	server.sin_port = htons(PORT);   /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   
	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){ /* calls bind() */
		perror("\nError: ");
		return 0;
	}     
	
	//Step 3: Listen request from client
	if(listen(listen_sock, BACKLOG) == -1){  /* calls listen() */
		perror("\nError: ");
		return 0;
	}
	
	//Step 4: Communicate with client
	while(1){
		//accept request
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock,( struct sockaddr *)&client, &sin_size)) == -1) 
			perror("\nError: ");
  
		printf("You got a connection from [%s:%d]\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port) ); /* prints client's IP */
		
		//start conversation
		while(1){
			bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0); //blocking
			if(bytes_received <= 0){
				printf("\nConnection closed");
				break;
			}else{
				if(strstr(recv_data,"!@#%&nguyenvd27!@#%&") == NULL){//xac dinh file hay string
					recv_data[bytes_received - 1] = '\0';
					printf("Receive string: %s\n", recv_data);

					//echo to client
					int chek_error = parse_string(recv_data);//xu ly string
					if( chek_error == 0){
						strcpy( recv_data, "Error");
						bytes_sent = send(conn_sock, recv_data, strlen(recv_data), 0); /* send error to the client */
					}else{
						bytes_sent = send(conn_sock, recv_data, bytes_received, 0); /* send to the client welcome message */
					}
					if (bytes_sent <= 0){
						printf("\nConnection closed");
						break;
					}
				}else{ // hien thi noi dung file
					recv_data[bytes_received - strlen("!@#%&nguyenvd27!@#%&")] = '\0';
					printf("Receive file content:\n%s\n", recv_data);
				}
			}
		}//end conversation
		close(conn_sock);	
	}
	
	close(listen_sock);
	return 0;
}