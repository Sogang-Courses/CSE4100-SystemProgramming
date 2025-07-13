#include "csapp.h"

#define MAX_STOCK 1000
#define FILEPATH "./stock.txt"

/* prj3 신지원) Stock, Pool 구조체 선언 */
typedef struct {
    int id;
    int left_stock;
    int price;
} Stock;

typedef struct {
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int clientfd[FD_SETSIZE];
    rio_t clientrio[FD_SETSIZE];
} Pool;

/* prj3 신지원) select() 방식을 위한 함수 선언 */
Stock stock_list[MAX_STOCK];
int stock_count = 0;

void init_stock_data(const char *filename);
void save_stock_data(const char *filename);
void init_pool(int listenfd, Pool *p);
void add_client(int connfd, Pool *p);
void check_clients(Pool *p);
int process_command(int connfd, char *buf);

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    static Pool pool;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    signal(SIGINT, SIG_IGN);

    /* prj3 신지원) pool 구조체 초기화 */
    listenfd = Open_listenfd(argv[1]);
    init_stock_data(FILEPATH);
    init_pool(listenfd, &pool);

    while (1) {
        pool.ready_set = pool.read_set;
        pool.nready = Select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL);
        
	/* prj3 신지원) 새로운 클라이언트 일때 */
	if (FD_ISSET(listenfd, &pool.ready_set)) {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            add_client(connfd, &pool);
        }

        check_clients(&pool);
    }
}

void init_stock_data(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open stock file.\n");
        exit(1);
    }

    while (fscanf(fp, "%d %d %d", &stock_list[stock_count].id,
                  &stock_list[stock_count].left_stock,
                  &stock_list[stock_count].price) == 3) {
        stock_count++;
    }

    fclose(fp);
}

void save_stock_data(const char *filename) {
    FILE *fp = fopen(filename, "w");
    for (int i = 0; i < stock_count; i++) {
        fprintf(fp, "%d %d %d\n", stock_list[i].id,
                stock_list[i].left_stock,
                stock_list[i].price);
    }
    fclose(fp);
}

void init_pool(int listenfd, Pool *p) {
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
    for (int i = 0; i < FD_SETSIZE; i++) {
        p->clientfd[i] = -1;
    }
}

void add_client(int connfd, Pool *p) {
    p->nready--;
    for (int i = 0; i < FD_SETSIZE; i++) {
        if (p->clientfd[i] < 0) {
            p->clientfd[i] = connfd;
            Rio_readinitb(&p->clientrio[i], connfd);
            FD_SET(connfd, &p->read_set);
            if (connfd > p->maxfd)
                p->maxfd = connfd;
            break;
        }
    }
}

void check_clients(Pool *p) {
    char buf[MAXLINE];
    int n;

    for (int i = 0; i < FD_SETSIZE && p->nready > 0; i++) {
        int connfd = p->clientfd[i];
        rio_t *rio = &p->clientrio[i];

        if ((connfd > 0) && FD_ISSET(connfd, &p->ready_set)) {
            p->nready--;
            
	    /* prj3 신지원) 한 줄 씩 읽어서 처리 */
	    if ((n = Rio_readlineb(rio, buf, MAXLINE)) != 0) {
                int should_exit = process_command(connfd, buf);
                if (should_exit) {
                    Close(connfd);
                    FD_CLR(connfd, &p->read_set);
                    p->clientfd[i] = -1;
                }
	     
            /* prj3 신지원) client 에서 종료 명령어 */
            } else {
                Close(connfd);
                FD_CLR(connfd, &p->read_set);
                p->clientfd[i] = -1;
            }
	}
    }
}

int process_command(int connfd, char *buf) {
    int id, amount;
    char cmd[16];
    char res[MAXLINE];

    if (strncmp(buf, "show", 4) == 0) {
        for (int i = 0; i < stock_count; i++) {
            sprintf(res, "%d %d %d\n", stock_list[i].id,
                    stock_list[i].left_stock,
                    stock_list[i].price);
            Rio_writen(connfd, res, strlen(res));
        }
	
	/* prj3 신지원) show 일 때 끝나는 부분을 명시 */
	Rio_writen(connfd, "end\n", 4);
	return 0;
	
	/* prj3 신지원) 다른 명령어는 1줄만 출력 */
    } else if (sscanf(buf, "%s %d %d", cmd, &id, &amount) == 3) {
        for (int i = 0; i < stock_count; i++) {
            if (stock_list[i].id == id) {
                if (strcmp(cmd, "buy") == 0) {
                    if (stock_list[i].left_stock >= amount) {
                        stock_list[i].left_stock -= amount;
                        strcpy(res, "[buy] success\n");
                    } else {
                        strcpy(res, "Not enough left stock\n");
                    }
                } else if (strcmp(cmd, "sell") == 0) {
                    stock_list[i].left_stock += amount;
                    strcpy(res, "[sell] success\n");
                }
                Rio_writen(connfd, res, strlen(res));
                return 0;
            }
        }
        Rio_writen(connfd, "Stock not found\n", 17);

      /* prj3 신지원) Exit 명령시 파일 닫고, 기록 업데이트 */
    } else if (strncmp(buf, "exit", 4) == 0) {
        Close(connfd);
        save_stock_data(FILEPATH);
	return 1;
    }

    return 0;
}
