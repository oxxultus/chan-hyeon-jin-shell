#define _GNU_SOURCE // d_type 사용을 위함
#include "custom_command.h"
#include <sys/stat.h>  // mkdir 관련 함수 
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
int custom_mkdir(char** argv) {
	if (argv[1] == NULL) { // 파일 끝까지 확인 후, 디렉토리 이름이 제공되지 않은 경우
        fprintf(stderr, "생성할 디렉토리 이름을 지정해주세요\n");
        return 1; // 오류 코드 반환
    }

    // 삭제 권한은 0777: rwxrwxrwx 권한으로 디렉토리 생성
    if (mkdir(argv[1], 0777) != 0) {
        perror("mkdir");
        return 1;
    }
	 return 0;
 }
int custom_rmdir(char** argv) {
 	 if (argv[1] == NULL) { // 파일 끝까지 확인 후, 디렉토리 이름이 없을 경우
        fprintf(stderr, "삭제할 디렉토리 이름을 지정해주세요\n");
        return 1; // 오류 코드 반환
    }

    if (rmdir(argv[1]) != 0) {
        perror("rmdir");
        return 1;
    }
	 return 0;
 }
int custom_ln(char** argv) { 
    if (argv[1] == NULL || argv[2] == NULL) { // 원본 파일과 링크 파일 이름이 제공되었는지 확인
            fprintf(stderr, "원본 파일과 링크 파일 이름을 확인해주세요\n");
            return 1; // 오류 코드 반환
        }

        // 하드링크 생성
        if (link(argv[1], argv[2]) != 0) {
            perror("ln");
            return 1; // 오류 반환
        }
    return 0;
 }
int custom_cp(char** argv) { 
  if(argv[1] == NULL || argv[2] == NULL) {
        fprintf(stderr, "원본 파일과 복사할 파일의 이름을 확인해주세요\n"); // 원본 파일과 복사할 파일 이름이 제공되었는지 확인
        return 1;
    }

    // FILE* f = fopen("파일명.형식", "모드");
    FILE *src = fopen(argv[1], "rb"); // rb : 바이너리 읽기 모드
    if (src == NULL) { // 원본 파일 열기 실패
        perror("원본");
        return 1;
    }
    FILE *dest = fopen(argv[2], "wb"); // wb : 바이너리 쓰기 모드
    if (dest == NULL) { // 복사할 파일 열기 실패
        perror("복사본");
        fclose(src);
        return 1;
    }

    char buffer[4096]; // 원본 파일에서 데이터를 읽어올 버퍼 생성
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dest);
    }
    fclose(src);
    fclose(dest);
return 0;
 }
int custom_rm(char** argv) {
 if (argv[1] == NULL) {
        fprintf(stderr, "삭제할 파일 이름을 입력하세요.\n");
        return 1;
    }

    if (unlink(argv[1]) != 0) {
        perror("rm");
        return 1;
    }
    return 0;
 }

 int custom_mv(char** argv) {
    if (argv[1] == NULL || argv[2] == NULL) {
        fprintf(stderr, "사용법: mv <원본파일> <새파일>\n");
        return 1;
    }

    if (rename(argv[1], argv[2]) != 0) {
        perror("mv");
        return 1;
    }

    return 0;
}

int custom_cat(char** argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "사용법: cat <파일명>\n");
        return 1;
    }

    // 파일명이 있을 경우에는 해당하는 파일에서 값을 읽어와서 불러와야하고
    // 파일명이 존재하지 않는 경우에는 표준입력으로 값을 입력받아서 처리해야한다.
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("cat");
        return 1;
    }

    char buffer[4096];
    size_t bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        fwrite(buffer, 1, bytes, stdout);
    }

    fclose(fp);
    return 0;
}

int custom_grep(char** argv) {
    // 1. 검색어는 필수, 파일명은 옵션
    if (argv[1] == NULL) {
        fprintf(stderr, "사용법: grep <검색어> [파일]\n");
        return 1;
    }

    char* keyword = argv[1];
    FILE* fp = NULL;
    int use_stdin = 0;   // stdin을 쓰는지 여부

    // 2. 파일명이 없으면 표준입력(stdin)에서 읽기
    if (argv[2] == NULL) {
        fp = stdin;      // FD 0에 연결된 표준입력
        use_stdin = 1;
    } else {
        // 3. 파일명이 있으면 그 파일을 연다
        fp = fopen(argv[2], "r");
        if (fp == NULL) {
            perror("grep");
            return 1;
        }
    }

    // 4. 한 줄씩 읽으면서 keyword가 포함된 줄만 출력
    char line[4096];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, keyword) != NULL) {
            printf("%s", line);
        }
    }

    // 5. stdin은 fclose 하면 안 되니까, 파일을 연 경우에만 닫기
    if (!use_stdin) {
        fclose(fp);
    }

    return 0;
}



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
