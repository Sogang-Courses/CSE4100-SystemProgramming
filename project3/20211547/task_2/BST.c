#include "BST.h"

BST *bst_create(){
    BST *ret = (BST*)malloc(sizeof(BST));
    return ret;
}

node *bst_create_node() {
    node *new_node = (node *)malloc(sizeof(node));
    if (!new_node) return NULL;

    new_node->left = NULL;
    new_node->right = NULL;
    new_node->data.id = 0;
    new_node->data.left_stock = 0;
    new_node->data.price = 0;

    return new_node;
}

void bst_delete(node *current){
    if(current->left != NULL) bst_delete(current->left);
    if(current->right != NULL) bst_delete(current->right);
    free(current);
}

void bst_destroy(BST *bst) {
    if (bst == NULL) return;
    if (bst->root != NULL)
        bst_delete(bst->root);
    free(bst);
}

void bst_push(BST *bst, node *new_node) {
    if (bst == NULL || new_node == NULL) return;

    if (bst->root == NULL) {
        bst->root = new_node;
        return;
    }

    node *current = bst->root;
    while (1) {
        if (new_node->data.id < current->data.id) {
            if (current->left == NULL) {
                current->left = new_node;
                return;
            }
            current = current->left;
        } else {
            if (current->right == NULL) {
                current->right = new_node;
                return;
            }
            current = current->right;
        }
    }
}

node *bst_search(BST *bst, int id){
    if (!bst) return NULL;
    node *current = bst->root;

    while(current){
        if(current->data.id == id) return current;
        else if(current->data.id < id) current = current->right;
        else current = current->left;
    }

    return NULL;
};

void bst_traverse(node *current, node_callback_fn cb, void *context) {
    if (!current) return;
    cb(context, current);
    bst_traverse(current->left, cb, context);
    bst_traverse(current->right, cb, context);
}

void show_callback(void *context, node *n) {
    char *buf = (char *)context;
    char line[64];
    snprintf(line, sizeof(line), "%d %d %d\n", n->data.id, n->data.left_stock, n->data.price);
    strcat(buf, line);
}

void bst_show(int connfd, char buf[MAXLINE], node *root) {
    if (!root) return;
    bst_traverse(root, show_callback, buf);
}

void write_callback(void *context, node *n) {
    FILE *file = (FILE *)context;
    fprintf(file, "%d %d %d\n", n->data.id, n->data.left_stock, n->data.price);
}

void bst_write(FILE *file, node *root) {
    if (!root || !file) return;
    bst_traverse(root, write_callback, file);
}

void print_callback(void *context, node *n) {
    printf("%d %d %d\n", n->data.id, n->data.left_stock, n->data.price);
}

void bst_print(node *root) {
    bst_traverse(root, print_callback, NULL);
}
