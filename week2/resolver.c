#include <stdio.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// kiem tra neu la dia chi ip tra ve 1, domain tra ve 0
int check_ip_addr(char *argv){
	int count_period = 0; // dem dau cham
	if(argv[0] == '.' || argv[strlen(argv)-1] == '.') return -1;// Neu dau cham o dau hoac cuoi thi ko phai ip addr
	for(int i = 0; i < strlen(argv); i++){
		if(argv[i] != '.'){
			if (argv[i] < '0' || argv[i] > '9') // neu ko phai so thi ko phai ip addr
				return -1;
		}else{
			count_period++;
			if(count_period > 3) // so dau cham > 3, ko phai ip addr
				return -1;
		}
	}
	if(count_period != 3) // so dau cham != 3, ko phai ip addr
		return -1;

	return 1; // nguoc lai tra ve 1, la dia chi ip
}

// chuyen tu ip_addr sang domain
void ip_to_domain(char *argv){
	struct hostent *host; // luu gia tri tra ve cua gethostbyaddr
  struct in_addr addr;
  inet_pton(AF_INET,argv, &addr); // chuyen ip_addr tu so sang struct in_addr
	host = gethostbyaddr(&addr, sizeof(addr), AF_INET);
	if(host == NULL){ // ko co dia chi ip
		printf("Not found information\n");
	}else{
		printf("Official name: %s\n", host->h_name);

		printf("Alias name:\n");
		int i = 0;
		while(host->h_aliases[i] != NULL){// h_aliases ton tai thi in ra
			printf("%s\n", host->h_aliases[i]);
			i++;
		}
		if(i == 0) printf("NULL\n");
	}
		
}

// chuyen tu domain sang ip_addr
void domain_to_ip(char *argv){
	struct hostent *host; // luu gia tri tra ve cua gethostbymane
	host = gethostbyname(argv);
	if (host == NULL){ // ko co domain
    printf("Not found information\n");
  }else{
  	struct in_addr **addr_list; // luu tru danh sach cac dia chi ip

  	printf("Official IP: %s\n", inet_ntoa(*(struct in_addr*)host->h_addr));// chuyen ip_addr tu struct in_addr sang so
  	printf("Alias IP:\n");
		addr_list = (struct in_addr **)host->h_addr_list;
		int i = 1;
		// i =1 vi addr_list[0] == host->h_addr
		while(addr_list[i] != NULL){
			printf("%s\n", inet_ntoa(*addr_list[i]));
			i++;
		}
		if(i == 1) printf("NULL\n");
  }
}

int main(int argc, char *argv[]){
	if(argc != 2){
		printf("Syntax Error.\n");
		printf("Syntax: ./resolver parameter\n");
		return 1;
	}
	int check = check_ip_addr(argv[1]);// kiem tra parameter
	if(check == 1){ // ip_addr
		ip_to_domain(argv[1]);
	}else{ // domain
		domain_to_ip(argv[1]);
	}
	return 0;
}