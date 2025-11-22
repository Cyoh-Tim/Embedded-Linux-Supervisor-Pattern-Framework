#include "common_ipc.h"
#include "log.h"
#include <errno.h>
#include <string.h>

void send_ipc_message(int msgid, long recipient_type, int cmd, const char* data) {
    IpcMessage msg;
    
    msg.mtype = recipient_type;
    msg.command = cmd; // command가 int 타입이므로 enum CommandType의 값 할당 가능
    
    strncpy(msg.payload, data, MAX_MSG_SIZE - 1);
    msg.payload[MAX_MSG_SIZE - 1] = '\0';

    if (msgsnd(msgid, &msg, sizeof(IpcMessage) - sizeof(long), 0) == -1) {
        LOG_ERROR("msgsnd failed to send type %ld, cmd %d: %s", 
                   recipient_type, cmd, strerror(errno));
    }
}