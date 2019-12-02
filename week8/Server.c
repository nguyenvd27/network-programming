/*UDP Echo Server*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFF_SIZE 2048

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

// Sinh số ngẫu nhiên trong khoảng minN maxN
int randomN(int minN, int maxN){
    return minN + rand() % (maxN + 1 - minN);
}

// Khởi tạo tên file
void init_name_file_log(int n, char *name){
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	sprintf(name, "%d#%d-%d-%d#%d:%d:%d.log", n, timeinfo->tm_year + 1900,timeinfo->tm_mon + 1, timeinfo->tm_mday,  timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

// ghi vào file
void write_file_log(char* logFile, char* buff){
  FILE* f = fopen(logFile, "a");
  fprintf(f, "%s", buff);
  fclose(f);
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
	bzero(&(server.sin_zero),8);
  
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
		int n = randomN(1000000,9999999);
		char nameFile[100];
		init_name_file_log(n, nameFile);
		write_file_log(nameFile, buff);
	}
	
	close(server_sock);
	return 0;
}