#ifndef INCLUDE_H
#define INCLUDE_H

// 각종 설정 값 모음
#include "config.h"

// 시스템 함수
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>

// 사용자 함수
#include "signal_func.h"
#include "command_func.h"
#include "custom_command.h"

#endif // INCLUDE_H