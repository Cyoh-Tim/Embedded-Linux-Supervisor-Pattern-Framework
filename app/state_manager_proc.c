#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include "common_ipc.h"
#include "state_manager.h"
#include "log.h"

// 상태에 따라 LED/Motor에 다르게 명령을 내리는 함수
void handle_operation(int msgid) {
    SystemState current = get_system_state();
    LOG_INFO("Handling operation in State: %d", current);

    switch (current) {
        case STATE_DISPLAY:
            // 전시 모드: LED는 계속 깜빡이게 하고, 모터는 움직이지 않음
            send_ipc_message(msgid, TYPE_LED_MANAGER, CMD_ON, "Display Mode Blink ON");
            // send_ipc_message(msgid, TYPE_MOTOR_MANAGER, CMD_STOP, "Display Mode Stop");
            break;
            
        case STATE_OPERATIONAL:
            // 운영 모드: LED 켜고, 모터 구동
            send_ipc_message(msgid, TYPE_LED_MANAGER, CMD_ON, "Operational Mode ON");
            send_ipc_message(msgid, TYPE_MOTOR_MANAGER, CMD_START, "Operational Mode Start");
            break;

        case STATE_TEST:
            // 테스트 모드: 특정 테스트 시퀀스 실행
            send_ipc_message(msgid, TYPE_LED_MANAGER, CMD_ON, "Test Mode ON");
            send_ipc_message(msgid, TYPE_MOTOR_MANAGER, CMD_START, "Test Mode Start/Stop Cycle");
            sleep(1);
            send_ipc_message(msgid, TYPE_LED_MANAGER, CMD_OFF, "Test Mode OFF");
            send_ipc_message(msgid, TYPE_MOTOR_MANAGER, CMD_STOP, "Test Mode Stop");
            break;
        case STATE_ECO:
            LOG_INFO("ECO Mode: Preparing for HARD POWER OFF.");
            
            // 모든 하드웨어 정리
            send_ipc_message(msgid, TYPE_LED_MANAGER, CMD_OFF, "Power Off Prepare");
            send_ipc_message(msgid, TYPE_MOTOR_MANAGER, CMD_STOP, "Power Off Prepare");

            // --- 핵심: Power Manager에게 종료 명령 전송 ---
            send_ipc_message(msgid, TYPE_POWER_MANAGER, CMD_SHUTDOWN, "Initiate Hard Shutdown");
            // Power Manager가 이 메시지를 받으면 시스템 전원을 차단합니다.
            
            break;

        case STATE_BOOTING:
        default:
            LOG_INFO("System still booting or unknown state. Doing nothing.");
            break;
    }
}


void manager_loop(int msgid) {
    IpcMessage rcv_msg;
    
    while (1) {
        if (msgrcv(msgid, &rcv_msg, sizeof(IpcMessage) - sizeof(long), TYPE_STATE_MANAGER, 0) == -1) break;

        if (rcv_msg.command == CMD_SHUTDOWN) {
            LOG_INFO("Shutdown received. Orchestrating system shutdown.");
            send_ipc_message(msgid, TYPE_LED_MANAGER, CMD_SHUTDOWN, "Shutdown Request");
            send_ipc_message(msgid, TYPE_MOTOR_MANAGER, CMD_SHUTDOWN, "Shutdown Request");
            break; 
        }

        if (strcmp(rcv_msg.payload, "boot_done") == 0) {
            LOG_INFO("Received 'boot_done'. Defaulting to OPERATIONAL mode.");
            set_system_state(STATE_OPERATIONAL); // 기본 운영 모드로 설정
            handle_operation(msgid); // 초기 동작 실행
        }
        
        if (rcv_msg.command == CMD_SET_MODE) {
            SystemState new_state = (SystemState)atoi(rcv_msg.payload);
            set_system_state(new_state);
            LOG_INFO("Mode changed via IPC to %d. Re-evaluating operations.", new_state);
            // 모드 변경 후, 새로운 상태에 맞는 동작 실행
            handle_operation(msgid); 
        }

        if (rcv_msg.command == CMD_GET_STATUS) {
            LOG_INFO("Current state is %d.", get_system_state());
        }
    }
}

int main() {
    key_t key; int msgid;
    log_init("STATE Proc", NULL);
    if ((key = ftok("./ipc.key", 'A')) == -1) exit(1);
    if ((msgid = msgget(key, 0666)) == -1) exit(1);
    manager_loop(msgid);
    return 0;
}