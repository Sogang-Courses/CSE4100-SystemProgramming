#include "csapp.h"

int main(int argc, char **argv) {
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        Rio_writen(clientfd, buf, strlen(buf));
        /* prj3 신지원) show 일 때 처리 */
	if (strncmp(buf, "show", 4) == 0) {
            while (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
                if (strcmp(buf, "end\n") == 0) break;
                Fputs(buf, stdout);
            }
        } else {
	    /* prj3 신지원) buy, sell, exit 일 때 처리 */
            if (Rio_readlineb(&rio, buf, MAXLINE) > 0)
                Fputs(buf, stdout);
	    /* prj3 신지원) exit 일 떄 즉시 break 처리 */
            if (strncmp(buf, "exit", 4) == 0)
                break;
        }
    }
    Close(clientfd);
    exit(0);
}
