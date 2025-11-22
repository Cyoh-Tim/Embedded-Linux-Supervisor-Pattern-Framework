# 1. 변수 정의 (Variables)
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
RM = rm -f

# 소스 파일 목록
COMMON_SRC = common_ipc.c log.c

# 실행 파일 목록 (OUTPUTS)
TARGETS = main led_manager_proc motor_manager_proc state_manager_proc power_manager_proc

#파일이 아니라 명령어 임을 명시합니다.
.PHONY: all clean

# 'all' 타겟: 모든 실행 파일(TARGETS)을 빌드합니다. (기본 타겟이므로 최상위에 둡니다.)
all: $(TARGETS) ipc.key

ipc.key:
	touch ipc.key

main: main_ipc.c $(COMMON_SRC)
	$(CC) $(CFLAGS) -o $@ $^

led_manager_proc: led_manager_proc.c led_manager.c $(COMMON_SRC)
	$(CC) $(CFLAGS) -o $@ $^

motor_manager_proc: motor_manager_proc.c motor_manager.c $(COMMON_SRC)
	$(CC) $(CFLAGS) -o $@ $^

state_manager_proc: state_manager_proc.c state_manager.c $(COMMON_SRC)
	$(CC) $(CFLAGS) -o $@ $^

power_manager_proc: power_manager_proc.c power_manager.c $(COMMON_SRC)
	$(CC) $(CFLAGS) -o $@ $^


# 'clean' 타겟: 빌드된 모든 실행 파일과 오브젝트 파일을 제거합니다.
clean:
	$(RM) $(TARGETS) ipc.key