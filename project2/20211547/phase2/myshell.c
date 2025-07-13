/* $begin shellmain */
#include "csapp.h"
#include<errno.h>
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
void eval_pipeline(char *cmdline, int bg);
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
    
    /* prj2-phase2 신지원) 파이프처리를 위해 추가 */
    if (strchr(buf, '|') != NULL) {
      bg = parseline(buf, NULL);

      if (bg) {
        if ((pid = Fork()) == 0) {
          eval_pipeline(cmdline, 0);
          exit(0);
        } else {
          printf("[%d] %s", pid, cmdline);
          return;
        }
      } else {
        eval_pipeline(cmdline, 0);
        return;
      }
    }

    bg = parseline(buf, argv);
    
    if (argv[0] == NULL) return;

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

void eval_pipeline(char *cmdline, int bg) {
  int fd[2];
  pid_t pid1, pid2;
  
 // 왼쪽 명령문에 공백없이 파이프라인올 때 반응 못하는 에러 해결 - 파이프 앞 뒤 공백 강제 삽입 
  char temp_buf[MAXLINE];
  int j = 0;
  for (int i = 0; cmdline[i] != '\0'; i++) {
    if (cmdline[i] == '|') {
      if (i > 0 && temp_buf[j - 1] != ' ') temp_buf[j++] = ' ';
      temp_buf[j++] = '|';
      if (cmdline[i + 1] != ' ') temp_buf[j++] = ' ';
    } else {
      temp_buf[j++] = cmdline[i];
    }
  }
  temp_buf[j] = '\0';
  strcpy(cmdline, temp_buf);

  char *pipe_pos = strchr(cmdline, '|');
  
  //pipe 기호가 없을 경우
  if(pipe_pos == NULL) {
    char *argv[MAXARGS];
    parseline(cmdline, argv);
    if (argv[0] == NULL) return;

    // 기존명령어 대로 실행할 수 없을 경우
    if(!builtin_command(argv)) {
      if ((pid1 = Fork()) == 0) {
        execvp(argv[0], argv);
        fprintf(stderr, "%s: Command not found\n", argv[0]);
        exit(1);
      }
      if (!bg) waitpid(pid1, NULL, 0);
    }

    return;
  }

  //pipe 기호가 있을 경우, 좌우 분리 준비
  *pipe_pos = '\0'; 
  char *left_cmd = cmdline; 
  char *right_cmd = pipe_pos + 1;

  while (*left_cmd == ' ') left_cmd++;
  while (*right_cmd == ' ') right_cmd++;
  
  // pipe() 실패 여부 확인
  if (pipe(fd) < 0) {
    perror("pipe");
    return;
  }

  //left 명령어 처리
  if ((pid1 = Fork()) == 0) {
    close(fd[0]);
    dup2(fd[1], STDOUT_FILENO);
    close(fd[1]);
    
    char *argv[MAXARGS];
    parseline(left_cmd, argv);

    execvp(argv[0], argv);
    fprintf(stderr, "%s: Command not found\n", argv[0]);
    exit(1);
  }
  
  //right 명령어 처리
  if ((pid2 = Fork()) == 0) {
    close(fd[1]);
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]);
    eval_pipeline(right_cmd, bg);
    exit(0);
  }

  close(fd[1]);
  close(fd[0]);

  if (!bg) {
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
  }
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

    /* prj2 phase2 신지원) 백그라운드 여부 명확히 초기화 */
    int bg = 0;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    while (*buf) {
	if (*buf == '"') {
            buf++;
            if ((delim = strchr(buf, '"')) == NULL)
                break;

            *delim = '\0';
            if (argv) argv[argc] = strdup(buf);
            argc++;

            buf = delim + 1;
        } else {
            delim = strchr(buf, ' ');
            if (delim) {
                *delim = '\0';
                if (argv) argv[argc] = strdup(buf);
                argc++;
                buf = delim + 1;
            } else {
                if (argv) argv[argc] = strdup(buf);
                argc++;
                break;
            }
        }

        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }

    if (argv) argv[argc] = NULL;
    if (argc > 0 && argv && strcmp(argv[argc-1], "&") == 0) {
        if (argv) argv[--argc] = NULL;
        bg = 1;
    }

    return bg;
}
/* $end parseline */


