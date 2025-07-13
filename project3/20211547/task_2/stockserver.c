#include "csapp.h"
#include "BST.h"

#define FILEPATH "./stock.txt" 
#define SBUFSIZE 8192
#define NTHREADS 20

typedef struct {
    int *buf;
    int n;
    int front;
    int rear;
    sem_t mutex;
    sem_t slots;
    sem_t items;
} sbuf_t;

void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);

void *thread(void *vargp);
void work(int connfd);
static void init_work(void);

void init_server(char *filepath, BST **bst);
void close_server(char *filepath, BST *bst);

void sigint_handler(int sig);
void save_stock_data(char *filepath, BST *bst);

static sem_t mutex;
sbuf_t sbuf;
BST *bst;

int main(int argc, char **argv) {
    int i, listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    pthread_t tid;
    
    signal(SIGINT, sigint_handler);

    if (argc != 2) {
	    fprintf(stderr, "usage: %s <port>\n", argv[0]);
	    exit(0);
    }
    
    init_server(FILEPATH, &bst);

    listenfd = Open_listenfd(argv[1]);
    sbuf_init(&sbuf, SBUFSIZE);
    for(i = 0; i< NTHREADS; i++)
        Pthread_create(&tid, NULL, thread, NULL);

    while (1) {
	    clientlen = sizeof(struct sockaddr_storage); 
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        sbuf_insert(&sbuf, connfd);
    }

    exit(1);
}

void *thread(void *vargp){
    Pthread_detach(pthread_self());
    while (1) {
        int connfd = sbuf_remove(&sbuf);

        work(connfd);
        Close(connfd);
    }
}

void sbuf_init(sbuf_t *sp, int n){
    sp->buf = calloc(n, sizeof(int));
    sp->n = n;
    sp->front = sp->rear = 0;
    sem_init(&sp->mutex, 0, 1);
    sem_init(&sp->slots, 0, n);
    sem_init(&sp->items, 0, 0); 
}

void sbuf_deinit(sbuf_t *sp){
    free(sp->buf);
}

void sbuf_insert(sbuf_t *sp, int item){
    /* prj3 신지원) 빈 슬롯 확인 */
    P(&sp->slots);
    P(&sp->mutex);
    
    sp->rear = (sp->rear + 1) % sp->n;
    sp->buf[sp->rear] = item;
    
    V(&sp->mutex);
    V(&sp->items);
}

int sbuf_remove(sbuf_t *sp){
    P(&sp->items);
    P(&sp->mutex);
    
    sp->front = (sp->front + 1) % sp->n;
    int item = sp->buf[sp->front];

    V(&sp->mutex);
    V(&sp->slots);

    return item;
}

void init_server(char *filepath, BST **bst){
    FILE *file = fopen(filepath, "r");
    char buf[MAXBUF];

    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", FILEPATH);
        exit(1);
    }

    *bst = bst_create();
    //int line_num = 0;

    while(fgets(buf, MAXBUF, file)){        
        int id, left_stock, price;
	/*
	line_num++;
        
	if (sscanf(line, "%d %d %d", &id, &left_stock, &price) != 3) {
            fprintf(stderr, "[WARNING] Malformed line %d skipped: %s", line_num, line);
            continue;
        }
	*/
	node *new_node = bst_create_node();

        new_node->data.id = id;
        new_node->data.left_stock = left_stock;
        new_node->data.price = price;

        bst_push(*bst, new_node);
    }

    fclose(file);
}

void close_server(char *filepath, BST *bst){
    FILE *file = fopen(filepath, "w");

    if(file == NULL){
        fprintf(stderr, "file open error");
        exit(1);
    }

    bst_write(file, bst->root);   
    bst_destroy(bst);
    fclose(file);
}

/* prj3 신지원) Sem_init 함수 사용 위하여 따로 선언 */
static void init_work(void){
    Sem_init(&mutex, 0, 1);
}

void work(int connfd)
{
    char buf[MAXLINE];
    rio_t rio;
    static pthread_once_t once_control = PTHREAD_ONCE_INIT;

    Pthread_once(&once_control, init_work);
    Rio_readinitb(&rio, connfd);

    while(Rio_readlineb(&rio, buf, MAXLINE) >  0) {
        int id, num;
        char cmd[8];

        sscanf(buf, "%s %d %d", cmd, &id, &num);

        if(!strcmp(cmd, "buy")) {
            char *res;
            node *s = bst_search(bst, id);
            P(&mutex);
            if (s->data.left_stock < num) {
                res = "Not enough left stock\n";
            }
            else {
                s->data.left_stock -= num;
                res = "[buy] success\n";
            }
            V(&mutex);

            Rio_writen(connfd, res, MAXLINE);
        }
        else if(!strcmp(cmd, "sell")) {
            node *s = bst_search(bst, id);

            P(&mutex);
            s->data.left_stock += num;
            V(&mutex);

            char *res = "[sell] success\n";
            Rio_writen(connfd, res, MAXLINE);
        }

	/* prj3 신지원) show 일 때 모든 줄 읽기 */
        else if(!strcmp(cmd, "show")) {
            char buf[MAXLINE] = {0};
            bst_show(connfd, buf, bst->root);
            Rio_writen(connfd, buf, MAXLINE);
        }

	/* prj3 신지원) exit 일 때 저장후 종료 될 수 있도록 */
	/* ++ 추가로 안전하게 처리하기 위해 저장 함수 앞뒤로 뮤텍스 호출 */
        else if(!strcmp(cmd, "exit")) {
            P(&mutex);
	    save_stock_data(FILEPATH,bst);
	    V(&mutex);
	    return;
        }
    }
}

void sigint_handler(int sig){
    
    /* prj3 신지원) 시그널 마스크 설정 */
    sigset_t mask, prev; 
    Sigfillset(&mask);
    Sigprocmask(0, &mask, &prev);

    /* prj3 신지원) 서버가 사용하는 작업큐를 해제함 => 메모리 free 하는 과정 */
    sbuf_deinit(&sbuf);

    close_server(FILEPATH, bst);
    sigprocmask(SIG_SETMASK, &prev, NULL);
    exit(0);
}

void save_stock_data(char *filepath, BST *bst) {
    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        fprintf(stderr, "file open error\n");
        return;
    }

    bst_write(file, bst->root);
    fclose(file);
}
