#include <stdio.h>          /* These are the usual header files */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFF_SIZE 10240

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

/*
Đếm số ký tự có trong file text
Output: số lượng ký tự có trong file text đó
*/
int count_characters_in_file(FILE *f){
	int count = 0;
	char c;
	while((c = getc(f)) != EOF) count++;
	if(count<=0)
		count = 0;
	else
		count--;
	return count;
}

/*
Kiểm tra tên file có hợp lệ không
Output: 1 - hợp lệ, 0 - không hợp lệ 
*/
int check_file_name(char *string){
	for(int i = 0;i < strlen(string); i++){
		if(string[i] == ' ') return 0;
		if(strcmp(string, "\n") == 0) return 0;
	}
	return 1;
}

void menu_func(){
	printf("MENU\n");
	printf("---------------------\n");
	printf("1. Gửi xâu bất kỳ\n");
	printf("2. Gửi nội dung 1 file\n");
	printf("Chọn: \n");
}

int main( int argc, char *argv[]){

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
  char file_name[BUFF_SIZE];

	int client_sock; /* file descriptors */
	char buff[BUFF_SIZE];
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
	
	int menu = -1;
	do{
		menu_func();
		menu = -1;
		scanf("%d", &menu);
		switch(menu){
			case 1:
				//send message
				while(1){
					printf("Insert string:");
					memset(buff,'\0',(strlen(buff)+1));
					__fpurge(stdin); // xoa bo nho dem
					fgets(buff, BUFF_SIZE, stdin);
					if(strcmp(buff,"\n") == 0) break;
					msg_len = strlen(buff);
					if (msg_len == 0) break;
					
					bytes_sent = send(client_sock, buff, msg_len, 0); // send to server
					if(bytes_sent <= 0){
						printf("\nConnection closed!\n");
						break;
					}
					
					//receive echo reply
					bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
					if(bytes_received <= 0){
						printf("\nError!Cannot receive data from sever!\n");
						break;
					}
					
					buff[bytes_received] = '\0';
					//printf("Reply from server:\n%s\n", buff);
					printf("%s\n", buff);
				}
				break;
			case 2:
				//send message
				printf("Insert file name(E.g. test.txt): ");
				__fpurge(stdin);
				fgets(file_name, BUFF_SIZE, stdin);
				if(check_file_name(file_name) == 0){ // check file name
					printf("Error file name\n");
					break;
				}

				file_name[strlen(file_name)-1] = '\0';
				
				FILE *f;
				f = fopen(file_name, "r");
				if(f==NULL){
					printf("Cannot open file!\n");
					break;
				}

				int count = count_characters_in_file(f); // count characters in the file

				char *buff_file = calloc(count,sizeof(char));
				fseek(f, 0, SEEK_SET); // di chuyen con tro file ve dau file
				fread(buff_file, count, 1, f); // read file
				fclose(f);

				buff_file[count] = '\0';

				if( count <= 0)
					printf("File empty\n");	
				else{
					// printf("%s\n", buff_file);
					strcat(buff_file,"!@#%&nguyenvd27!@#%&"); // gan 1 xau dac biet de nhan biet noi dung file
					bytes_sent = send(client_sock, buff_file, strlen(buff_file), 0); // send to server
					if(bytes_sent <= 0){
						printf("\nConnection closed!\n");
						break;
					}
				}

				free(buff_file);
				break;
			default:
				break;
		}
	}while(menu == 1 || menu == 2);

	//Step 4: Close socket
	close(client_sock);
	return 0;
}
