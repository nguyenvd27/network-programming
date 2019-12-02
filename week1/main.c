#include <string.h>
#include "linked_list.h"

void menu_func(){
	printf("USER MANAGEMENT PROGRAM\n");
  printf("-------------------------------------\n");
  printf("1. Register\n");
  printf("2. Sign in\n");
  printf("3. Search\n");
  printf("4. Sign out\n");
  printf("Your choice (1-4, other to quit): \n");
}

void read_file(FILE *f){
  user_infor user;
  while(!feof(f)){
  	fscanf(f,"%s %s %d", user.username, user.password, &user.status);
  	if(feof(f)) break;
  	insert(user);
  }
}

void update_file(FILE *f){
  f = fopen("account.txt", "w");
  node* p;
  for ( p = root; p!= NULL; p = p->next ){
  	fprintf(f, "%s %s %d\n", p->user.username, p->user.password, p->user.status);
  }
  fclose(f);
  f = fopen("account.txt", "r+");
}

int check_user(char name[100]){// kiem tra user co trong list chua
	node* p;
  for ( p = root; p!= NULL; p = p->next ){
  	if(strcmp(p->user.username,name)==0) // =0 la giong nhau
  		return 1; // tra ve 1 la user da ton tai
  }
  return 0; // tra ve 0 la user chua ton tai
}

int check_password(char password[100]){
	node* p;
  for ( p = root; p!= NULL; p = p->next ){
  	if(strcmp(p->user.password,password)==0) // =0 la giong nhau
  		return 1; // password giong
  }
  return 0; // sai password
}

int find_status(char name[100]){
	node* p;
  for ( p = root; p!= NULL; p = p->next ){
  	if(strcmp(p->user.username,name)==0) // =0 la giong nhau
  		return p->user.status; // tra ve status cua username
  }
  return -1; // -1 la username ko co trong list
}

void block_status(char name[100]){ // update status cua user ve 0
	node* p;
  for ( p = root; p!= NULL; p = p->next ){
  	if(strcmp(p->user.username,name)==0) // =0 la giong nhau
  		p->user.status = 0;
  }
}

void register_user(FILE *f){
	user_infor user;
	printf("Username: ");
	scanf("%s", user.username);
	int check = check_user(user.username);
	if(check==0){ // username chua ton tai
		user.status = 1; // status cua new user = 1
		printf("Password: ");
		scanf("%s", user.password);
		fprintf(f, "%s %s %d\n", user.username, user.password, user.status);
		insert(user);
		printf("Successful registration\n");
	}else{ // check==1 username da ton tai
		printf("Account existed\n");
	}
	
}

void sign_in(FILE *f,char *signed_in_name, int *signed_in_status){
	int nhap_sai_password = 0; // so lan nhap sai password
	user_infor user;
	printf("Username: ");
	scanf("%s", user.username);

	int check_u = check_user(user.username); // check_u bien kiem tra user da ton tai chua
	if(check_u==1){
		int status = find_status(user.username);
		if(status == 1){ // user dang o trang thai active
			printf("Password: ");
			scanf("%s", user.password);

			int check_p = check_password(user.password); // check_password
			if(check_p == 1){ // password ok
				printf("Hello %s\n", user.username);
				strcpy(signed_in_name, user.username);// save username da dang nhap vao signed_in_name
				*signed_in_status = 1;
			}else{ // nhap sai password
				printf("Password is incorrect\n");
				do{
					nhap_sai_password++;
					if(nhap_sai_password>3){
						printf("Password is incorrect. Acount is blocked\n");
						block_status(user.username); // status = 0
						update_file(f);
						break;
					}
					printf("Password: ");
					scanf("%s", user.password);
					check_p = check_password(user.password);
					if(check_p == 1){ // password ok
						printf("Hello %s\n", user.username);
						strcpy(signed_in_name, user.username);
						*signed_in_status = 1;
					}
				}while(check_p != 1);
			}
		}else{ // user dang o trang thai blocked
			printf("Acount is blocked\n");
		}
	}else{ // user ko ton tai
		printf("Cannot find account\n");
	}
}

void search(int *signed_in_status){
	char name[100];
	printf("Username: ");
	scanf("%s", name);
	if(*signed_in_status==0){ // Neu chua dang nhap thi ko search dc user
		printf("Account is not sign in\n");
	}else{
		int check_u = check_user(name);
		if(check_u == 1){
			int status;
			node* p;
		  for ( p = root; p!= NULL; p = p->next ){
		  	if(strcmp(p->user.username,name)==0) // =0 la giong nhau
		  		status = p->user.status;
		  }
		  if(status==0){
		  	printf("Account is blocked\n");
		  }else if(status == 1){
		  	printf("Account is active\n");
		  }
		}else{
			printf("Cannot find account\n");
		}
	}
}

void sign_out(char *signed_in_name, int *signed_in_status){
	char name[100];
	printf("Username: ");
	scanf("%s", name);

	int check_u = check_user(name);
	if(check_u == 1){ // user ton tai
		if(*signed_in_status==0){ // user chua dang nhap
			printf("Account is not sign in\n");
		}else{ // user da dang nhap
			printf("Goodbye %s\n", signed_in_name);
			signed_in_name = "";
			*signed_in_status = 0;
		}
	}else{
		printf("Cannot find account\n");
	}
}

int main(){

	char signed_in_name[100]=""; // username da sign in
	int *signed_in_status; // trang thai dang nhap, neu =0 la chua dang nhap, =1 la da dang nhap
	int x = 0;
	signed_in_status = &x;
	FILE *f;
	f = fopen("account.txt", "r+");
	if(f==NULL){// check file
    printf("Cannot open file!!!\n");
    return 0;
	}
	read_file(f);
	int menu;
	do{
		menu_func();
		scanf("%d", &menu);
		switch(menu){
			case 1:
				register_user(f);
				break;
			case 2:
				sign_in(f, signed_in_name, signed_in_status);
				break;
			case 3:
				search(signed_in_status);
				break;
			case 4:
				sign_out(signed_in_name, signed_in_status);
				break;
			default:
				to_free(root);
				fclose(f); // close file
				break;
		}
	}while(menu>0&&menu<5);
	return 0;
}