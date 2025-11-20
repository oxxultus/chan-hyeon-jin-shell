#ifndef COMMAND_FUNC
#define COMMAND_FUNC

#include "include.h"

// 입력창 접두사 설정
void print_prompt();

// 메인 명령어 처리
int handle_command();

// 명령어 토큰화 (띄어쓰기 기준으로)
int parse_command(char* line, char** argv);

// 명령 실행 및 I/O/파이프 처리
void execute_command(char** argv, int background);

// 4. I/O 재지향 처리 (<, >)
int handle_redirection(char** argv, int* argc);

// 4. 파이프 처리 (|)
// 반복문을 이용한 파이프라인 실행 로직
// cmd_argv: 파이프를 기준으로 분할된 명령어 배열들의 배열 (char ***)
// cmd_count: 총 명령어 블록 수
int execute_pipe_loop(char ***cmd_argv, int cmd_count, int background);

// 자식 프로세스 종료
void cleanup_and_exit(pid_t *pids, int count, int exit_code);

// 파이프를 기준으로 명령어 분할
int split_commands_by_pipe(char **argv, char ***commands);

#endif  // COMMAND_FUNC