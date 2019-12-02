#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>

#include "linked_list.h"

#define BACKLOG 20
#define BUFF_SIZE 1024

/* Handler process signal*/
void sig_chld(int signo){
	pid_t pid;
	int stat;
	
	/* Wait the child process terminate */
	while((pid = waitpid(-1, &stat, WNOHANG))>0)
		printf("\nChild %d terminated\n",pid);
}

/*
* Receive and echo message to client
* [IN] sockfd: socket descriptor that connects to client 	
*/
void echo(int sockfd) {
	char buff[BUFF_SIZE];
	int bytes_sent, bytes_received;
	
	bytes_received = recv(sockfd, buff, BUFF_SIZE, 0); //blocking
	if (bytes_received < 0)
		perror("\nError: ");
	else if (bytes_received == 0)
		printf("Connection closed.");
	
	printf("Client sent: %s\n", buff);
	// printf("HELE\n");
	bytes_sent = send(sockfd, buff, bytes_received, 0); /* echo to the client */
	if (bytes_sent < 0)
		perror("\nError: ");
			
	close(sockfd);
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
Kiểm tra xem trong xâu có khoảng trắng hay ko
Output: 1 - có 1 khoảng trắng, 0 - không có khoảng trắng hoặc nhiều hơn 1 khoảng trắng
*/
int check_space(char *buff){
	int n = strlen(buff), count = 0;
	for(int i=0; i< n; i++){
		if( buff[i] == ' ') count++; 
	}

	if(count == 1) return 1;
	else return 0;
}

/*
Tách xâu để lấy phần username or password phía sau
Output: Nếu đúng định dạng trả về option, ngược lại trả về -1
*/
int split_string( char *buff){
	if(buff[1] == ' '){
		int j = 0;
		int option = buff[0] - 48;
		char tmp[100];
		for(int i = 2; i< strlen(buff); i++){
			tmp[j] = buff[i];
			j++;
		}
		tmp[j] ='\0';
		strcpy(buff, tmp);
		return option;
	}else{
		return -1;
	}
}

/*
Đọc nội dung file lưu vào danh sách liên két
*/
void read_file(FILE *f){
  user_infor user;
  while(!feof(f)){
  	fscanf(f,"%s %s %d", user.username, user.password, &user.status);
  	if(feof(f)) break;
  	insert(user);
  }
  fclose(f);
}

/*
Cập nhật nội dung file
*/
void update_file(FILE *f){
  f = fopen("account.txt", "w");
  node* p;
  for ( p = root; p!= NULL; p = p->next ){
  	fprintf(f, "%s %s %d\n", p->user.username, p->user.password, p->user.status);
  }
  fclose(f);
}

/*
kiểm tra username có trong list chưa
Output: 1 - username đã tồn tại, 0 - chưa tồn tại
*/
int check_user(char name[100]){
	node* p;
  for ( p = root; p!= NULL; p = p->next ){
  	if(strcmp(p->user.username,name)==0)
  		return 1;
  }
  return 0;
}

/*
Kiểm tra password có trùng ko
Output: 1 - password trùng, 0 - sai password
*/
int check_password(char name[100],char password[100]){
	node* p;
  for ( p = root; p!= NULL; p = p->next ){
  	if(strcmp(p->user.username,name)==0){
  		if(strcmp(p->user.password, password) == 0)
  			return 1;
  	}
  }
  return 0;
}

/*
Tìm trạng thái của username
Output: trả về status của username, trả về -1 nếu xảy ra lỗi 
*/
int find_status(char name[100]){
	node* p;
  for ( p = root; p!= NULL; p = p->next ){
  	if(strcmp(p->user.username,name)==0)
  		return p->user.status;
  }
  return -1;
}

/*
Khóa tài khoản
*/
void block_status(char name[100]){
	node* p;
  for ( p = root; p!= NULL; p = p->next ){
  	if(strcmp(p->user.username,name)==0)
  		p->user.status = 0;
  }
}


int main(int argc, char* argv[]){

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
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	pid_t pid;
	int sin_size;

	char buff[BUFF_SIZE];
	int bytes_sent, bytes_received;


	if ((listen_sock=socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		printf("socket() error\n");
		return 0;
	}
	
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET; 
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   

	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){
		perror("\nError: ");
		return 0;
	}     

	if(listen(listen_sock, BACKLOG) == -1){
		perror("\nError: ");
		return 0;
	}
	
	/* Establish a signal handler to catch SIGCHLD */
	signal(SIGCHLD, sig_chld);

	while(1){
		sin_size=sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size))==-1){
			if (errno == EINTR)
				continue;
			else{
				perror("\nError: ");
				return 0;
			}
		}
		
		/* For each client, fork spawns a child, and the child handles the new client */
		pid = fork();
		
		/* fork() is called in child process */
		if(pid  == 0){
			close(listen_sock);
			printf("You got a connection from [%s:%d]\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port) ); /* prints client's IP */

			FILE *f;
			f = fopen("account.txt", "r+");
			if(f==NULL){// check file
		    printf("Cannot open file!!!\n");
		    return 0;
			}
			read_file(f);

			char username[100], password[100]; // thieu truong hop password thanh cong thi ko cho tiep password sai nua
			char tmp[100];
			int count_wrong_password = 0;
			int login_status = 0;
			strcpy(username," ");
			strcpy(password, " ");

			while(1){
				bytes_received = recv(conn_sock, buff, BUFF_SIZE, 0); //blocking
				if (bytes_received < 0){
					perror("\nError: ");
					break;
				}
				else if (bytes_received == 0){
					printf("Connection closed.");
					break;
				}
				buff[bytes_received-1] = '\0';
				
				printf("Client sent: %s\n", buff);
				
				int option = -1;
				if( check_space(buff) == 0){
					int n = strlen("Error Space");
					bytes_sent = send(conn_sock, "Error Space", n, 0);
				}else{
					option = split_string(buff);
					if(option == -1){
						int n = strlen("Error Format");
						bytes_sent = send(conn_sock, "Error Format", n, 0);
					}else{

						switch(option){
							case 1: // username
								if( strcmp(username, " ") == 0){ // check username da co chua
									if( check_user(buff) == 0){
										strcpy(tmp, "Account does not exist");
										bytes_sent = send(conn_sock, tmp, strlen(tmp), 0);
									}else{
										if( find_status(buff) != 1){
											strcpy(tmp, "Account is blocked");
											bytes_sent = send(conn_sock, tmp, strlen(tmp), 0);
										}else{
											strcpy(username, buff);
											strcpy(tmp, "Account is correct");
											bytes_sent = send(conn_sock, tmp, strlen(tmp), 0);
										}
									}
								}else{
									strcpy(tmp, "Account was entered");
									bytes_sent = send(conn_sock, tmp, strlen(tmp), 0);
								}
								break;
							case 2: // password
								if(strcmp(username," ")== 0){ // check username da co chua
									strcpy(tmp, "Username not entered");
									bytes_sent = send(conn_sock, tmp, strlen(tmp), 0);
								}else if(strcmp(password, " ")!=0){ // check password da co chua
									strcpy(tmp, "Logged in");
									bytes_sent = send(conn_sock, tmp, strlen(tmp), 0);
								}else{
									if( find_status(username) != 1){
										strcpy(tmp, "Account is blocked");
										bytes_sent = send(conn_sock, tmp, strlen(tmp), 0);
										break;
									}
									if( check_password(username, buff) == 0){
										count_wrong_password++;
										strcpy(tmp, "Password is incorrect");

										if(count_wrong_password >= 3){
											printf("Password is incorrect. Account is blocked\n");
											block_status(username);
											strcpy(username, " ");
											update_file(f);
											strcpy(tmp, "Password is incorrect. Account is blocked");
											bytes_sent = send(conn_sock, tmp, strlen(tmp), 0);
											break;
										}
										bytes_sent = send(conn_sock, tmp, strlen(tmp), 0);
									}else{
										count_wrong_password = 0;
										login_status = 1;
										strcpy(password, buff);
										strcpy(tmp, "Password is correct. Login successful");
										bytes_sent = send(conn_sock, tmp, strlen(tmp), 0);
									}
								}
								break;
							case 3: // logout
								if(login_status == 0){
									strcpy(tmp, "Account is not login");
									bytes_sent = send(conn_sock, tmp, strlen(tmp), 0);
								}else{
									if( strcmp(username, buff) != 0){
										strcpy(tmp, "Username is incorrect");
										bytes_sent = send(conn_sock, tmp, strlen(tmp), 0);
									}else{
										strcpy(username, " ");
										strcpy(password, " ");
										login_status = 0;
										count_wrong_password = 0;
										strcpy(tmp, "Logout successful");
										bytes_sent = send(conn_sock, tmp, strlen(tmp), 0);
									}
								}
								break;
							default:
								bytes_sent = send(conn_sock, "No option", strlen("No option"), 0);
								break;
						}
					}
					
				}
				if (bytes_sent < 0)
					perror("\nError: ");
			}
			exit(0);
		}
		
		/* The parent closes the connected socket since the child handles the new client */
		close(conn_sock);
	}
	close(listen_sock);
	return 0;
}