/* $begin shellmain */
#include "csapp.h"
#include<errno.h>
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 

int main() 
{
    char cmdline[MAXLINE]; /* Command line */

    while (1) {
	/* Read */
	/* prj2) 신지원 추가 작성 */
	printf("CSE4100-SP-P2> ");                   
	fgets(cmdline, MAXLINE, stdin); 
	if (feof(stdin))
	    exit(0);

	/* Evaluate */
	eval(cmdline);
    } 
}
/* $end shellmain */
  
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 
    if (argv[0] == NULL)  
	return;   /* Ignore empty lines */
    if (!builtin_command(argv)) { //quit -> exit(0), & -> ignore, other -> run
        
	/* prj2 신지원) fork() 처리를 위하여 execve -> execvp 로 변경 */
	if((pid = Fork()) == 0){
	  if (execvp(argv[0], argv) < 0) {	//ex) /bin/ls ls -al &
            printf("%s: Command not found.\n", argv[0]);
            exit(0);
          }
	}

	/* Parent waits for foreground job to terminate */
	if (!bg){ 
	    int status;
	    if (waitpid(pid, &status, 0) < 0) unix_error("waitfg: waitpid error");
	}
	else//when there is backgrount process!
	    printf("%d %s", pid, cmdline);
    }
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "quit")) /* quit command */
	exit(0);  
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
	return 1;

    /* prj2 신지원) exit & cd 추가 구현 */
    if (!strcmp(argv[0], "exit")) exit(0);
    if (!strcmp(argv[0], "cd")) {
      
      char path[MAXBUF];
      
      // NULL 이거나 ~ 만 입력될 때 home 으로 이동
      if (argv[1] == NULL || strcmp(argv[1], "~") == 0) {
        const char *home = getenv("HOME");
        if (home == NULL) return 1;
        strcpy(path, home);
      }

      // ~ 뒤에 경로가 입력 될 때 해당 경로로 이동
      else if (argv[1][0] == '~') {
        const char *home = getenv("HOME");
        if (home == NULL) return 1;
        snprintf(path, sizeof(path), "%s%s", home, argv[1] + 1);
      }

      // 그 외 입력 처리
      else {
        strncpy(path, argv[1], sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
      }

      if (chdir(path) != 0) {
        perror("cd");
      }

      return 1;
    }

    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
	return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	argv[--argc] = NULL;

    return bg;
}
/* $end parseline */


