#include "include.h"

// 메인 쉘 루프
int main() {
    // 시그널 설정 초기화
    shell_siganl_init();

    // 명령어 입력 루프
    while (handle_command()) {}

    return 0;
}