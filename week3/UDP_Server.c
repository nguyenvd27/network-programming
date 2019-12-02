/*UDP Echo Server*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define BUFF_SIZE 2048
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

	int server_sock; /* file descriptors */
	char buff[BUFF_SIZE]; // luu tru string gui tu client
	int bytes_sent, bytes_received;// so byte gui va nhan
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; // client's address information 
	unsigned int sin_size;

	//Step 1: Construct a UDP socket
	if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){  /* calls socket() */
		perror("\nError");
		return 0;
	}
	
	//Step 2: Bind address to socket
	server.sin_family = AF_INET;         
	server.sin_port = htons(PORT); // host to network
	server.sin_addr.s_addr = INADDR_ANY;  /* INADDR_ANY puts your IP address automatically */   
	bzero(&(server.sin_zero),8); // thuoc tinh ko su dung, set ve zero
  
	if(bind(server_sock,(struct sockaddr*)&server,sizeof(struct sockaddr))==-1){ /* calls bind() */
		perror("\nError");
		return 0;
	}     
	
	//Step 3: Communicate with clients
	while(1){
		sin_size=sizeof(struct sockaddr_in);
		
		bytes_received = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client, &sin_size);// nhan du lieu tu client
		if(bytes_received < 0){
			perror("\nError.");
		}
		else{
			buff[bytes_received] = '\0';
			printf("[%s:%d]: %s", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff);
		}

		int check_error = parse_string( buff);
		if(check_error == 0){
			strcpy(buff, "Error");
      		bytes_sent = sendto(server_sock, buff, 5, 0, (struct sockaddr *) &client, sin_size); // gui error cho client
		}
    	else{
  			bytes_sent = sendto(server_sock, buff, bytes_received, 0, (struct sockaddr *)&client, sin_size); // gui du lieu da xu ly cho clien
    	}
    	
		if(bytes_sent < 0)
			perror("\nError: ");					
	}
	
	close(server_sock);
	return 0;
}