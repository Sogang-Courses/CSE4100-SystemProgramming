/* prj2) 신지원 myshell.h 생성 */
#ifndef __MYSHELL_H__
#define __MYSHELL_H__

#define MAXARGS 128

void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

#endif
