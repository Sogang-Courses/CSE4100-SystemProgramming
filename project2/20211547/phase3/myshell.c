/* $begin shellmain */
#include "csapp.h"
#include<errno.h>
#define MAXARGS   128

/* prj2-phase3 신지원) Job states and structure */
#define MAXJOBS   64

typedef enum {
    RUN, STOP, TERM, FG
} job_state_t;

static const char *state_str[] = {
    "Running", "Stopped", "Terminated", "Foreground"
};

typedef struct {
    pid_t pid;
    job_state_t state;
    char cmd[MAXBUF];
} job_t;

job_t jobs[MAXJOBS];
int job_idx = 0;
pid_t fg_pid = 0;

/* Function prototypes */
void eval(char *cmdline);
void eval_pipeline(char *cmdline, int bg);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 

/* prj2-phase3 신지원) Function prototypes 추가 */
void install_signal_handler(int signum, void (*handler)(int));
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigchld_handler(int sig);
void sigquit_handler(int sig);

void init_jobs();
int add_job(pid_t pid, job_state_t state, char *cmdline);
void delete_job(pid_t pid);
void print_jobs();

int main() 
{
    char cmdline[MAXLINE]; /* Command line */

    /* prj2-phase3 신지원) 시그너 해들러 연동 및 jobs 초기화 */
    install_signal_handler(SIGCHLD, sigchld_handler);
    install_signal_handler(SIGINT, sigint_handler);
    install_signal_handler(SIGTSTP, sigtstp_handler);
    install_signal_handler(SIGQUIT, sigquit_handler);
    init_jobs();

    while (1) {
	/* Read */
	/* prj2) 신지원 추가 작성 */
	printf("CSE4100-SP-P2> ");                   
	fflush(stdout);
	if (fgets(cmdline, MAXLINE, stdin) == NULL && ferror(stdin)) 
		unix_error("fgets error");
	if (feof(stdin))
	    exit(0);

	/* Evaluate */
	eval(cmdline);
    } 
}
/* $end shellmain */

/* prj2-phase3 신지원) jobs function */
void init_jobs() {
    for (int i = 0; i < MAXJOBS; i++) {
        jobs[i].pid = -1;
        jobs[i].state = TERM;
        jobs[i].cmd[0] = '\0';
    }
}

int add_job(pid_t pid, job_state_t state, char *cmdline) {
  for (int i = 0; i < MAXJOBS; i++) {
    if(jobs[i].pid == -1) {
      jobs[i].pid = pid;
      jobs[i].state = state;
      strncpy(jobs[i].cmd, cmdline, MAXBUF - 1);
      jobs[i].cmd[MAXBUF - 1] = '\0';
      if (i >= job_idx) job_idx = i + 1;
      return i;
    }
  }
  return -1;
}

void delete_job(pid_t pid) {
  for (int i = 0; i < job_idx; i++) {
    if (jobs[i].pid == pid) {
      jobs[i].pid = -1;
      jobs[i].state = TERM;
      jobs[i].cmd[0] = '\0';
      break;
    }
  }
}

void print_jobs() {
  int found = 0;

  for (int i = 0; i < job_idx; i++) {
    if (jobs[i].pid != -1) {
      found = 1;
      printf("[%d] (%d) %s %s", i + 1, jobs[i].pid, state_str[jobs[i].state], jobs[i].cmd);
      if (jobs[i].cmd[strlen(jobs[i].cmd) - 1] != '\n') printf("\n");
    }
  }

  if (!found) {
    printf("no such job\n");
  }
}

/* prj2-phase3 신지원) signal function */
void install_signal_handler(int signum, void (*handler)(int)) {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = handler;
  sa.sa_flags = SA_RESTART;
  if (sigaction(signum, &sa, NULL) < 0) {
    unix_error("sigaction error");
  }
}

void sigint_handler(int sig) {
  if (fg_pid > 0) {
    kill(-fg_pid, SIGINT);
  }
  printf("\n");
}

