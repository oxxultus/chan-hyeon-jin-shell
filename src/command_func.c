#include "command_func.h"

// 입력 접두사 설정(현재경로로 표시)
void print_prompt() {
    char cwd[MAX_CMD_LEN];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s> ", cwd);
    } else {
        perror("getcwd");
        printf("unknown> "); // 오류 시 대체 프롬프트 출력
    }
}

// 메인 명령어 처리
int handle_command() {
    char line[MAX_CMD_LEN]; // 최대 입력받을 문자
    char* argv[MAX_ARGS];   // 토큰을 저장할 배열
    int argc;               // 토근의 개수

    print_prompt();
    if (fgets(line, MAX_CMD_LEN, stdin) == NULL) {
        // Ctrl-D (EOF) 처리
        return 0;
    }

    line[strcspn(line, "\n")] = 0; // 개행문자 제거
    argc = parse_command(line, argv); // 토큰화

    if (argc == 0) {
        return 1;  // 빈 입력
    }

    // 1. "exit" 처리
    if (strcmp(argv[0], "exit") == 0) {
        printf("쉘을 종료합니다.\n");
        return 0;
    }

    // 5. 내부 명령어 구현 (예: cd)
    if (strcmp(argv[0], "cd") == 0) {
        if (argc < 2) {
            chdir(getenv("HOME")); // 홈으로 이동
        } else {
            if (chdir(argv[1]) != 0) {
                perror("chdir");
            }
        }
        return 1;  // 쉘 내부 명령어는 루프를 다시 시작
    }

    // 2. 백그라운드 실행 여부 확인 ('&' 처리)
    int background = 0;
    if (argc > 0 && strcmp(argv[argc - 1], "&") == 0) {
        background = 1;
        argv[argc - 1] = NULL;  // '&'를 명령 인자 목록에서 제거
        argc--;
    }

    execute_command(argv, background);
    return 1;
}

// 명령어 토큰화 (띄어쓰기 기준으로)
int parse_command(char* line, char** argv) {
    int argc = 0;
    char* token;

    token = strtok(line, " ");
    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;  // execvp를 위해 인자 목록의 끝을 NULL로 표시
    return argc;
}

// 4. I/O 재지향 처리 (<, >)
int handle_redirection(char** argv, int* argc) {
    int i;
    for (i = 0; argv[i] != NULL; i++) {
        // 출력 재지향 (>)
        if (strcmp(argv[i], ">") == 0) {
            if (argv[i + 1] == NULL) {
                fprintf(stderr, "출력 파일명이 필요합니다.\n");
                return -1;
            }
            // 표준 출력(1)을 새 파일로 재지향 (TRUNC: 덮어쓰기)
            int fd_out = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                perror(argv[i + 1]);
                return -1;
            }
            dup2(fd_out, 1);
            close(fd_out);

            // 명령어 인자 목록에서 >와 파일명 제거
            argv[i] = NULL;
            if (argc != NULL) *argc = i;
            break;
        }

        // 입력 재지향 (<)
        else if (strcmp(argv[i], "<") == 0) {
            if (argv[i + 1] == NULL) {
                fprintf(stderr, "입력 파일명이 필요합니다.\n");
                return -1;
            }
            // 표준 입력(0)을 파일에서 읽도록 재지향
            int fd_in = open(argv[i + 1], O_RDONLY);
            if (fd_in < 0) {
                perror(argv[i + 1]);
                return -1;
            }
            dup2(fd_in, 0);
            close(fd_in);

            // 명령어 인자 목록에서 <와 파일명 제거
            argv[i] = NULL;
            if (argc != NULL) *argc = i;
            break;
        }
    }
    return 0;
}

