#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include "common_ipc.h"
#include "power_manager.h"
#include "log.h" 

void manager_loop(int msgid) {
    IpcMessage rcv_msg;
    initialize_power_manager_hardware(); 

    while (1) {
        // Power Manager 타입의 메시지를 기다림
        if (msgrcv(msgid, &rcv_msg, sizeof(IpcMessage) - sizeof(long), TYPE_POWER_MANAGER, 0) == -1) break;

        if (rcv_msg.command == CMD_SHUTDOWN) {
            LOG_INFO("Shutdown received. Preparing for final power off.");
            // 실제 전원 차단 함수 호출
            trigger_hardware_power_off_signal(); 
            break;
        }
        // 다른 전원 관련 명령 처리...
    }
}

int main() {
    key_t key; int msgid;
    log_init("Power Proc", NULL);
    if ((key = ftok("./ipc.key", 'A')) == -1) exit(1);
    if ((msgid = msgget(key, 0666)) == -1) exit(1);
    manager_loop(msgid);
    return 0;
}