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
        switch(rcv_msg.command)
        {
            case CMD_SHUTDOWN:
                LOG_INFO("Shutdown received. Preparing for final power off.");
                // 실제 전원 차단 함수 호출
                trigger_hardware_power_off_signal(); 
                break;
            // Watchdog PING 요청 처리
            case CMD_REQUEST_PING: 
                // Main Manager(TYPE_MAIN_MANAGER에게 PONG 응답 전송
                send_ipc_message(msgid, TYPE_MAIN_MANAGER, CMD_SEND_PONG, "Send Pong");
                break;
        }
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