// 명령 실행 및 I/O/파이프 처리
void execute_command(char** argv, int background) {
    char **cmd_argv[MAX_PIPES + 1];
    int cmd_count;
    
    // 1. 파이프를 기준으로 명령어 분할 시도
    cmd_count = split_commands_by_pipe(argv, cmd_argv);

    if (cmd_count == -1) {
        return; // 파이프 분할 중 오류 발생 시
    }

    // 2. 파이프 존재 여부 분기 (cmd_count > 1이면 파이프 존재)
    if (cmd_count > 1) {
        // 파이프가 존재: 반복문 실행 함수 호출
        execute_pipe_loop(cmd_argv, cmd_count, background);
        return;
    }

    // 파이프가 없을 때 처리
    pid_t pid, wpid;
    int status;
    pid = fork();

    if (pid == 0) {
        // 자식 프로세스: 명령어 실행

        // 3. 자식은 신호 처리를 기본값으로 재설정하여 Ctrl-C에 종료되도록
        // 합니다.
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);

        // 4. I/O 재지향 처리
        int argc_dummy;  
        // handle_redirection이 인자 수를 수정할 수 있으므로 더미 변수 사용
        if (handle_redirection(argv, &argc_dummy) == -1) {
            exit(EXIT_FAILURE);
        }

        // 명령어 실행
        if (custom_command(argv) == -1) { 
            // custom_command가 -1을 반환했다는 것은 execvp가 시스템 명령어를 찾지 못했다는 뜻
            perror(argv[0]);
            exit(EXIT_FAILURE); // 이 경우에만 실패 코드로 종료
        }
        exit(EXIT_FAILURE);  // execvp 실패 시 자식 프로세스 종료
    } else if (pid < 0) {
        // fork 실패
        perror("fork");
    } else {
        // 부모 프로세스: 쉘

        // 2. 백그라운드 여부에 따라 대기
        if (!background) {
            do { // 포그라운드: 자식이 끝날 때까지 대기
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (wpid == -1 && errno == EINTR);  // 인터럽트 시 다시 대기
        } else { // 백그라운드: 바로 다음 프롬프트로 이동
            printf("[%d] %s (백그라운드 실행)\n", pid, argv[0]);
        }
    }
}

