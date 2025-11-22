// log.c
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <unistd.h> // getpid()

// 콘솔 색깔
#define COLOR_RESET  "\033[0m"
#define COLOR_DEBUG  "\033[36m" // Cyan
#define COLOR_INFO   "\033[32m" // Green
#define COLOR_WARN   "\033[33m" // Yellow
#define COLOR_ERROR  "\033[31m" // Red
#define COLOR_FATAL  "\033[35m" // Magenta

static char g_module_name[32] = "UNKNOWN";
static char g_log_file_path[256] = {0};
static FILE* g_log_fp = NULL;

// 로그 레벨별 문자열 및 색상 매핑
static const char* get_level_str(LogLevel level) {
    switch(level) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO ";
        case LOG_LEVEL_WARN:  return "WARN ";
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

static const char* get_level_color(LogLevel level) {
    switch(level) {
        case LOG_LEVEL_DEBUG: return COLOR_DEBUG;
        case LOG_LEVEL_INFO:  return COLOR_INFO;
        case LOG_LEVEL_WARN:  return COLOR_WARN;
        case LOG_LEVEL_ERROR: return COLOR_ERROR;
        case LOG_LEVEL_FATAL: return COLOR_FATAL;
        default: return COLOR_RESET;
    }
}

void log_init(const char* module_name, const char* log_file_path) {
    if (module_name) {
        strncpy(g_module_name, module_name, sizeof(g_module_name) - 1);
    }
    
    if (log_file_path) {
        strncpy(g_log_file_path, log_file_path, sizeof(g_log_file_path) - 1);
        // Append 모드로 열어서 프로세스 간 덮어쓰기 방지
        // 콘솔에서는 잘 생기진 않지만 UART 통신으로 로그를 출력할 때 멀티 프로세스이기 때문에 동시 출력 현상이 있을 수 있음
        // 나중에 이슈로 IPC로 로그 내용과 타임 스탬프까지 로그를 받아 출력하는 것도 생각하는 방향이 있음
        g_log_fp = fopen(g_log_file_path, "a");
        if (!g_log_fp) {
            perror("Failed to open log file");
        }
    }
}

void log_close() {
    if (g_log_fp) {
        fclose(g_log_fp);
        g_log_fp = NULL;
    }
}

void log_message(LogLevel level, const char* file, int line, const char* fmt, ...) {
    // 1. 시간 구하기
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t); // 시간 구조체
    char time_buffer[26];   // 시간을 저장할 땐 보통 넉넉하게 크기를 잡음
    strftime(time_buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    // 2. 메시지 포맷팅
    va_list args;
    
    // (A) 콘솔 출력 (색상 포함)
    // 포맷: [TIME][MODULE][LEVEL] Message (File:Line)
    printf("%s[%s][%s][%s] ", get_level_color(level), time_buffer, g_module_name, get_level_str(level));
    
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf(" (%s:%d)%s\n", file, line, COLOR_RESET);

    // (B) 파일 출력
    if (g_log_fp) {
        // 파일 출력이 꼬이지 않게 하기 위해 필요하다면 flock 등을 쓸 수 있으나,
        // 간단한 임베디드에서는 append 모드와 짧은 로그로 보통 충분함.
        fprintf(g_log_fp, "[%s][%s][%s] ", time_buffer, g_module_name, get_level_str(level));
        
        va_start(args, fmt);
        vfprintf(g_log_fp, fmt, args);
        va_end(args);

        fprintf(g_log_fp, " (%s:%d)\n", file, line);
        fflush(g_log_fp); // 즉시 파일에 기록
    }
}