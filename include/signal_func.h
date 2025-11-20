#ifndef SIGNAL_FUNC_H
#define SIGNAL_FUNC_H

#include "include.h"

// ------------------------------------------------
// 3. 인터럽트키 (SIGINT, SIGQUIT) 처리
// ------------------------------------------------

// 시그널 설정 초기화
void shell_siganl_init();

// 쉘(부모 프로세스)에서 Ctrl-C 무시
void shell_signal_handler(int signo);

#endif // SIGNAL_FUNC_H