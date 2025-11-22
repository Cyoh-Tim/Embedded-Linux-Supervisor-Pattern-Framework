#ifndef LOG_H
#define LOG_H

// U-BOOT에서 보통 "console=ttyS0,115200" 이런 식의 인자를 커널로 보냄
// 커널에선 이 인자로 /dev/console을 UART로 연결하게되어 stdout은 /dev/console로 연결되어 printf문을 써도 무방함.
// 이런 식이 아닌 경우 fd open을 통해 /dev/ttyS0 같이 출력부에 직접 write하는 함수를 만들어 사용함. << 추천은 안함.
#include <stdio.h>
#include <errno.h>
#include <string.h>

typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
} LogLevel;

// 각 파일에서 로그 초기화
// module_name: "LED", "MOTOR" 등
// log_file_path: 파일로 로그를 저장할 땐 경로를 지정, NULL 값일 경우 console에만 출력.
//                개발할 땐 파일을 한 곳 지정하여 로그를 저장하여 개발하고 실제 release할 땐 빼고 하는 경우가 있음.
//                파일에 저장하는 것까지 구현은 하지만 평소에는 NULL값을 넣을 예정.
void log_init(const char* module_name, const char* log_file_path);

// 로그 리소스 해제
// 프로세스가 종료될 때 사용하지만 프로세스가 종료될 일이 없다고 봐도 무방하므로 에러가 아닌 이상 실제로 사용될 일이 없음.
void log_close();

// 주로 사용할 로그 출력문. (직접 호출할 일 거의 없고 아래 매크로를 통해 사용을 필히 권장)
void log_message(LogLevel level, const char* file, int line, const char* fmt, ...);

// 로그 매크로
// ex: LOG_INFO("Motor Speed: %d", motor_speed);
#define LOG_DEBUG(...) log_message(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  log_message(LOG_LEVEL_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  log_message(LOG_LEVEL_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_message(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) log_message(LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#endif