#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include "common_ipc.h"
#include "led_manager.h" 

void manager_loop(int msgid) {
    IpcMessage rcv_msg;
    initialize_led_hardware(); 

    while (1) {
        if (msgrcv(msgid, &rcv_msg, sizeof(IpcMessage) - sizeof(long), TYPE_LED_MANAGER, 0) == -1) break;
        if (rcv_msg.command == CMD_SHUTDOWN) break;

        switch(rcv_msg.command) {
            case CMD_ON: set_led_state(1); break;
            case CMD_OFF: set_led_state(0); break;
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
    if ((key = ftok("./ipc.key", 'A')) == -1) exit(1);
    if ((msgid = msgget(key, 0666)) == -1) exit(1);
    manager_loop(msgid);
    return 0;
}