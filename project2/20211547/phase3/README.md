# myshell phase3: n Processes in Background

## 목표
phase3 에서는 job control, signal 처리를 구현한다.
기존 shell 과 최대한 동일학 작동하도록 해야하며,
사용자의 입력에 맞춰 jobs, fg, bg, kill 명령어를 제어할 수 있어야 한다.

## 구현 방식
- sigint_handler, sigtstp_handler, sigchld_handler, sigquit_handler 에서 각각의 시그널 처리
- job table 관리
- foreground, background 기반 job 수행
- - foreground 에서는  ^C, ^Z 로 종료와 중단 실행
- - background 에서는 kill 가능
- - 중지된 job 에 대하여 foreground/background 로 재개 가능
- pipe 를 통해 여러 가지 명령어 수행 가능

## 테스트 방식
make 후, ./myshell 명령어를 통해 테스트 가능합니다.
