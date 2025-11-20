#ifndef CUSTOM_COMMAND_H
#define CUSTOM_COMMAND_H

#include "include.h"

// EXECVP 대체 함수: 사용자 정의 명령어 처리
// 반환: 0 (성공), -1 (실패, execvp 시도 필요), 또는 실행 함수의 종료 코드
int custom_command(char **argv);

int custom_ls(char **argv);
int custom_pwd(char **argv);
int custom_mkdir(char **argv);
int custom_rmdir(char **argv);
int custom_ln(char **argv);
int custom_cp(char **argv);
int custom_rm(char **argv);
int custom_mv(char **argv);
int custom_cat(char **argv);
int custom_grep(char **argv);

#endif // CUSTOM_COMMAND_H