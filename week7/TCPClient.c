#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFF_SIZE 1024

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

void show(){
	printf("--------------------------------------------------\n");
	printf("Login syntax\n");
	printf("- Username syntax: 1 username\n");
	printf("- Password syntax: 2 password\n");
	printf("Logout syntax:     3 username\n");
	printf("--------------------------------------------------\n");
}

int main(int argc, char* argv[]){

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

  int SERVER_PORT = atoi(argv[2]);

	int client_sock;
	char buff[BUFF_SIZE + 1];
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;
	
	//Step 1: Construct socket
	client_sock = socket(AF_INET,SOCK_STREAM,0);
	
	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	//Step 3: Request to connect server
	if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
		
	//Step 4: Communicate with server			
	
	//send message
	FILE *f;
	f = fopen("account.txt", "r");
	if(f==NULL){// check file
    printf("Cannot open file!!!\n");
    return 0;
	}
	fclose(f);
	while(1){
		show();
		printf("--> ");
		memset(buff,'\0',(strlen(buff)+1));
		fgets(buff, BUFF_SIZE, stdin);
		if(strcmp(buff,"\n") == 0) break;	
		msg_len = strlen(buff);
			
		bytes_sent = send(client_sock, buff, msg_len, 0);
		if(bytes_sent < 0)
			perror("\nError: ");
		
		//receive echo reply
		bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
		if (bytes_received < 0)
			perror("\nError: ");
		else if (bytes_received == 0)
			printf("Connection closed.\n");
			
		buff[bytes_received] = '\0';
		printf("Reply: %s\n", buff);
		
		// if(strcmp(buff, "Logout successful") == 0) break;
	}
	
	//Step 4: Close socket
	close(client_sock);
	return 0;
}
