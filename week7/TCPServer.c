#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h> 
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "linked_list.h"

#define BACKLOG 20   /* Number of allowed connections */
#define BUFF_SIZE 1024

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

/* The recv() wrapper function*/
int receiveData(int s, char *buff, int size, int flags){
	int n;
	n = recv(s, buff, size, flags);
	if(n < 0)
		perror("Error: ");
	return n;
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

	int i, maxi, maxfd, listenfd, connfd, sockfd;
	int nready, client[FD_SETSIZE];
	ssize_t	bytes_received, bytes_sent;
	fd_set	readfds, allset;
	char buff[BUFF_SIZE];
	socklen_t clilen;
	struct sockaddr_in client_addr, server_addr;

	//Step 1: Construct a TCP socket to listen connection request
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		return 0;
	}

	//Step 2: Bind address to socket
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port        = htons(PORT);

	if(bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr))==-1){ /* calls bind() */
		perror("\nError: ");
		return 0;
	} 

	//Step 3: Listen request from client
	if(listen(listenfd, BACKLOG) == -1){  /* calls listen() */
		perror("\nError: ");
		return 0;
	}

	maxfd = listenfd; /* initialize */
	maxi = -1; /* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1; /* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	
	// read file and add link list
	FILE *f;
	f = fopen("account.txt", "r+");
	if(f==NULL){// check file
    printf("Cannot open file!!!\n");
    return 0;
	}
	read_file(f);

	char username[100], password[100];
	char tmp[100];
	int count_wrong_password = 0;
	int login_status = 0;
	strcpy(username," ");
	strcpy(password, " ");

	//Step 4: Communicate with clients
	while (1) {
		readfds = allset;		/* structure assignment */
		nready = select(maxfd+1, &readfds, NULL, NULL, NULL);
		if(nready < 0){
			perror("\nError: ");
			return 0;
		}
		
		if (FD_ISSET(listenfd, &readfds)) {	/* new client connection */
			clilen = sizeof(client_addr);
			if((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &clilen)) < 0)
				perror("\nError: ");
			else{
				printf("You got a connection from %s\n", inet_ntoa(client_addr.sin_addr)); /* prints client's IP */
				for (i = 0; i < FD_SETSIZE; i++)
					if (client[i] < 0) {
						client[i] = connfd;	/* save descriptor */
						break;
					}
				if (i == FD_SETSIZE){
					printf("\nToo many clients");
					close(connfd);
				}

				FD_SET(connfd, &allset); /* add new descriptor to set */
				if (connfd > maxfd)
					maxfd = connfd; /* for select */
				if (i > maxi)
					maxi = i; /* max index in client[] array */

				if (--nready <= 0)
					continue; /* no more readable descriptors */
			}
		}

		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0){
				continue;
			}
			if (FD_ISSET(sockfd, &readfds)) {
				bytes_received = receiveData(sockfd, buff, BUFF_SIZE, 0);
				if (bytes_received <= 0){
					FD_CLR(sockfd, &allset);
					close(sockfd);
					client[i] = -1;
				}else {
					buff[bytes_received-1] = '\0';

					printf("Client sent: %s\n", buff);
					int option = -1;

					if( check_space(buff) == 0){
						int n = strlen("Error Space");
						bytes_sent = send(sockfd, "Error Space", n, 0);
					}else{
						option = split_string(buff);
						if(option == -1){
							int n = strlen("Error Format");
							bytes_sent = send(sockfd, "Error Format", n, 0);
						}else{
							switch(option){
								case 1: // xu ly username
									if( strcmp(username, " ") == 0){ // check username da co chua
										if( check_user(buff) == 0){
											strcpy(tmp, "Account does not exist");
											bytes_sent = send(sockfd, tmp, strlen(tmp), 0);
										}else{
											if( find_status(buff) != 1){
												strcpy(tmp, "Account is blocked");
												bytes_sent = send(sockfd, tmp, strlen(tmp), 0);
											}else{
												strcpy(username, buff);
												strcpy(tmp, "Account is correct");
												bytes_sent = send(sockfd, tmp, strlen(tmp), 0);
											}
										}
									}else{
										strcpy(tmp, "Account was entered");
										bytes_sent = send(sockfd, tmp, strlen(tmp), 0);
									}
									break;
								case 2: // xu ly password
									if(strcmp(username," ")== 0){ // check username da co chua
										strcpy(tmp, "Username not entered");
										bytes_sent = send(sockfd, tmp, strlen(tmp), 0);
									}else if(strcmp(password, " ")!=0){ // check password da co chua
										strcpy(tmp, "Logged in");
										bytes_sent = send(sockfd, tmp, strlen(tmp), 0);
									}else{
										if( find_status(username) != 1){
											strcpy(tmp, "Account is blocked");
											bytes_sent = send(sockfd, tmp, strlen(tmp), 0);
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
												bytes_sent = send(sockfd, tmp, strlen(tmp), 0);
												break;
											}
											bytes_sent = send(sockfd, tmp, strlen(tmp), 0);
										}else{
											count_wrong_password = 0;
											login_status = 1;
											strcpy(password, buff);
											strcpy(tmp, "Password is correct. Login successful");
											bytes_sent = send(sockfd, tmp, strlen(tmp), 0);
										}
									}
									break;
								case 3: // xu ly logout
									if(login_status == 0){
										strcpy(tmp, "Account is not login");
										bytes_sent = send(sockfd, tmp, strlen(tmp), 0);
									}else{
										if( strcmp(username, buff) != 0){
											strcpy(tmp, "Username is incorrect");
											bytes_sent = send(sockfd, tmp, strlen(tmp), 0);
										}else{
											strcpy(username, " ");
											strcpy(password, " ");
											login_status = 0;
											count_wrong_password = 0;
											strcpy(tmp, "Logout successful");
											bytes_sent = send(sockfd, tmp, strlen(tmp), 0);
										}
									}
									break;
								default:
									bytes_sent = send(sockfd, "No option", strlen("No option"), 0);
									break;
							}
						}
					}
					if (bytes_received <= 0){
						FD_CLR(sockfd, &allset);
						close(sockfd);
						client[i] = -1;
					}
				}

				if (--nready <= 0)
					break;		/* no more readable descriptors */
			}
		}
	}
	
	return 0;
}