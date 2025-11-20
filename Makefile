# Makefile

# 변수 정의
CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-parameter -I./include -c
LDFLAGS = 
TARGET = bin/main

# 소스 파일 목록
SRCS = src/main.c \
       src/command_func.c \
       src/signal_func.c \
       src/custom_command.c

# 오브젝트 파일 목록 (src/ 디렉토리에 저장)
OBJS = $(SRCS:.c=.o)

# 기본 타겟: 컴파일 및 링크
.PHONY: all
all: $(TARGET)

# 1. 최종 실행 파일 생성: 오브젝트 파일들을 링크하여 TARGET 생성
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# 2. 오브젝트 파일 생성 규칙 (컴파일)
src/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@


# 유틸리티 타겟

# clean 타겟: 생성된 실행 파일 및 src/ 내의 .o 파일 제거
.PHONY: clean
clean:
	rm -f $(TARGET)
	rm -f $(OBJS)