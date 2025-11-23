#ifndef COMMON_IPC_H
#define COMMON_IPC_H

#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_MSG_SIZE 128    

typedef struct {
    long mtype;             
    int command;            
    char payload[MAX_MSG_SIZE]; 
} IpcMessage;

// 메시지 타입
typedef enum {
    TYPE_MAIN_MANAGER = 1,  // fix
    TYPE_LED_MANAGER,
    TYPE_MOTOR_MANAGER,
    TYPE_STATE_MANAGER,
    TYPE_POWER_MANAGER,
    TYPE_MANAGER_MAX    // MANAGER 메시지 타입 마지막 enum, TYPE_MANAGER_MAX - 1 값으로 메시지 타입 개수를 알 수 있음
} ManagerType;

// 명령어 정의
typedef enum {
    CMD_ON = 1,
    CMD_OFF,
    CMD_START,
    CMD_STOP,
    CMD_GET_STATUS,
    CMD_SET_MODE,
    CMD_REQUEST_PING,
    CMD_SEND_PONG,
    CMD_BOOT_SEQUENCE = 98,
    CMD_SHUTDOWN = 99
} CommandType;

void send_ipc_message(int msgid, long recipient_type, int cmd, const char* data);

// 시스템 상태 정의
typedef enum {
    STATE_BOOTING = 0,
    STATE_DISPLAY = 1,
    STATE_OPERATIONAL = 2,
    STATE_TEST = 3,
    STATE_ECO = 4           
} SystemState;

// 매니저 실행 파일 이름 정의 및 개수
#define LED_MANAGER_EXEC      "./led_manager_proc"
#define MOTOR_MANAGER_EXEC    "./motor_manager_proc"
#define STATE_MANAGER_EXEC    "./state_manager_proc"
#define POWER_MANAGER_EXEC    "./power_manager_proc"
#define NUM_MANAGERS          4 

#endif // COMMON_IPC_H