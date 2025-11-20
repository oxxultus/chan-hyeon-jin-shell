#include "signal_func.h"


void shell_siganl_init(){

    // 3. 쉘에서 SIGINT, SIGQUIT 기본 동작 무시 설정
    signal(SIGINT, shell_signal_handler);  // Ctrl-C
    signal(SIGQUIT, shell_signal_handler); // Ctrl-\ (SIG_IGN: 신호 무시)
    
    // 백그라운드 프로세스 종료 시 좀비 방지를 위한 SIGCHLD 설정 (선택 사항이지만 권장됨)
    signal(SIGCHLD, SIG_IGN);
    
}

void shell_signal_handler(int signo) {
    // 아무것도 하지 않음 (신호 무시)
    // SIGCHLD는 백그라운드 프로세스 관리를 위해 별도로 처리할 수 있으나, 
    // 여기서는 간단하게 Ctrl-C/Ctrl-\만 처리합니다.
    print_prompt();
    fflush(stdout);
}