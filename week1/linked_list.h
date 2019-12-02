#include <stdio.h>
#include <stdlib.h>

typedef struct elementtype{
  char username[100]; // Khai bao username
  char password[30];
  int status;
}user_infor;

struct node{
  user_infor user;
  struct node *next;
};
typedef struct node node;
node *root,*cur,*new;

node* make_new_node(user_infor user){
  node *new=(node*)malloc(sizeof(node));
  new->user=user;
  new->next=NULL;
  return new;
}

void insert(user_infor user){
  node* new=make_new_node(user);
  if( root == NULL ) {
    root = new;
    cur = root;
  }else {
    new->next=cur->next;
    cur->next = new;
    cur = cur->next;
  }
}

void display_node(node* p){ // hien thi 1 node
  if (p==NULL){printf("Loi con tro NULL\n");
    return; }
  user_infor tmp = p->user;
  printf("%-20s%-20s%-5d\n",tmp.username,tmp.password,tmp.status);
}

void traversing_list(){ // duyet ca list
  node* p;
  for ( p = root; p!= NULL; p = p->next )
    display_node(p);
}

void to_free(node* root){ // giai phong list
  node *to_free;
  to_free=root;
  while(to_free!=NULL){
    root=root->next;
    free(to_free);
    to_free=root;
  }
}