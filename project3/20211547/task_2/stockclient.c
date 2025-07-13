#include "csapp.h"

int main(int argc, char **argv)
{
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

        /* prj3 신지원) exit 명령어면 루프 종료*/
        if (strncmp(buf, "exit", 4) == 0)
            break;

	/* prj3 신지원) 버퍼비우기 */
        memset(buf, 0, MAXLINE);

	/* prj3 신지원) 한번에 수신함 */
        ssize_t n = Rio_readnb(&rio, buf, MAXLINE);

	/* prj3 신지원) 연결 종료되었거나 오류 발생시 break */
        if (n <= 0) break;
        Fputs(buf, stdout);
    }

    Close(clientfd);
    exit(0);
}