// 반복문을 이용한 파이프라인 실행 로직
int execute_pipe_loop(char ***cmd_argv, int cmd_count, int background) {
    int pipe_fds[2]; 
    int input_fd = STDIN_FILENO; 
    pid_t pids[MAX_PIPES + 1]; 
    int current_pids_count = 0; // 현재까지 생성된 자식 수 (PID 배열의 크기)
    
    // 1. 첫 명령어부터 마지막 명령어까지 반복
    for (int i = 0; i < cmd_count; i++) {
        char **current_cmd = cmd_argv[i];
        
        // 마지막 명령이 아닐 경우 파이프 생성
        if (i < cmd_count - 1) { 
            if (pipe(pipe_fds) == -1) {
                perror("pipe");
                cleanup_and_exit(pids, current_pids_count, EXIT_FAILURE);
                return -1;
            }
        }
        
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            cleanup_and_exit(pids, current_pids_count, EXIT_FAILURE);
            return -1;
        } 
        
        // 2. 자식 프로세스 (명령 실행)
        else if (pid == 0) { 
            // A. 신호 처리 복구
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);

            // B. 입력 연결: 이전 파이프의 읽기 쪽(input_fd)을 표준 입력(0)에 연결
            if (input_fd != STDIN_FILENO) {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd); // 원본 FD 닫기
            }
            
            // C. 출력 연결: 다음 파이프의 쓰기 쪽을 표준 출력(1)에 연결
            if (i < cmd_count - 1) {
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[0]); // 자식은 읽기 쪽(0)을 사용하지 않으므로 닫음
                close(pipe_fds[1]); // dup2로 복사했으므로 원본 쓰기 쪽(1)을 닫음
            } // 마지막 연결일 경우 표준 출력 그대로 유지한다.
            
            // D. 파일 리다이렉션 처리 (가장 마지막에 처리하여 파이프 연결을 덮어씀)
            int argc_dummy;
            if (handle_redirection(current_cmd, &argc_dummy) == -1) {
                exit(EXIT_FAILURE);
            }

            // E. 명령어 실행
            if (custom_command(current_cmd) == -1) {
                // custom_command가 -1을 반환했다면, execvp가 시스템 명령어를 찾지 못했다는 뜻
                perror(current_cmd[0]);
                exit(EXIT_FAILURE); // 이 경우에만 실패 코드로 종료
            }
            
            // 여기에 도달해서는 안 됩니다. (도달 시 custom_command 내부의 exit() 누락)
            // 안전을 위해 한번 더 실패 코드로 종료합니다.
            exit(EXIT_FAILURE);
        }
        
        // 3. 부모 프로세스 (파이프 및 FD 정리)
        
        // PID 저장 및 카운트 증가
        pids[current_pids_count++] = pid;
        
        // A. 이전 파이프의 입력 FD 닫음 (부모에게 불필요)
        if (input_fd != STDIN_FILENO) {
            close(input_fd);
        }
        
        // B. 다음 루프를 위한 입력 FD 설정
        if (i < cmd_count - 1) {
            // 다음 루프의 input_fd는 현재 생성된 파이프의 읽기 쪽이 됨
            input_fd = pipe_fds[0]; 
            close(pipe_fds[1]); // 현재 파이프의 쓰기 쪽은 닫음
        }
    }
    
    // 4. 모든 자식 프로세스 대기
    if (!background) {
        // current_pids_count 만큼만 루프를 돌며 대기 
        for (int i = 0; i < current_pids_count; i++) { 
            int status;
            waitpid(pids[i], &status, WUNTRACED); 
        }
    } else {
        printf("[%d] 파이프라인 (백그라운드 실행)\n", pids[0]);
    }
    
    // 파이프라인 그룹의 대표 PID 반환 (가장 왼쪽 명령어 PID)
    return pids[0]; 
}

// 파이프를 기준으로 명령어를 분할
int split_commands_by_pipe(char **argv, char ***commands) {
    int cmd_count = 0;
    commands[cmd_count++] = &argv[0];

    for (int i = 0; argv[i] != NULL; i++) {
        if (strcmp(argv[i], "|") == 0) {
            // 파이프 기호를 NULL로 만들어 명령어 인자 목록을 종료시킴
            argv[i] = NULL; 
            
            // 다음 명령어의 시작 주소를 commands 배열에 저장
            if (argv[i + 1] != NULL) {
                commands[cmd_count++] = &argv[i + 1];
            } else {
                // 파이프 뒤에 명령어가 없는 경우 오류 처리
                fprintf(stderr, "myshell: 파이프 뒤에 명령어가 없습니다.\n");
                return -1;
            }
        }
    }
    return cmd_count; // 총 명령어 블록 수 반환
}

// 자식 프로세스 종료
void cleanup_and_exit(pid_t *pids, int count, int exit_code) {
    fprintf(stderr, "myshell: 파이프라인 오류 발생. 자식 프로세스 정리 중...\n");
    
    // 1. 모든 자식에게 SIGTERM 신호를 보내 종료 요청
    for (int i = 0; i < count; i++) {
        if (pids[i] > 0) {
            // 자식 프로세스에게 SIGTERM (종료 요청) 신호를 보냄
            kill(pids[i], SIGTERM); 
        }
    }
    
    // 2. 모든 자식들이 실제로 종료될 때까지 대기
    int status;
    for (int i = 0; i < count; i++) {
        if (pids[i] > 0) {
            // WNOHANG을 사용하여 블로킹 없이 대기할 수도 있지만, 여기서는 확실한 정리를 위해 블로킹 대기
            waitpid(pids[i], &status, 0); 
        }
    }
    
    // 부모 프로세스 종료
    exit(exit_code);
}