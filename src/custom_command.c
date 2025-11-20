#define _GNU_SOURCE // d_type 사용을 위함
#include "custom_command.h"

// 사용자 정의 명령어 + execvp 처리
// 반환: 0 (성공), -1 (실패, execvp 시도 필요), 또는 실행 함수의 종료 코드
int custom_command(char** argv) {
    char* cmd = argv[0];
    int status = -1;

    // 1. 내부 구현된 목록
    if (strcmp(cmd, "ls") == 0) {
        status = custom_ls(argv);
    } else if (strcmp(cmd, "pwd") == 0) {
        status = custom_pwd(argv);
    } else if (strcmp(cmd, "mkdir") == 0) {
        status = custom_mkdir(argv);
    } else if (strcmp(cmd, "rmdir") == 0) {
        status = custom_rmdir(argv);
    } else if (strcmp(cmd, "ln") == 0) {
        status = custom_ln(argv);
    } else if (strcmp(cmd, "cp") == 0) {
        status = custom_cp(argv);
    } else if (strcmp(cmd, "rm") == 0) {
        status = custom_rm(argv);
    } else if (strcmp(cmd, "mv") == 0) {
        status = custom_mv(argv);
    } else if (strcmp(cmd, "cat") == 0) {
        status = custom_cat(argv);
    } else if (strcmp(cmd, "grep") == 0) {
        status = custom_grep(argv);
    }

    // 2. 명령어 실행 결과 반환
    if (status != -1) {
        // 내부 구현 함수가 실행되었다면 (0 또는 다른 종료 코드 반환),
        // 자식 프로세스를 이 상태 코드로 종료시키도록 반환합니다.
        return status;
    }

    // 3. 내부 구현된 명령이 아니면 execvp를 통해 시스템 명령어로 처리
    // 이 시점에서 execvp를 직접 호출하여 현재 프로세스를 대체합니다.
    execvp(argv[0], argv);

    // 4. execvp 실패 시 (오류 발생)
    // execvp가 성공하면 이 코드는 실행되지 않습니다.
    return -1;  // -1을 반환하여 외부에서 perror 처리를 유도
}

// 사용자 설정 함수 프로토타입 예시 (반환값 1은 오류, 0은 정상작동)
// 사용자 설정 ls 기능 함수 (여러 옵션기능을 추가해야됨)
int custom_ls(char** argv) {
    // 1. 현재 디렉토리를 기본 경로로 설정
    char* path = (argv[1] != NULL) ? argv[1] : ".";
    DIR* dir;
    struct dirent* entry;

    // 2. 디렉토리를 엽니다.
    dir = opendir(path);
    if (dir == NULL) {
        // perror는 표준 에러(FD 2)로 출력합니다.
        perror(path);
        return 1;  // 0이 아닌 값은 오류를 의미 (EXIT_FAILURE)
    }

    // 3. 디렉토리 내용을 읽고 표준 출력(FD 1)으로 내보냅니다.
    //fprintf(stdout, "───────────────────────────────────────────\n");
    //fprintf(stdout, "타입  | 파일명\n");
    //fprintf(stdout, "───────────────────────────────────────────\n");

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0){
            continue;
        }
        
        // 파일 이름을 표준 출력에 씁니다.
        // fprintf(stdout, ...) 또는 printf(...)를 사용하면 FD 1에 씁니다.
        // **FD 1은 이미 파이프나 파일로 리다이렉션되어 있을 수 있습니다.**
        // 예 dup2로 디스크립션이 변경되어 있는 경우에 출력으로 지정하면 해당
        // 출력에 해당하는 파이프로 출력됨
        //const char* type_str = get_file_type_string(entry->d_type);
        //fprintf(stdout, "[%s]   | %s\n", type_str, entry->d_name);
        
        fprintf(stdout, "%s\n", entry->d_name);
    }
    //fprintf(stdout, "───────────────────────────────────────────\n");

    // 4. 리소스 정리
    closedir(dir);

    // 5. 성공 반환
    return 0;  // 0은 성공 (EXIT_SUCCESS)을 의미
}
int custom_pwd(char** argv) { 
    char cwd[MAX_CMD_LEN];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n",cwd);
    } else {
        perror("getcwd");
        return 1;
    }
    return 0; 
}

// 개발안됨
int custom_mkdir(char** argv) { return 0; }
int custom_rmdir(char** argv) { return 0; }
int custom_ln(char** argv) { return 0; }
int custom_cp(char** argv) { return 0; }
int custom_rm(char** argv) { return 0; }
int custom_mv(char** argv) { return 0; }
int custom_cat(char** argv) { return 0; }
int custom_grep(char** argv) { return 0; }


// 유틸리티 함수
const char* get_file_type_string(unsigned char d_type) {
    switch (d_type) {
        case DT_REG:
            return "F";    // 일반 파일 (Regular) File
        case DT_DIR:
            return "D";     // 디렉터리 Dir
        case DT_LNK:
            return "L";    // 심볼릭 링크 Link
        case DT_FIFO:
            return "F";    // 명명된 파이프 FIFO
        case DT_SOCK:
            return "S";  // 소켓 Socket
        case DT_CHR:
            return "C";   // 문자 장치 C-Dev
        case DT_BLK:
            return "B";   // 블록 장치 B-Dev
        case DT_UNKNOWN:
            return "U"; // 알 수 없음 Unknown
        default:
            return "O";
    }
}