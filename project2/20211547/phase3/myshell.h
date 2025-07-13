/* prj2) 신지원 myshell.h 생성 */
#ifndef __MYSHELL_H__
#define __MYSHELL_H__

#define MAXARGS 128

void eval(char *cmdline);
void eval_pipeline(char *cmdline, int bg);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

void install_signal_handler(int signum, void (*handler)(int));
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigchld_handler(int sig);
void sigquit_handler(int sig);

void init_jobs();
int add_job(pid_t pid, job_state_t state, char *cmdline);
void delete_job(pid_t pid);
void print_jobs();

#endif
