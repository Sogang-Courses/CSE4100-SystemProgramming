#include "csapp.h"

struct stock {
    int id;
    int left_stock;
    int price;
};

typedef struct node{
  struct stock data;
  struct node *left;
  struct node *right;
} node;

typedef struct BST{
    node *root;    
} BST;


typedef void (*node_callback_fn)(void *context, node *n);

BST *bst_create();
void bst_delete(node *current);
void bst_destroy(BST *bst);
node *bst_create_node();
void bst_push(BST *bst, node *new_node);
node *bst_search(BST *bst, int id);

void bst_traverse(node *current, node_callback_fn cb, void *context);

void show_callback(void *context, node *n);
void bst_show(int connfd, char buf[MAXLINE], node *current);

void print_callback(void *context, node *n);
void bst_write(FILE *file, node *current);

void print_callback(void *context, node *n);
void bst_print(node *current);