void sigtstp_handler(int sig) {
  if (fg_pid > 0) {
    kill(-fg_pid, SIGTSTP);
  }
  printf("\n");
}

void sigchld_handler(int sig) {
  int old_errno = errno;
  pid_t pid;
  int status;
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) delete_job(pid);
  errno = old_errno;
}

void sigquit_handler(int sig) {
  printf("Terminating after SIGQUIT...\n");
  exit(0);
}

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
      if ((pid = Fork()) == 0) {
        setpgid(0, 0);
	eval_pipeline(cmdline, 0);
        exit(0);\
      }

      if (bg) {
        add_job(pid, RUN, cmdline);
	printf("[%d] %s", pid, cmdline);
      } else {
        int status;
	if (waitpid(pid, &status, WUNTRACED) < 0) unix_error("waitfg error");
      }

      return;
    }

    bg = parseline(buf, argv);
    if (argv[0] == NULL) return;

    if (!builtin_command(argv)) { //quit -> exit(0), & -> ignore, other -> run
        
	/* prj2 신지원) fork() 처리를 위하여 execve -> execvp 로 변경 */
	if((pid = Fork()) == 0){
	  
	  //Kill 시 shell 자체가 종료되는 것 방지
          setpgid(0, 0);
	  if (execvp(argv[0], argv) < 0) {
            printf("%s: Command not found.\n", argv[0]);
            exit(0);
          }
	}

	/* Parent waits for foreground job to terminate */
	if (!bg){ 
	    int status; 
	    fg_pid = pid;

	    if (waitpid(pid, &status, WUNTRACED) < 0) unix_error("waitfg: waitpid error");
	    fg_pid = 0;

	    if (WIFSTOPPED(status)) {
	      add_job(pid, STOP, cmdline);
	      printf("[%d] Stopped %s", job_idx, cmdline);
	    }
	} else {
            add_job(pid, RUN, cmdline);
            printf("[%d] %s", pid, cmdline);
        }
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

        //Kill 시 shell 자체가 종료되는 것 방지
	setpgid(0, 0);

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

    /* prj2-phase3 신지원) jobs, bg, fg, kill 추가 구현 */
    if (!strcmp(argv[0], "jobs")) {
      print_jobs();
      return 1;
    }

    if (!strcmp(argv[0], "kill") || !strcmp(argv[0], "bg") || !strcmp(argv[0], "fg")) {
      if (!argv[1]) return 1;

      int id = -1;
      if (argv[1][0] == '%' && isdigit(argv[1][1])) {
        id = atoi(&argv[1][1]) - 1;
      } else if (isdigit(argv[1][0])) {
        id = atoi(argv[1]) - 1;
      }

      // 유효 범위와 상태 검사
      if (id < 0 || id >= MAXJOBS || jobs[id].pid == -1) {
        printf("no such job\n");
        return 1;
      }

      pid_t pid = jobs[id].pid;
      
      //kill
      if (!strcmp(argv[0], "kill")) {
          printf("Kill [%d] %s", jobs[id].pid, jobs[id].cmd);
          kill(-getpgid(jobs[id].pid), SIGKILL);
          delete_job(jobs[id].pid);

      //bg
      } else if (!strcmp(argv[0], "bg")) {
	jobs[id].state = RUN;
        kill(-pid, SIGCONT);
        printf("[%d] (%d) %s\n", id + 1, pid, jobs[id].cmd);
  
      //fg
      } else {
	jobs[id].state = FG;
        fg_pid = pid;
	kill(-pid, SIGCONT);
        
	int status;
        if (waitpid(pid, &status, WUNTRACED) < 0) unix_error("waitfg: waitpid error");
        fg_pid = 0;
	if (WIFSTOPPED(status)) jobs[id].state = STOP;
        else delete_job(pid);
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

        while (*buf && (*buf == ' ')) buf++;
    }

    if (argv) argv[argc] = NULL;

    if (argc > 0 && argv && strcmp(argv[argc-1], "&") == 0) {
        if (argv) argv[--argc] = NULL;
        bg = 1;
    }

    return bg;
}
/* $end parseline */


