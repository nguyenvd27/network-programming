/*UDP Echo Client*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFF_SIZE 2048

/* 
Kiểm tra dấu chấm trong địa chỉ ip
Output: 1 - dấu chấm hợp lệ, 0 - dấu chấm lỗi
*/
int check_period(char *string){
  	int count_period = 0, n = strlen(string);

  	if(string[0] == '.') return 0;
  	if(string[n-1] == '.') return 0;
  	for (int i = 0; i < n-1; i++){
    	if (string[i] == '.')
      		count_period++;
    	if (string[i] == '.' && string[i + 1] == '.') //Kiểm tra 2 dấu chấm có cạnh nhau không
      		return 0;
  	}
  	if (count_period != 3) //Số lượng dấu chấm khác 3 sẽ fail
    	return 0;
  	return 1;
}

/* 
Kiểm tra xem string có là địa chỉ IP không
Output: 1 - là địa chỉ IP, 0 - không là địa chỉ IP
*/
int check_IP(char *string){
  	int value = 0, n = strlen(string);
  	if(check_period(string) == 0){
    	return 0;
  	}else{
    	for(int i=0; i<n; i++){
      		if( string[i] == '.'){
        		if(value < 0 || value > 255)
          			return 0;
        		value = 0;
      		}else{
        		if(string[i] >= '0' && string[i] <= '9'){
          			value = value*10 + (string[i] - '0');
          			if(i == n-1)
            			if(value < 0 || value > 255)
              				return 0;
        		}else
          		return 0;
      		}
    	}	
    return 1;
  }
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

	if(argc != 3){
		printf("Syntax Error.\n");
		printf("Syntax: ./client IPAddress PortNumber\n");
		return 0;
	}
	if(check_IP(argv[1]) == 0){
	    printf("IP address invalid\n");
	    return 0;
	}
	  if(check_port(argv[2]) == 0){
	    printf("Port invalid\n");
	    return 0;
	}
	int SERV_PORT = atoi(argv[2]);

	int client_sock; /* file descriptors */
	char buff[BUFF_SIZE]; // luu tru string gui cho server
	struct sockaddr_in server_addr; /* server's address information */
	int bytes_sent,bytes_received; // client's address information
	unsigned int sin_size;
	
	//Step 1: Construct a UDP socket
	if ((client_sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){  /* calls socket() */
		perror("\nError.");
		return 0;
	}

	//Step 2: Define the address of the server
	bzero(&server_addr, sizeof(server_addr)); //The bzero() function copies n bytes, each with a value of zero, into string s.
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERV_PORT);
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	//Step 3: Communicate with server
	while(1){
		printf("Send to server:\n");
		memset(buff,'\0',(strlen(buff)+1));//dat n byte trong vung nho cua buff thanh '\0'
		fgets(buff, BUFF_SIZE, stdin);// stdin la file mac dinh
		if(strcmp(buff,"\n") == 0) break;
		if(strcmp(buff,"\0") == 0) break;
		
		sin_size = sizeof(struct sockaddr);
		
		bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
		if(bytes_sent < 0){
			perror("Error.");
			close(client_sock);
			return 0;
		}

		bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &server_addr, &sin_size);
		if(bytes_received < 0){
			perror("Error: ");
			close(client_sock);
			return 0;
		}
		buff[bytes_received] = '\0';
		printf("Reply from server:\n%s\n", buff);
	}

	close(client_sock);
	return 0;
}