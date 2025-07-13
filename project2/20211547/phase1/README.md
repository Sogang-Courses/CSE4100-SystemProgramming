# myshell phase1: Building and Testing Your Shell

## 목표
phase1 에서는 기본적인 shell의 기능을 구현하고자 한다.
사용자가 입력한 명령어를 올바르게 실행하는 구조를 갖추어야 한다.

- 사용자 입력을 받아 파싱
- fork() 를 통해 자식 프로세스 생성
- 자식프로세스에서는 execvp() 를 호출
- 부모프로세스에서는 waitpid() 를 통해 자식 프로세스의 종료를 기다림

## 구현 방식
- eval() 에서 명령어 파싱과 백그라운드 실행 여부를 판단
- 명령어 중 cd, exit, quit 는 별도의 builtin_command() 에서 처리
- 그 외 명령어는 fork() - execvp() 플로우를 통해 실행
- & 기호를 통해 백그라운드 실행
- cd 명령어는 ~/경로, ~ 의 경우를 나누어 처리

## 테스트 방식
make 후, ./myshell 명령어를 통해 테스트 가능합니다.
